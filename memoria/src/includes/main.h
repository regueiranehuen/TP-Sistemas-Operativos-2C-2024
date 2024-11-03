#ifndef MAIN_H
#define MAIN_H

#include "server.h"
#include "auxiliaresMem.h"

extern int estado_cpu;
extern t_log* logger;
extern t_config* config;

extern int tamanio_memoria;
extern int retardo_restp;
extern int esquema;
extern char* algoritmo_busqueda;
extern char* particiones;

void hilo_recibe_cpu();

#endif