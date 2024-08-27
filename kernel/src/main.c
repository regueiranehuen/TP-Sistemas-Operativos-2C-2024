#include "includes/main.h"

int main(int argc, char* argv[]) {

t_log* log;
t_config* config;

log= log_create("tp0.log", "tp0", true, LOG_LEVEL_INFO);
config= config_create("kernel.config");

cliente_Memoria_Kernel(log,config);

    return 0;
}
