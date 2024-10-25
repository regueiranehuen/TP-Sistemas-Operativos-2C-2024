#ifndef CICLODEINSTRUCCION_H
#define CICLODEINSTRUCCION_H

#include "server.h"
#include "utils/includes/estructuras.h"
#include "utils/includes/semaforosCompartidos.h"

extern t_instruccion instruccion;
extern bool seguir_ejecutando;

extern int tid_exec;
extern int pid_exec;

void ciclo_de_instruccion(t_contexto_pid *contextoPid, t_contexto_tid *contextoTid);
void checkInterrupt(t_contexto_tid* contextoTid);
t_instruccion* fetch(t_contexto_tid*contexto);
op_code decode(t_instruccion *instruccion);
void execute(t_contexto_pid*contextoPid,t_contexto_tid* contextoTid ,op_code instruccion_nombre, t_instruccion* instruccion);
void pedir_instruccion_memoria(int tid, int pid, uint32_t pc);

#endif

