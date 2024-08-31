#include "includes/main.h"

int main(int argc, char* argv[]) {

t_log * log;
t_config * config;
int socket_servidor;

log = log_create ("filesystem.log","tp",true,LOG_LEVEL_TRACE);
config = config_create("filesystem.config");

socket_servidor = hilo_filesystem(log,config);

config_destroy(config);
log_destroy(log);
close(socket_servidor);

    return 0;
}
