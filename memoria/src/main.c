#include "includes/main.h"
//#include "includes/memoriaUser.h"

int estado_cpu = 1;
t_log *logger;
t_config *config;
//t_memoria* mem;


int main(int argc, char *argv[]){

    sockets_memoria *sockets = malloc(sizeof(sockets_memoria));
    sockets_iniciales = malloc(sizeof(sockets_memoria));

    logger = log_create("memoria.log", "tp", true, LOG_LEVEL_TRACE);
    config = config_create("memoria.config");

    leer_config(argv[1]);
    mem = inicializar_memoria(mem->esquema, mem->tamano_memoria, mem->lista_particiones, mem->num_particiones); // ver como contabilizar las particiones

    inicializar_mutex();
    inicializar_semaforos();
    inicializar_estructuras();

    if (config == NULL){
        log_error(logger, "Error al crear la configuración");
        return -1;
    }

    sockets = hilos_memoria(logger, config);

    sem_wait(&sem_conexion_iniciales); //esperar a que se haga la conexion con cpu y kernel
    sem_wait(&sem_conexion_iniciales);

    hilo_recibe_cpu();
    
    sem_wait(&sem_fin_memoria);
    //sem_wait para terminar la ejecucion de memoria
    
    destruir_mutex();
    destruir_semaforos();

    config_destroy(config);
    log_destroy(logger);
    close(sockets->socket_servidor);
    close(sockets->socket_cliente);
    close(sockets_iniciales->socket_cpu);
    close(sockets_iniciales->socket_kernel);
    free(sockets_iniciales);
    free(sockets);

    return 0;
}

void hilo_recibe_cpu(){
    pthread_t hilo_cliente_cpu;
    int resultado = pthread_create(&hilo_cliente_cpu, NULL, recibir_cpu, NULL);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo que recibe a cpu desde memoria");
    }
    pthread_detach(hilo_cliente_cpu);
}

void leer_config(char* path) {
    config = iniciar_config(path);

    mem->tamano_memoria = config_get_int_value(config, "TAM_MEMORIA");
    mem->esquema = config_get_int_value(config, "FIJAS");
    mem->estrategia = config_get_int_value(config, "ALGORITMO_BUSQUEDA");

    // Obtener la cadena de particiones
    char* particiones_string = config_get_string_value(config, "PARTICIONES");
    int count;
    if (particiones_string != NULL) {
        count = 0;
        mem->lista_particiones = parse_partitions(particiones_string, &count);
        
        if (mem->lista_particiones) {
            // Aquí puedes usar mem->lista_particiones y count como necesites
            // Por ejemplo, podrías imprimir las particiones
            for (int i = 0; i < count; i++) {
                printf("Partición %d: %d\n", i + 1, mem->lista_particiones[i]);
            }
        }
    } else {
        fprintf(stderr, "Error: No se pudo obtener la configuración de PARTICIONES.\n");
    }
    mem->num_particiones = count;
}

int* parse_partitions(const char* partition_string, int* count) {
    int size = 4;
    int* partitions = malloc(sizeof(int) * size);  // Almacenamos los enteros
    if (!partitions) {
        perror("Failed to allocate memory");
        return NULL; // Manejo de error
    }

    char* token;
    char* str_copy = strdup(partition_string); // Hacemos una copia de la cadena

    token = strtok(str_copy, ",");  // Separa la cadena por comas
    *count = 0;  // Inicializa el contador

    while (token != NULL) {
        // Si el arreglo está lleno, lo duplicamos
        if (*count >= size) {
            size *= 2;
            partitions = realloc(partitions, sizeof(int) * size);
            if (!partitions) {
                perror("Failed to allocate memory");
                free(str_copy); // Libera la copia de la cadena
                return NULL; // Manejo de error
            }
        }
        partitions[*count] = atoi(token); // Convierte y almacena el número
        (*count)++;  // Incrementa el contador
        token = strtok(NULL, ",");  // Obtiene el siguiente token
    }

    free(str_copy); // Libera la copia de la cadena
    return partitions; // Retorna el arreglo dinámico
}