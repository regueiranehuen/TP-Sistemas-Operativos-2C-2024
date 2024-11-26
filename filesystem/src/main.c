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

    config_destroy(config);
    log_destroy(log_filesystem);
    close(socket_servidor);
    //crear una func que borre el bitmap cada vez que se vuelva a correr
    //borrar el crear_archivo_dump
    //borrar el bloques.dat

    return 0;
}