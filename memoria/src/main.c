#include "includes/main.h"

int estado_cpu = 1;
t_log *logger;
t_config *config;

int main(int argc, char *argv[]){

    sockets_memoria *sockets = malloc(sizeof(sockets_memoria));
    sockets_iniciales = malloc(sizeof(sockets_memoria));

    config = config_create("memoria.config");
    t_log_level log_level_int = log_level(config);
    logger = log_create("memoria.log", "tp", true, log_level_int);
    
    inicializar_Memoria(config);
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
/*
void leer_config() {
    memoria->tamano_memoria = config_get_int_value(config, "TAM_MEMORIA");
    char* esquema_string = config_get_string_value(config, "ESQUEMA");
    char* estrategia_string = config_get_string_value(config, "ALGORITMO_BUSQUEDA");

    if(strcmp(esquema_string,"FIJAS") == 0){
        memoria->esquema = PARTICION_FIJA;
    }else{
        memoria->esquema = PARTICION_DINAMICA;
    }

    if(strcmp(estrategia_string,"FIRST") == 0){
        memoria->estrategia = FIRST_FIT;
    }else if (strcmp(estrategia_string,"BEST") == 0){
        memoria->estrategia = BEST_FIT;
    }else {
        memoria->estrategia = WORST_FIT;
    }
    // Obtener la cadena de particiones
    memoria->lista_particiones = convertirArrayDeNumeros(config_get_array_value(config, "PARTICIONES"));
}

int* convertirArrayDeNumeros(char** caracteres){
    int size = string_array_size(caracteres);
    int* intArray = (int*)malloc(size * sizeof(int));

        for (int i = 0; i < size; ++i) {
            intArray[i] = atoi(caracteres[i]);
        }
        
    return intArray;
}
*/