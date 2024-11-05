#include "includes/main.h"

t_log* log_filesystem;
t_config* config;


int main(int argc, char* argv[]) {

    int socket_servidor;

    config = config_create("filesystem.config");
    t_log_level log_level_int = log_level(config);
    log_filesystem = log_create ("filesystem.log","tp",true,log_level_int);

    socket_servidor = hilo_filesystem(log_filesystem,config);

    config_destroy(config);
    log_destroy(log_filesystem);
    close(socket_servidor);

    return 0;
}
