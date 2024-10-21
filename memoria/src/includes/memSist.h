#ifndef MEMSIST_H
#define MEMSIST_H

#include "server.h"
#include "utils/includes/estructuras.h"

//Variables
int longitud_maxima=200;
int parametros_maximos=6;
int instrucciones_maximas=200;
t_instruccion* instrucciones[200];


void cargar_instrucciones_desde_archivo(char* nombre_archivo, int pid, int tid);
void copiarBytes(uint32_t tamanio, t_contexto_pid *contexto);



#endif
