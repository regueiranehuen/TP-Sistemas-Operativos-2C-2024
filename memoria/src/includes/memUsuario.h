#ifndef MEMUSUARIO_H
#define MEMUSUARIO_H

#include "includes/server.h"

extern void* memoria;
extern t_list* lista_particiones;
extern int tamanio_memoria;

typedef struct{
    bool ocupada;
    int base;
    int limite;
    int tamanio;
    int pid;
}t_particiones;

#endif