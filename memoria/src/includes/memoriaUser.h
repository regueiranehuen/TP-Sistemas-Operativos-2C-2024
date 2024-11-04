#ifndef MEMORIAUSER_H
#define MEMORIAUSER_H

#include "utils/includes/estructuras.h"
#include "utils/includes/sockets.h"
#include "utils/includes/serializacion.h"

extern t_memoria* mem;

t_memoria* inicializar_memoria(t_esquema_particion esquema, int tamano, int* lista_particiones, int num_particiones);
void liberar_memoria(t_memoria* memoria);
void unir_bloques_libres(t_memoria* memoria);
int asignar_memoria_fija(t_memoria* memoria, int pid);
int asignar_memoria_dinamica(t_memoria* memoria, int id_proceso, int tamano_requerido, t_estrategia_busqueda estrategia);
int asignar_memoria(t_memoria* memoria, int pid, int tamano);
void liberar_memoria_proceso(t_memoria* memoria, int pid);
void mostrar_estado_memoria(t_memoria* memoria);


#endif