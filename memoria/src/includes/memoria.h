#ifndef MEMORIA_H
#define MEMORIA_H

#include <utils/includes/sockets.h>
#include "utils/includes/serializacion.h"
#include <commons/config.h>
#include "main.h"
#include "commCpu.h"

void atender_conexiones(int socket_cliente);
uint32_t obtener_base();

#endif