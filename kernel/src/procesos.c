#include "includes/procesos.h"
/*
Al iniciar el módulo Kernel, se creará un proceso inicial para que esté lo planifique y para poder inicializarlo, se 
requerirá entonces que este módulo reciba dos argumentos adicionales del main: el nombre del archivo de pseudocódigo que deberá ejecutar 
y el tamaño del proceso para ser inicializado en Memoria, el TID 0 creado por este proceso tendrá la prioridad máxima 0 (cero).
Ejemplo de comando de ejecución:
./bin/kernel [archivo_pseudocodigo] [tamanio_proceso] [...args]
*/

t_pcb* crear_pcb(){
static int pid = 0;
t_pcb* pcb= malloc(sizeof(t_pcb));
t_list* lista_tids= list_create();
pcb->pid=pid;
pcb->tids=lista_tids;
pid ++;
return pcb;
}
t_tcb* crear_tcb(t_pcb* pcb){
static int tid = 0;
t_tcb* tcb = malloc(sizeof(t_tcb));
tcb -> tid = tid;
list_add(pcb->tids,tcb->tid);
if(tid==0){
    tcb->prioridad=0;
}
tid ++;
return tcb;
}

t_proceso iniciar_kernel (char* archivo_pseudocodigo, int tamanio_proceso){
t_pcb* pcb = crear_pcb();
t_tcb* tcb = crear_tcb();
t_proceso proceso;
proceso.pcb=pcb;
proceso.tcb=tcb;
return proceso;
}