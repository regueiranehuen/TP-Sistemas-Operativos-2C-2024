#ifndef MAIN_H
#define MAIN_H

#include "cliente.h"
#include "procesos.h"
#include "funcionesAuxiliares.h"
#include "planificacion.h"

void liberar_espacio(t_log *log, t_config *config, sockets_kernel *sockets);
void inicializar_mutex();
void destroy_mutex();
void inicializar_semaforo();
void destroy_semaforo();

#endif