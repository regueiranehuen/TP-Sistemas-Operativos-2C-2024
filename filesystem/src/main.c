#include "includes/main.h"

int main(int argc, char* argv[]) {

t_log * log;
t_config * config;
int socket_servidor;

config = config_create("filesystem.config");
t_log_level log_level_int = log_level(config);
log = log_create ("filesystem.log","tp",true,log_level_int);

socket_servidor = hilo_filesystem(log,config);

config_destroy(config);
log_destroy(log);
close(socket_servidor);

    return 0;
}
