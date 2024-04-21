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
    EMOC, //qual a diferença desse EMOC para o empty?
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
                                                {entity_type_t::empty, " "},
                                                {entity_type_t::EMOC, " "},
                                                {entity_type_t::plant, "P"},
                                                {entity_type_t::herbivore, "H"},
                                                {entity_type_t::carnivore, "C"},
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
std::condition_variable readytogo;
std::unique_lock<std::mutex> lock_t(m);

bool simulate_plant(pos_t position, entity_t& entity){
    readytogo.wait(lock_t); //espera sinal da thread principal

    if (entity.type == EMOC){
        return 1;
    }
    if (entity.age == PLANT_MAXIMUM_AGE){
        return 1;    
    }

    // Incrementa a idade da entidade em 1
    entity.age += 1;

    if (random_action(PLANT_REPRODUCTION_PROBABILITY)){
    // procuro posição adjacente vazia
        pos_t current_position; // Declara a posição atual

        for (auto& dir : directions) {

            int new_i = current_position.i + dir.i;
            int new_j = current_position.j + dir.j;
            // Verifica se a posição está dentro dos limites do grid
            if (new_i >= 0 && new_i < entity_grid.size() &&
                new_j >= 0 && new_j < entity_grid[0].size()) {
                entity_t& adjacent_entity = entity_grid[new_i][new_j]; // Use uma referência para modificar a entidade adjacente

                if (adjacent_entity.type == entity_type_t::empty) {
                    // Antes de atualizar a posição da planta
                    pos_t old_position = current_position;

                    // Atualize a posição da planta
                    current_position.i = new_i;
                    current_position.j = new_j;

                    // Remova a planta da posição anterior
                    entity_grid[old_position.i][old_position.j].type = entity_type_t::empty;

                    // Crie uma nova planta na posição atualizada
                    entity_grid[current_position.i][current_position.j].type = entity_type_t::plant;

                }
            }
        } 
    }
    // m.unlock();
    return 0;
}

bool simulate_herbivore(pos_t position, entity_t& entity) {

    readytogo.wait(lock_t); //espera sinal da thread principal

    if (entity.type == EMOC) {
        return 1;
    }
    if (entity.age == HERBIVORE_MAXIMUM_AGE) {
        return 1;
    }
    if (entity.energy == 0) {
        return 1;
    }

    // Incrementa a idade da entidade em 1
    entity.age += 1;

    if (random_action(HERBIVORE_MOVE_PROBABILITY)) {
        // procuro posição adjacente vazia
        pos_t current_position; // Declara a posição atual

        for (auto& dir : directions) {

            int new_i = current_position.i + dir.i;
            int new_j = current_position.j + dir.j;
            // Verifica se a posição está dentro dos limites do grid
            if (new_i >= 0 && new_i < entity_grid.size() &&
                new_j >= 0 && new_j < entity_grid[0].size()) {
                entity_t& adjacent_entity = entity_grid[new_i][new_j]; // Use uma referência para modificar a entidade adjacente

                if (adjacent_entity.type == entity_type_t::empty) {
                    // Antes de atualizar a posição 
                    pos_t old_position = current_position;

                    // Atualize a posição 
                    current_position.i = new_i;
                    current_position.j = new_j;

                    // Remova  da posição anterior
                    entity_grid[old_position.i][old_position.j].type = entity_type_t::empty;

                    // Crie  na posição atualizada
                    entity_grid[current_position.i][current_position.j].type = entity_type_t::herbivore;

                }
            }
        } 
    }
    if (random_action(HERBIVORE_EAT_PROBABILITY)) {
        pos_t current_position; // Declara a posição atual

        for (auto& dir : directions) {

            int new_i = current_position.i + dir.i;
            int new_j = current_position.j + dir.j;
            // Verifica se a posição está dentro dos limites do grid
            if (new_i >= 0 && new_i < entity_grid.size() &&
                new_j >= 0 && new_j < entity_grid[0].size()) {
                entity_t& adjacent_entity = entity_grid[new_i][new_j]; // Use uma referência para modificar a entidade adjacente

                if (adjacent_entity.type == entity_type_t::plant) {
                    // m.lock();
                    adjacent_entity.type = entity_type_t::EMOC;
                    // m.unlock();
                    entity.energy += 30;
                }
            }
        } 
        
    }
    return 0;
}

bool simulate_carnivore(pos_t position, entity_t& entity){
    readytogo.wait(lock_t); //espera sinal da thread principal

    if (entity.type == EMOC){
        return 1;
    }
    if (entity.age == CARNIVORE_MAXIMUM_AGE){
        return 1;    
    }
    if (entity.energy == 0){
        return 1;    
    }

    // Incrementa a idade da entidade em 1
    entity.age += 1;

    if (random_action(CARNIVORE_MOVE_PROBABILITY)){
    // procuro posição adjacente vazia
        pos_t current_position; // Declara a posição atual

        for (auto& dir : directions) {

            int new_i = current_position.i + dir.i;
            int new_j = current_position.j + dir.j;
            // Verifica se a posição está dentro dos limites do grid
            if (new_i >= 0 && new_i < entity_grid.size() &&
                new_j >= 0 && new_j < entity_grid[0].size()) {
                entity_t& adjacent_entity = entity_grid[new_i][new_j]; // Use uma referência para modificar a entidade adjacente

                if (adjacent_entity.type == entity_type_t::empty) {
                    // Antes de atualizar a posição 
                    pos_t old_position = current_position;

                    // Atualize a posição 
                    current_position.i = new_i;
                    current_position.j = new_j;

                    // Remova  da posição anterior
                    entity_grid[old_position.i][old_position.j].type = entity_type_t::empty;

                    // Crie  na posição atualizada
                    entity_grid[current_position.i][current_position.j].type = entity_type_t::carnivore;
                }
            }
        } 
    }
    if (random_action(CARNIVORE_EAT_PROBABILITY)){
        pos_t current_position;

        for (auto& dir : directions) {

            int new_i = current_position.i + dir.i;
            int new_j = current_position.j + dir.j;

            // Verifica se a posição está dentro dos limites do grid
            if (new_i >= 0 && new_i < entity_grid.size() &&
                new_j >= 0 && new_j < entity_grid[0].size()) {
                entity_t& adjacent_entity = entity_grid[new_i][new_j];

                if (adjacent_entity.type == entity_type_t::herbivore) {
                    //m.lock();
                    adjacent_entity.type = entity_type_t::EMOC;
                    //m.unlock();
                    entity.energy = entity.energy + 20;
                }               
            }
        }
    }

    if (random_action(CARNIVORE_REPRODUCTION_PROBABILITY)){
        pos_t current_position;

        for (auto& dir : directions) {

            int new_i = current_position.i + dir.i;
            int new_j = current_position.j + dir.j;

            // Verifica se a posição está dentro dos limites do grid
            if (new_i >= 0 && new_i < entity_grid.size() &&
                new_j >= 0 && new_j < entity_grid[0].size()) {
                entity_t& adjacent_entity = entity_grid[new_i][new_j];

                if (adjacent_entity.type == entity_type_t::carnivore) {
                    std::thread tCarn(simulate_carnivore, std::ref(position), std::ref(entity));
                    tCarn.detach();
                    entity.energy = entity.energy - 10;
                }               
            }
        }
    }
    return 0;
}

int main(){
    crow::SimpleApp app;

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
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { entity_type_t::empty, 0, 0}));

        
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
        ////////////////////////////////////////////////////////////////////////////////////
        

        const int NUM_ROWS = entity_grid.size();
        vector<thread> thread_vec; // acredito que o vetor de threads deveria ser global
        pos_t current_position;

        for (int i = 0; i < NUM_ROWS; ++i) {
            for (int j = 0; j < NUM_ROWS; ++j) {
                entity_t& current_entity = entity_grid[i][j];
                current_position.i = i;
                current_position.j = j;

                if (current_entity.type != entity_type_t::empty) {
                    switch (current_entity.type) {
                        case plant:
                            readytogo.wait(lock_t);
                            thread_vec.push_back(thread(simulate_plant, std::ref(current_position), std::ref(current_entity)));
                            break; 
                        case herbivore:
                            readytogo.wait(lock_t);
                            thread_vec.push_back(thread(simulate_plant, std::ref(current_position), std::ref(current_entity)));
                            break; 
                        case carnivore:
                            readytogo.wait(lock_t);
                            thread_vec.push_back(thread(simulate_plant, std::ref(current_position), std::ref(current_entity)));
                            break; 
                    }
                }
            }
        }


        readytogo.notify_all();
        for (int i=0; i < thread_vec.size(); ++i) {
            thread_vec[i].join();
        }


        /////////////////////////////////////////////////////////////////////////////////////
        // <YOUR CODE ENDS HERE>

        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump();
        
    });

    app.port(8080).run();

    return 0;
}