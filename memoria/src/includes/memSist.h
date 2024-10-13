#ifndef MEMSIST_H
#define MEMSIST_H

#include "server.h"

//Variables
int longitud_maxima=200;
int parametros_maximos=6;
int instrucciones_maximas=200;
t_instruccion* instrucciones[200];

t_list** listas_instrucciones;

//Funciones
void cargar_instrucciones_desde_archivo(char* nombre_archivo,  uint32_t pid);
void copiarBytes(uint32_t tamanio, t_contexto *contexto);


#endif
