#ifndef MMU_H
#define MMU_H

#include "server.h"

uint32_t traducir_direccion_logica(uint32_t direccion_logica);
void actualizar_contexto_en_memoria(t_contexto *contexto);
void notificar_kernel_terminacion(int tid, int razon_salida);

#endif