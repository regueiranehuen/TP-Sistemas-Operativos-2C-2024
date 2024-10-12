#ifndef MMU_H
#define MMU_H

#include "server.h"

int traducir_direccion_logica(int direccion_logica);
void actualizar_contexto_en_memoria(t_contexto *contexto);
void notificar_kernel_terminacion(int tid, int razon_salida);

#endif