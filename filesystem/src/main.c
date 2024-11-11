#include "includes/main.h"

t_log* log_filesystem;
t_config* config;
sem_t sem_fin_filesystem;
t_bitarray* bitmap;
char* path;
int block_count; 
int block_size;

int main(int argc, char* argv[]) {

int socket_servidor;
estado_filesystem = 1;



sem_init(&sem_fin_filesystem,0,0);
sem_init(&sem_conexion_hecha,0,0);


config = config_create("filesystem.config");
t_log_level log_level_int = log_level(config);
log_filesystem = log_create ("filesystem.log","tp",true,log_level_int);

path = config_get_string_value(config, "MOUNT_DIR");
block_count = config_get_int_value(config, "BLOCK_COUNT");
block_size = config_get_int_value(config, "BLOCK_SIZE");

bitmap = cargar_bitmap(path, block_count);


socket_servidor = hilo_filesystem(log_filesystem,config);

sem_wait(&sem_fin_filesystem);

config_destroy(config);
log_destroy(log_filesystem);
close(socket_servidor);

    return 0;
}
