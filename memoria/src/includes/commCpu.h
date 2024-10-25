#ifndef COMMCPU_H
#define COMMCPU_H

#include "server.h"
#include "utils/includes/estructuras.h"

void* recibir_cpu(void*args);
t_contexto_pid* obtener_contexto_pid(int pid);
t_contexto_tid* obtener_contexto_tid(int pid, int tid);

void actualizar_contexto(int pid, int tid, t_registros_cpu* reg);
t_contexto_tid* inicializar_contexto_tid(t_contexto_pid* cont,int tid);
t_contexto_pid*inicializar_contexto_pid(int pid,uint32_t base, uint32_t limite);

#endif
