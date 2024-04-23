#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include "../samples/simulate_random_actions.cpp"
#include <mutex>
#include <condition_variable>
#include <barrier>
#include <vector>
#include <string>
#include <unistd.h>
#include <semaphore.h>


static const uint32_t NUM_ROWS = 15;

// Constants
const uint32_t PLANT_MAXIMUM_AGE = 10;
const uint32_t HERBIVORE_MAXIMUM_AGE = 50;
const uint32_t CARNIVORE_MAXIMUM_AGE = 80;
const uint32_t MAXIMUM_ENERGY = 200;
const uint32_t THRESHOLD_ENERGY_FOR_REPRODUCTION = 20;

// Probabilities
const double PLANT_REPRODUCTION_PROBABILITY = 0.2;
const double HERBIVORE_REPRODUCTION_PROBABILITY = 0.075;
const double CARNIVORE_REPRODUCTION_PROBABILITY = 0.025;
const double HERBIVORE_MOVE_PROBABILITY = 0.7;
const double HERBIVORE_EAT_PROBABILITY = 0.9;
const double CARNIVORE_MOVE_PROBABILITY = 0.5;
const double CARNIVORE_EAT_PROBABILITY = 1.0;

// Type definitions
enum entity_type_t
{
    empty,
    plant,
    herbivore,
    carnivore
};

struct pos_t
{
    uint32_t i;
    uint32_t j;
};

struct entity_t
{
    entity_type_t type;
    int32_t energy;
    int32_t age;
};

// Auxiliary code to convert the entity_type_t enum to a string
NLOHMANN_JSON_SERIALIZE_ENUM(entity_type_t, {
                                                {empty, " "},
                                                {plant, "P"},
                                                {herbivore, "H"},
                                                {carnivore, "C"},
                                            })

// Auxiliary code to convert the entity_t struct to a JSON object
namespace nlohmann
{
    void to_json(nlohmann::json &j, const entity_t &e)
    {
        j = nlohmann::json{{"type", e.type}, {"energy", e.energy}, {"age", e.age}};
    }
}

// Grid that contains the entities
static std::vector<std::vector<entity_t>> entity_grid;

std::mutex m;
std::condition_variable readyForNextIteration;

std::mutex t;
std::condition_variable readyToCheckThreadCount;
std::condition_variable readyToCreateThread;
int numActiveThreads;
int numProcessedThreads;
int removedThreads;

sem_t semaphore;


char translate(entity_t& entity){
    char v[] = {'E','P','H','C'};
    return v[entity.type];
}


bool simulate_herbivore(pos_t position, entity_t& entity){
    sem_post(&semaphore);
    bool firstrun = true;
    bool entityIsDead = false;
    printf("Starting a new Thread\n");
    while(1){
        printf("%c at %d %d: wait lock \n", translate(entity), position.i, position.j);
        std::unique_lock<std::mutex> lock(m);
        readyForNextIteration.wait(lock);
        printf("C in \n");
        if (firstrun ){
            numActiveThreads++;
            firstrun = false;
        }

        if (entity.age > PLANT_MAXIMUM_AGE){
            entity_grid[position.i][position.j] = { empty, 0, 0 };
            entityIsDead = true;
            removedThreads++;
            
        }else{
            entity.age++;
        }
        numProcessedThreads++;
        
        lock.unlock();
        lock.release();
        readyToCheckThreadCount.notify_all();
        printf("C out \n");
        if (entityIsDead){
            return true;
        }
            
    }

    
    return true;
}


int main(){
    crow::SimpleApp app;
    sem_init(&semaphore, 0, 0);

    // Endpoint to serve the HTML page
    CROW_ROUTE(app, "/")([](crow::request &, crow::response &res){
        // Return the HTML content here
        res.set_static_file_info_unsafe("../public/index.html");
        res.end();
    });

    CROW_ROUTE(app, "/start-simulation").methods("POST"_method)([](crow::request &req, crow::response &res){ 
        // Parse the JSON request body
        nlohmann::json request_body = nlohmann::json::parse(req.body);

       // Validate the request body 
        uint32_t total_entinties = (uint32_t)request_body["plants"] + (uint32_t)request_body["herbivores"] + (uint32_t)request_body["carnivores"];
        if (total_entinties > NUM_ROWS * NUM_ROWS) {
        res.code = 400;
        res.body = "Too many entities";
        res.end();
        return;
        }

        // Clear the entity grid
        entity_grid.clear();
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0}));
        
        // Create the entities
        // <YOUR CODE HERE>
        ////////////////////////////////////////////////////////////////////////////////////
        
        // Create the entities
        int numPlants = (uint32_t)request_body["plants"];
        int numHerbivores = (uint32_t)request_body["herbivores"];
        int numCarnivores = (uint32_t)request_body["carnivores"];


        // Inicializa as plantas aleatoriamente
        for (int i = 0; i < numPlants; ++i) {
            pos_t position = { std::rand() % NUM_ROWS, std::rand() % NUM_ROWS };
            entity_grid[position.i][position.j] = { plant, 0, 0 };
        }

        // Inicializa os herbívoros aleatoriamente
        for (int i = 0; i < numHerbivores; ++i) {
            pos_t position = { std::rand() % NUM_ROWS, std::rand() % NUM_ROWS };
            entity_grid[position.i][position.j] = { herbivore, 100, 0 };
        }

        // Inicializa os carnívoros aleatoriamente
        for (int i = 0; i < numCarnivores; ++i) {
            pos_t position = { std::rand() % NUM_ROWS, std::rand() % NUM_ROWS };
            entity_grid[position.i][position.j] = { carnivore, 100, 0 };
        } 

        printf("checkpoint 1\n");
        
        m.lock();
            numActiveThreads = 0;
            numProcessedThreads = 0;
            removedThreads = 0;
        m.unlock();

         printf("checkpoint 2\n");
        

        pos_t current_position;
        std::thread t;
        for (int i = 0; i < NUM_ROWS; ++i) {
            for (int j = 0; j < NUM_ROWS; ++j) {
                std::unique_lock<std::mutex> lock(m);
                entity_t& current_entity = entity_grid[i][j];
                current_position.i = i;
                current_position.j = j;
                printf("%d %d %c\n",i,j, translate(entity_grid[i][j]));

                if (current_entity.type != entity_type_t::empty) {
                    switch (current_entity.type) {
                        case plant:
                            t = std::thread(simulate_herbivore, std::ref(current_position), std::ref(current_entity));
                            t.detach();
                            break; 
                        case herbivore:
                            t = std::thread(simulate_herbivore, std::ref(current_position), std::ref(current_entity));
                            t.detach();
                            break; 
                        case carnivore:
                            t = std::thread(simulate_herbivore, std::ref(current_position), std::ref(current_entity));
                            t.detach();
                            break; 
                    }
                    printf("thread: %d for %c at %d %d\n",t.get_id(), translate(current_entity),current_position.i,current_position.j);
                    sem_wait(&semaphore);
                }
                
            }
        }
         printf("checkpoint 3\n");
        
        /////////////////////////////////////////////////////////////////////////////////////
        // <YOUR CODE ENDS HERE>

        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        res.body = json_grid.dump();
        res.end(); 
    });

    // Endpoint to process HTTP GET requests for the next simulation iteration
    CROW_ROUTE(app, "/next-iteration").methods("GET"_method)([](){
        // Simulate the next iteration
        // Iterate over the entity grid and simulate the behaviour of each entity
        
        
        // <YOUR CODE HERE>
        /////////////////////////////////////////////////////////////////////////////////////

        m.lock();

            numProcessedThreads = 0;
            numActiveThreads -= removedThreads;
            removedThreads = 0;
        m.unlock();

        
        printf("Notifying threads\n");

        readyForNextIteration.notify_all();
        
        //esperando as threads terminarem de processar 
        while(1){

            std::unique_lock<std::mutex> lock(m);
            readyToCheckThreadCount.wait(lock);
            printf("numProcessedThreads %d\n", numProcessedThreads );
            printf("numActiveThreads %d\n", numActiveThreads );
            if(numProcessedThreads >= numActiveThreads){
                break;
            }
        }
    
        
        

        /////////////////////////////////////////////////////////////////////////////////////
        // <YOUR CODE ENDS HERE>

        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); 
    });
    app.port(18081).run();


    printf("Fim do codigo\n");
    return 0;
}