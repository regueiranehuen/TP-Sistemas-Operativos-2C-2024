#ifndef MEMUSUARIO_H
#define MEMUSUARIO_H

#include "server.h"

extern void* memoria;
extern t_list* lista_particiones;
extern int tamanio_memoria;

typedef struct{
    bool ocupada;
    uint32_t base;
    uint32_t limite;
    int tamanio;
    int pid;
}t_particiones;


void inicializar_Memoria(t_config *config);
void cargar_particiones_lista(char **particiones);
t_particiones* inicializar_proceso(int pid, int tamanio_proceso, t_config* config);

t_particiones* busqueda_fija(int pid,int tamanio_proceso,char* algoritmo_busqueda,int tamanio_lista);
t_particiones* busqueda_dinamica(int pid, int tamanio_proceso, char* algoritmo_busqueda,int tamanio_lista);

t_particiones* fija_first(int pid, int tamanio_proceso, int tamanio_lista);
t_particiones* fija_best(int pid, int tamanio_proceso, int tamanio_lista);
t_particiones* fija_worst(int pid, int tamanio_proceso, int tamanio_lista);

t_particiones* dinamica_first(int pid, int tamanio_proceso, int tamanio_lista);
t_particiones* dinamica_best(int pid, int tamanio_proceso, int tamanio_lista);
t_particiones* dinamica_worst(int pid, int tamanio_proceso, int tamanio_lista);

void liberar_espacio_proceso(int pid);
void fusionar_particiones_libres(t_list* lista_particiones,t_particiones* particion_actual,int indice);

t_list* lectura_datos_proceso(int pid);

t_particiones* busqueda_particion(int pid);
//char* generar_nombre_archivo(int pid, int tid);

uint32_t leer_Memoria(uint32_t direccionFisica);
int escribir_Memoria(t_write_mem* info);
void acomodar_particion_siguiente(t_particiones *particion, int index, int tamanio_lista);

#endif