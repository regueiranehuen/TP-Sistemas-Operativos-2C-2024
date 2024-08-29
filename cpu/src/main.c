#include "server.h"

int main(void) {
	logger = log_create("cpu_servidor.log", "cpu_servidor", 1, LOG_LEVEL_DEBUG);
	modo_server(logger);
}


