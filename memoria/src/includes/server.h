#ifndef SERVER_H
#define SERVER_H

#include <utils/includes/sockets.h>
#include <commons/config.h>

int servidor_memoria_kernel (t_log* log, t_config* config);
int cliente_memoria_filesystem (t_log* log, t_config* config);

#endif