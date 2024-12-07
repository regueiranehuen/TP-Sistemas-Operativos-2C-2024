#include "includes/main.h"

t_log *log_filesystem;
t_config *config;
sem_t sem_fin_filesystem;
pthread_mutex_t mutex_bitmap;
pthread_mutex_t mutex_estado_filesystem;
t_bitarray *bitmap;
char *mount_dir;
int block_count;
uint32_t block_size;
char* bitmap_path;


int main(int argc, char *argv[]){

    if (argc <= 1 || argc > 2)
    {

        printf("Ingrese ./bin/filesystem <path del config>");
        
        return -1;
    }

    int socket_servidor;
    pthread_mutex_lock(&mutex_estado_filesystem);
    estado_filesystem = 1;
    pthread_mutex_unlock(&mutex_estado_filesystem);
    char* path_config = argv[1];
    inicializar_estructuras(path_config);

    socket_servidor = hilo_filesystem(log_filesystem, config);
    //sem_wait(&sem_fin_filesystem);

    sem_wait(&sem_fin_filesystem);

    /*pthread_cancel(hilo_gestor);
    pthread_join(hilo_gestor,NULL);*/

    pthread_mutex_lock(&mutex_estado_filesystem);
    estado_filesystem = 0;
    pthread_mutex_unlock(&mutex_estado_filesystem);

    sem_post(&sem_conexion_hecha);
    sem_wait(&sem_termina_hilo);
    
    config_destroy(config);
    
    close(socket_servidor);
    pthread_mutex_destroy(&mutex_bitmap);
    pthread_mutex_destroy(&mutex_estado_filesystem);
    pthread_mutex_destroy(&cliente_count_mutex);
    sem_destroy(&sem_conexion_hecha);
    sem_destroy(&sem_fin_filesystem);
    sem_destroy(&sem_termina_hilo);
    //crear una func que borre el bitmap cada vez que se vuelva a correr
    //borrar el crear_archivo_dump
    //borrar el bloques.dat
    pthread_mutex_lock(&mutex_logs);
    log_debug(log_filesystem,"Estructuras liberadas. FILESYSTEM TERMINADO");
    log_destroy(log_filesystem);
    pthread_mutex_unlock(&mutex_logs);
    pthread_mutex_destroy(&mutex_logs);
    return 0;
}