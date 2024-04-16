#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include <mutex>
#include <condition_variable>
#include <barrier>

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

class barrier
{
    public:
    barrier(int c);
    void wait();
    private:
    mutex m;
    condition_varible cv;
    int desired_count;
    int current_count;
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
        .methods("GET"_method)([]()
                               {
        // Simulate the next iteration
        // Iterate over the entity grid and simulate the behaviour of each entity
        
        // <YOUR CODE HERE>
        bool update;

        std::barrier::barrier(int c)
        {
        desired_count = c;
        current_count = 0;
        }
        void barrier::wait()
        {
            m.lock();
            current_count++;
            while (current_count < desired_count)
            {
            cv.wait(); //dormindo
            }
            cv.notify_all();
            m.unlock();
        }

        void create_threads(){
            //mutex.lock()
            for (int i = 0; i < NUM_ROWS; ++i) {
                for (int j = 0; j < NUM_ROWS; ++i) {
                    if(entity_grid[i][j] != empty){
                        //thread
                        //c ++ que vai ser c
                    }
            //mutex.unlock()
                }
            }
        }

        bool simulate_plant(pos_t)
        {
            while(true)
            {
                // wait(espera sinal da thread principal)
                //lock mutex do grid
                // if (grid[pos_t.i][post_t.j] == entidade matada ou comida)
                // break;
                // if (age == max_age 10) {
                //  break;    
                //}
                //if (random_action(prob_crescimento)){
                // procuro posição adjacente vazia
                // if (acho posição){
                    // notfica término dessa interação da entidade
                //}
                //}
            }
        };

        bool simulate_herbivore(pos_t)
        {
            while(true)
            {
                // wait(espera sinal da thread principal)
                //lock mutex do grid
                // if (grid[pos_t.i][post_t.j] == entidade matada ou comida)
                // break;
                // if (age == max_age 50) {
                //  break;    
                //}
                // if (energy == 0) {
                //  break;    
                //}
                //if (random_action(prob_move)){
                // procuro posição adjacente vazia
                // if (acho posição){
                    // decresce energia em 5
                    // notfica término dessa interação da entidade
                //}
                //if (random_action(prob_eat)){
                // procuro posição adjacente com planta
                // if (acho posição){
                    // mata thread planta, lugar fica vazio
                    // acrescenta energia em 30
                    // notfica término dessa interação da entidade
                //}
                //if (random_action(prob_reproduzir)){
                // procuro posição adjacente com herbivoro
                // if (acho posição){
                    // decresce energia em 10
                    // crio nova thread herbivore
                    // notfica término dessa interação da entidade
                //}
                //}
            }
        };

        bool simulate_carnivore(pos_t)
        {
            while(true)
            {
                // wait(espera sinal da thread principal)
                //lock mutex do grid
                // if (grid[pos_t.i][post_t.j] == entidade matada ou comida)
                // break;
                // if (age == max_age 50) {
                //  break;    
                //}
                // if (energy == 0) {
                //  break;    
                //}
                //if (random_action(prob_move)){
                // procuro posição adjacente vazia
                // if (acho posição){
                    // decresce energia em 5
                    // notfica término dessa interação da entidade
                //}
                //if (random_action(prob_eat)){
                // procuro posição adjacente com herbivoro
                // if (acho posição){
                    // mata thread herbivoro, fica vazio 
                    // acrescenta energia em 20
                    // notfica término dessa interação da entidade
                //}
                //if (random_action(prob_reproduzir)){
                // procuro posição adjacente com carnivoro
                // if (acho posição){
                    // decresce energia em 10
                    // crio nova thread herbivore
                    // notfica término dessa interação da entidade
                //}
                //}
            }
        }

        // Tenho uma thread principal para a simulação que só acaba quando toda a simulação dentro do tempo especificado acabar
        // Ela recebe o pedido de update e dá cv e notify.all
        // Ela dorme quando todas não estiverem terminado
        // usar flags por info inteiros (current e desired count) se escolher notificar todas as threads
        // a cada interação deleta a barrier antiga e cria uma com new.  
        // Percorre o grid a cada interação. para cada posição que não é vazia cria thread para simular
        // todas tem que ser criadas primeiro para depois simular. Cria todas e não mexe na cv. Depois de todas criadas dá um notifyal. Dá para usar mutex tb ->
        // lock no mutex, cria threads e dá unlock no mutex
        // Threads adjacentes com detach
        // MUTEX : Protege os recursos (grid)
        // flag se posição já foi processada, não pode ter ação nela, o mutex não resolve isso
        // VARIÁVEL DE CONDIÇÃO : Interface manda req atualizar, a thread principal acorda as outras, simulam, e a variável de condição informa que simulou
        // Criar funções para cada tipo de entidade.
        // Verificar unidades de tempo para saber se morre ou continua em todas
        // Usar funções de ações aleatórias
        // Verificar se tá viva qundo der lock, salvar se está viva ou não no grid
        // Se for planta, espaço vazio pode ser crescimento. Cria thread adjacente em uma das direções, se for comida, a thread é derrubada
        // Se for herbívoro, comer acresenta energia, reproduzir e andar decresce energia, se for comida, a thread é derrubada
        // Se for carnívoro, comer acresenta energia, reproduzir e andar decresce energia.
        // Acabou a etapa de tempo (while), devolve a entity_grid

        
        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}