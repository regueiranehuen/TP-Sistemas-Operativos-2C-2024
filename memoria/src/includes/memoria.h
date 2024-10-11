#ifndef MEMORIA_H
#define MEMORIA_H

#include <utils/includes/sockets.h>
#include <commons/config.h>
#include "main.h"

void atender_conexiones(int socket_cliente);

typedef enum{
    DUMP_MEMORIA,
    PROCESS_EXIT_AVISO, 
    PROCESS_CREATE_AVISO,
    THREAD_CREATE_AVISO,
    THREAD_ELIMINATE_AVISO
}code_operacion;

#endif