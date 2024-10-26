#ifndef MMU_H
#define MMU_H

#include "server.h"

uint32_t traducir_direccion_logica(t_contexto_tid*contexto_tid,t_contexto_pid *contexto_pid,uint32_t direccion_logica);

#endif