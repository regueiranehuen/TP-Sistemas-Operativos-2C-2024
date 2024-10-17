#ifndef CICLODEINSTRUCCION_H
#define CICLODEINSTRUCCION_H

#include "server.h"
#include "utils/includes/estructuras.h"

extern t_instruccion instruccion;
extern bool seguir_ejecutando;

extern int tid_exec;
extern int pid_exec;


void ciclo_de_instruccion(t_contexto_pid* contextoPid, t_contexto_tid* contextoTid);
t_instruccion* fetch(t_contexto_tid*contexto);
void pedir_instruccion_memoria(uint32_t tid, uint32_t pc, t_log *logg);
op_code decode(t_instruccion *instruccion);
void execute(t_contexto_tid*contexto,op_code instruccion_nombre, t_instruccion* instruccion);
void checkInterrupt(int tid,int pid);
void esperar_ok_kernel(void);

#endif