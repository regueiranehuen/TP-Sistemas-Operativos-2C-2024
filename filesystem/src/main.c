#include "includes/main.h"

t_log *log_filesystem;
t_config *config;
sem_t sem_fin_filesystem;
t_bitarray *bitmap;
char *path;
int block_count;
int block_size;

int main(int argc, char *argv[])
{

    int socket_servidor;
    estado_filesystem = 1;

    inicializar_estructuras();

    socket_servidor = hilo_filesystem(log_filesystem, config);

    sem_wait(&sem_fin_filesystem);

    config_destroy(config);
    log_destroy(log_filesystem);
    close(socket_servidor);

    return 0;
}
