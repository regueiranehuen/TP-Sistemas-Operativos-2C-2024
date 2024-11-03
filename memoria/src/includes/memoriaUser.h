#ifndef MEMORIAUSER_H
#define MEMORIAUSER_H

#include "utils/includes/estructuras.h"
#include "utils/includes/sockets.h"
#include "utils/includes/serializacion.h"

typedef enum {
    PARTICION_FIJA,
    PARTICION_DINAMICA
} t_esquema_particion;

typedef enum {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
} t_estrategia_busqueda;

// Ver si conviene usar una copia o los propios contextos
typedef struct {
    int pid;           
    int base;          
    int limite;        
} t_segmento_usuario;

typedef struct {
    t_segmento_usuario* segmentos;
    int num_segmentos;
} t_tabla_segmentos;

typedef struct {
    int base;    
    int tamano;        
} t_segmento_libre;

typedef struct {
    t_segmento_libre* bloques_libres;
    int num_bloques_libres;
} t_tabla_libres;

typedef struct {
     void* memoria;                  
    t_tabla_segmentos tabla_segmentos;
    t_tabla_libres tabla_libres;
    t_esquema_particion esquema;
    t_estrategia_busqueda estrategia;
    int tamano_memoria;
    int* lista_particiones;
    int num_particiones;
} t_memoria;

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