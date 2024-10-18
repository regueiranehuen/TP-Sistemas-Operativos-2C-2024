#ifndef COMMCPU_H
#define COMMCPU_H

#include "server.h"
#include "memSist.h"
#include "utils/includes/estructuras.h"

void recibir_cpu(int SOCKET_CLIENTE_CPU);
t_contexto_pid* obtener_contexto_pid(int pid);
t_contexto_tid* obtener_contexto_tid(int pid, int tid);

#endif
