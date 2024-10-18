#ifndef COMMCPU_H
#define COMMCPU_H

#include "server.h"
#include "utils/includes/estructuras.h"

void recibir_cpu(int SOCKET_CLIENTE_CPU);
t_contexto_pid* obtener_contexto_pid(int pid);
t_contexto_tid* obtener_contexto_tid(int pid, int tid);

void actualizar_contexto_program_counter(int pid, int tid, uint32_t pc);
void actualizar_contexto_reg(int pid, int tid, t_registros_cpu* reg);

#endif
