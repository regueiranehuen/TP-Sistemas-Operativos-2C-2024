#ifndef MEMSIST_H
#define MEMSIST_H

#include "server.h"
#include "utils/includes/estructuras.h"

//Variables
extern int longitud_maxima;
extern int parametros_maximos;
extern int instrucciones_maximas;


void cargar_instrucciones_desde_archivo(char* nombre_archivo, int pid, int tid);
void copiarBytes(uint32_t tamanio, t_contexto_pid *contexto);



#endif
