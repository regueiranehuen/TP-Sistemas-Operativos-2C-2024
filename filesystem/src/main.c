#include "includes/main.h"

t_log *log_filesystem;
t_config *config;
sem_t sem_fin_filesystem;
pthread_mutex_t mutex_bitmap;
t_bitarray *bitmap;
char *mount_dir;
int block_count;
uint32_t block_size;

char* bitmap_path;
char* ruta_completa;


int main(int argc, char *argv[]){

    int socket_servidor;
    estado_filesystem = 1;
    
    inicializar_estructuras();

    socket_servidor = hilo_filesystem(log_filesystem, config);
    //sem_wait(&sem_fin_filesystem);

    sem_wait(&sem_fin_filesystem);

    /*pthread_cancel(hilo_gestor);
    pthread_join(hilo_gestor,NULL);*/

    estado_filesystem = 0;

    sem_post(&sem_conexion_hecha);
    sem_wait(&sem_termina_hilo);

    config_destroy(config);
    
    close(socket_servidor);
    pthread_mutex_destroy(&mutex_bitmap);
    sem_destroy(&sem_conexion_hecha);
    sem_destroy(&sem_fin_filesystem);
    sem_destroy(&sem_termina_hilo);
    
    //crear una func que borre el bitmap cada vez que se vuelva a correr
    //borrar el crear_archivo_dump
    //borrar el bloques.dat

    log_debug(log_filesystem,"Estructuras liberadas. FILESYSTEM TERMINADO");
    log_destroy(log_filesystem);
    return 0;
}