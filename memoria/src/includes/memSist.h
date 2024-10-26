#ifndef MEMSIST_H
#define MEMSIST_H

#include "server.h"
#include "utils/includes/estructuras.h"

//Variables
extern pthread_mutex_t mutex_lista_instruccion;
extern int longitud_maxima;
extern int parametros_maximos;
extern int instrucciones_maximas;


void cargar_instrucciones_desde_archivo(char* nombre_archivo, int pid, int tid);
void copiarBytes(uint32_t tamanio, t_contexto_pid *contexto);
void finalizar_hilo(int tid, int pid);
void eliminar_elemento_por_tid(int tid, t_list* contextos_tids);



#endif
