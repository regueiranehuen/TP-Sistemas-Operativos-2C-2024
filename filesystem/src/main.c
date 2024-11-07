#include "includes/main.h"

t_log* log_filesystem;
t_config* config;
sem_t sem_fin_filesystem;

int main(int argc, char* argv[]) {

int socket_servidor;
estado_filesystem = 1;

sem_init(&sem_fin_filesystem,0,0);
sem_init(&sem_conexion_hecha,0,0);

config = config_create("filesystem.config");
t_log_level log_level_int = log_level(config);
log_filesystem = log_create ("filesystem.log","tp",true,log_level_int);


socket_servidor = hilo_filesystem(log_filesystem,config);

sem_wait(&sem_fin_filesystem);

config_destroy(config);
log_destroy(log_filesystem);
close(socket_servidor);

    return 0;
}
