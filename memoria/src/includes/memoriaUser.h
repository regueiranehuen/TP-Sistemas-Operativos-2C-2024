#ifndef MEMORIAUSER_H
#define MEMORIAUSER_H

#include "utils/includes/estructuras.h"
#include <utils/includes/sockets.h>
#include "utils/includes/serializacion.h"

#define MEM_SIZE 1024 // Lo debe sacar de la config pero lo borraron ni idea
#define TAMANO_PARTICION_FIJA 64 // Lo debe sacar de la config pero lo borraron ni idea


typedef enum {
    PARTICION_FIJA,
    PARTICION_DINAMICA
} t_esquema_particion;

typedef enum {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
} t_estrategia_busqueda;

//sacarlo tambien de config
t_estrategia_busqueda estrategia;

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
    int tamano_memoria;
} t_memoria;


#endif