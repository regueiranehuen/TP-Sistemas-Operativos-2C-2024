#include "includes/main.h"

int main(int argc, char* argv[]) {

t_log* log;
t_config* config;

log = log_create("memoria.log", "tp", true, LOG_LEVEL_INFO);
config= config_create("memoria.config");
if (config == NULL) {
    log_error(log, "Error al crear la configuración. Asegúrate de que el archivo exista y sea legible.");
}
servidor_memoria_kernel(log,config);

    config_destroy(config);
	log_destroy(log);

    return 0;
}