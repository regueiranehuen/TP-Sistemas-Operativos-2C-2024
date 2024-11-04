#ifndef MAIN_H
#define MAIN_H

#include "server.h"
#include "auxiliaresMem.h"
#include "commCpu.h"
#include "memoriaUser.h"

extern int estado_cpu;
extern t_log* logger;
extern t_config* config;


extern int tamanio_memoria;
extern int esquema;
extern char* algoritmo_busqueda;
extern char* particiones;



void hilo_recibe_cpu();
void leer_config(char* path);
int* parse_partitions(const char* partition_string, int* count) ;

#endif