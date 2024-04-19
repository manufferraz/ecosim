#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include "../samples/simulate_random_actions.cpp"
#include <random>
#include <mutex>
#include <condition_variable>
#include <barrier>

static const uint32_t NUM_ROWS = 15;
using namespace std;

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
    EMOC,
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

uint32_t i, j;
pos_t directions[4] = {{i, j + 1}, {i, j - 1}, {i + 1, j}, {i - 1, j}};


// Auxiliary code to convert the entity_type_t enum to a string
NLOHMANN_JSON_SERIALIZE_ENUM(entity_type_t, {
                                                {empty, " "},
                                                {EMOC, " "},
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

bool simulate_plant(pos_t position, entity_t& entity)
        {
            // wait(); //espera sinal da thread principal
            // m.lock();//lock mutex do grid

            if (entity.type == EMOC)
            {
                return 1;
            }
            if (entity.age == PLANT_MAXIMUM_AGE)
            {
                return 1;    
            }
            if (random_action(PLANT_REPRODUCTION_PROBABILITY))
            {
            // procuro posição adjacente vazia
                for (auto& dir : directions) {
                    if (entity_grid[dir.i][dir.j] == empty) { //olhar se é isso mesmo
                        // Cria thread da planta na posição vazia
                        thread tPlant(simulate_plant, dir, entity);
                        tPlant.detach();
                    }
                }
            }
            // entity.age++;
            // m.unlock();
        }

        bool simulate_herbivore(pos_t position, entity_t& entity)
        {
                // wait()
                // m.lock();//lock mutex do grid

                if (entity.type == EMOC)
                {
                    return 1;
                }
                if (entity.age == HERBIVORE_MAXIMUM_AGE)
                {
                    return 1;    
                }
                if (entity.energy == 0)
                {
                    return 1;    
                }
                if (random_action(HERBIVORE_MOVE_PROBABILITY)){
                // procuro posição adjacente vazia
                    for (auto& dir : directions) {
                        if (entity_grid[dir.i][dir.j] == empty) { //olhar se é isso mesmo
                            // Cria thread da planta na posição vazia
                            thread tHerb(simulate_herbivore, dir, entity);
                            tHerb.detach();
                        }
                    }
                }
                if (random_action(HERBIVORE_EAT_PROBABILITY)){
                    pos_t current_position;
                    int new_i = current_position.i + dir.i;
                    int new_j = current_position.j + dir.j;

                    // Verifica se a posição está dentro dos limites do grid
                    if (new_i >= 0 && new_i < entity_grid.size() &&
                        new_j >= 0 && new_j < entity_grid[0].size()) {
                        const entity_t& adjacent_entity = entity_grid[new_i][new_j];

                        if (adjacent_entity.type == plant) {
                            // m.lock();
                            adjacent_entity.type = EMOC;
                            //m.unlock();
                            entity.energy = entity.energy + 30;
                        }               
                    }
                }
                if (random_action(HERBIVORE_REPRODUCTION_PROBABILITY)){
                    pos_t current_position;
                    int new_i = current_position.i + dir.i;
                    int new_j = current_position.j + dir.j;

                    // Verifica se a posição está dentro dos limites do grid
                    if (new_i >= 0 && new_i < entity_grid.size() &&
                        new_j >= 0 && new_j < entity_grid[0].size()) {
                        const entity& adjacent_entity = entity_grid[new_i][new_j];

                        if (adjacent_entity.type == herbivore) {
                            thread tHerb(simulate_herbivore, dir, entity);
                            tHerb.detach();
                            entity.energy = entity.energy - 10;
                        }               
                    }
                }
            // entity.age++;
            // m.unlock();
        }

        bool simulate_carnivore(pos_t position, entity_t& entity)
        {
            // wait();
            // m.lock();//lock mutex do grid

            if (entity.type == EMOC)
            {
                return 1;
            }
            if (entity.age == CARNIVORE_MAXIMUM_AGE)
            {
                return 1;    
            }
            if (entity.energy == 0)
            {
                return 1;    
            }
            if (random_action(CARNIVORE_MOVE_PROBABILITY)){
            // procuro posição adjacente vazia
                for (auto& dir : directions) {
                    if (entity_grid[dir.i][dir.j] == empty) {
                        // carnívoro vai para a posição vazia
                        // fazer lógica de substituir posição 
                        entity.energy = entity.energy - 5;
                    }
                }
            }
            if (random_action(CARNIVORE_EAT_PROBABILITY)){
                pos_t current_position;
                int new_i = current_position.i + dir.i;
                int new_j = current_position.j + dir.j;

                // Verifica se a posição está dentro dos limites do grid
                if (new_i >= 0 && new_i < entity_grid.size() &&
                    new_j >= 0 && new_j < entity_grid[0].size()) {
                    const entity_t& adjacent_entity = entity_grid[new_i][new_j];

                    if (adjacent_entity.type == herbivore) {
                        //m.lock();
                        adjacent_entity.type = EMOC;
                        //m.unlock();
                        entity.energy = entity.energy + 20;
                    }               
                }
            }
            if (random_action(CARNIVORE_REPRODUCTION_PROBABILITY)){
                pos_t current_position;
                int new_i = current_position.i + dir.i;
                int new_j = current_position.j + dir.j;

                // Verifica se a posição está dentro dos limites do grid
                if (new_i >= 0 && new_i < entity_grid.size() &&
                    new_j >= 0 && new_j < entity_grid[0].size()) {
                    const entity_t& adjacent_entity = entity_grid[new_i][new_j];

                    if (adjacent_entity.type == carnivore) {
                        thread tCarn(simulate_carnivore, dir, entity);
                        tCarn.detach();
                        entity.energy = entity.energy - 10;
                    }               
                }
            }
        }

int main()
{
    crow::SimpleApp app;

    // Endpoint to serve the HTML page
    CROW_ROUTE(app, "/")
    ([](crow::request &, crow::response &res)
     {
        // Return the HTML content here
        res.set_static_file_info_unsafe("../public/index.html");
        res.end(); });

    CROW_ROUTE(app, "/start-simulation")
        .methods("POST"_method)([](crow::request &req, crow::response &res)
                                { 
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
        
        // <YOUR CODE HERE>
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
        

        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        res.body = json_grid.dump();
        res.end(); 
        
        });

    // Endpoint to process HTTP GET requests for the next simulation iteration
    CROW_ROUTE(app, "/next-iteration")
        .methods("GET"_method)([](){

            const int NUM_ROWS = entity_grid.size();
            vector<thread> thread_vec;

            for (int i = 0; i < NUM_ROWS; ++i) {
                for (int j = 0; j < NUM_ROWS; ++j) {
                    entity_t& current_entity = entity_grid[i][j];

                    if (current_entity.type != empty) {
                        switch (current_entity.type) {
                            case plant:
                                //wait();
                                thread_vec.push_back(thread(simulate_plant, std::ref(current_entity)));
                                break;
                            case herbivore:
                                //wait();
                                thread_vec.push_back(thread(simulate_plant, std::ref(current_entity)));
                                break;
                            case carnivore:
                                //wait();
                                thread_vec.push_back(thread(simulate_plant, std::ref(current_entity)));
                                break;
                        }
                    }
                }
            }
            //notify_all();
            for (int i=0; i < thread_vec.size(); ++i) {
                thread_vec[i].join();
            }

        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump();
        
        });

    app.port(8080).run();

    return 0;
}