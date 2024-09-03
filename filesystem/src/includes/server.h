#ifndef SERVER_H
#define SERVER_H

#include "utils/includes/sockets.h"

int servidor_FileSystem_Memoria(t_log* log, t_config* config);
void* funcion_hilo_servidor(void* void_args);
int hilo_filesystem(t_log* log, t_config* config);

#endif