#ifndef AUXILIARESMEM_H
#define AUXILIARESMEM_H

#include "server.h"
#include "utils/includes/estructuras.h"
#include "../includes/memUsuario.h"

void inicializar_estructuras();
void inicializar_semaforos();
void inicializar_mutex();
void destruir_mutex();
void destruir_semaforos();
void liberar_instruccion(t_instruccion_tid_pid*instruccion);
void eliminar_contexto_pid(t_contexto_pid*contexto_pid);
void eliminar_estructuras();

#endif