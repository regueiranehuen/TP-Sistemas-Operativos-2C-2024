#ifndef CICLODEINSTRUCCION_H
#define CICLODEINSTRUCCION_H

#include "utils/sockets.h"
#include "main.h"

t_instruccion* instruccion;
t_log* log_cpu;
int hay_interrupcion;


void ciclo_de_instruccion();
t_instruccion* fetch(uint32_t tid, uint32_t pc);
void pedir_instruccion_memoria(uint32_t tid, uint32_t pc, t_log *logg);
op_code decode(t_instruccion *instruccion);
void execute(op_code instruccion_nombre, t_instruccion* instruccion);
void checkInterrupt(uint32_t tid);
#endif