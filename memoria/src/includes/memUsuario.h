#ifndef MEMUSUARIO_H
#define MEMUSUARIO_H

#include "includes/server.h"

extern void* memoria;
extern t_list* lista_particiones;
extern int tamanio_memoria;

typedef struct{
    bool ocupada;
    int base;
    int limite;
    int tamanio;
    int pid;
}t_particiones;

void inicializar_Memoria(t_config *config);
void cargar_particiones_lista(char **particiones);
int inicializar_proceso(int pid, int tamanio_proceso, t_config* config);

int busqueda_fija(int pid,int tamanio_proceso,char* algoritmo_busqueda,int tamanio_lista);
int busqueda_dinamica(int pid, int tamanio_proceso, char* algoritmo_busqueda,int tamanio_lista);

int fija_first(int pid, int tamanio_proceso, int tamanio_lista);
int fija_best(int pid, int tamanio_proceso, int tamanio_lista);
int fija_worst(int pid, int tamanio_proceso, int tamanio_lista);

int dinamica_first(int pid, int tamanio_proceso, int tamanio_lista);
int dinamica_best(int pid, int tamanio_proceso, int tamanio_lista);
int dinamica_worst(int pid, int tamanio_proceso, int tamanio_lista);

#endif