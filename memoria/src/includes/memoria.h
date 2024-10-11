#ifndef MEMORIA_H
#define MEMORIA_H

#include <utils/includes/sockets.h>
#include <commons/config.h>
#include "main.h"

void atender_conexiones(int socket_cliente);

typedef enum{
    ENUM_DUMP_MEMORY,
    ENUM_PROCESS_EXIT, 
    ENUM_PROCESS_CREATE,
    ENUM_THREAD_CREATE,
    ENUM_THREAD_EXIT,
    ENUM_THREAD_CANCEL,
}code_operacion;

#endif