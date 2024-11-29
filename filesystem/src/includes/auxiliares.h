#ifndef AUXILIARES_H
#define AUXILIARES_H

#include "estructuras.h"

void inicializar_estructuras(void);
uint32_t bytes_a_escribir(t_args_dump_memory* info,uint32_t bytes_escritos);
void detectar_cierre(int socket_cliente);

#endif