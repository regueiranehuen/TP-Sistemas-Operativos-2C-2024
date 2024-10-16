#ifndef CICLODEINSTRUCCION_H
#define CICLODEINSTRUCCION_H

#include "server.h"
#include "utils/includes/estructuras.h"

extern t_instruccion instruccion;
extern bool seguir_ejecutando;


void ciclo_de_instruccion(t_log* loggs);
t_instruccion* fetch(uint32_t tid, uint32_t pc);
void pedir_instruccion_memoria(uint32_t tid, uint32_t pc, t_log *logg);
op_code decode(t_instruccion *instruccion);
void execute(op_code instruccion_nombre, t_instruccion* instruccion);
void checkInterrupt(uint32_t tid);
t_paquete_code_operacion* recibir_paquete_code_operacion(int socket);
t_tid_pid* recepcionar_tid_pid_code_op(t_paquete_code_operacion* paquete);
t_contexto* recibir_contexto_para_thread_execute(int socket_memoria, uint32_t tid);
void enviar_contexto_a_memoria(int socket_memoria, t_contexto* contexto);




#endif