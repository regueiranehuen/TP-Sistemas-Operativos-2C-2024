#include "includes/planificacion.h"

t_pcb *fifo(t_queue *cola_proceso)
{

    if (cola_proceso != NULL)
    {
        t_pcb *pcb = queue_pop(cola_proceso);
        return pcb;
    }
    return NULL;
}



t_tcb* prioridades(t_queue* cola_hilos) {
    if (!queue_is_empty(cola_hilos)) {
        t_tcb* hilo_actual = queue_pop(cola_hilos);  // Sacamos el primer hilo (actual)

        // Recorremos el resto de la cola para comparar con el hilo actual
        for (int i = 0; i < queue_size(cola_hilos); i++) {
            t_tcb* hilo_siguiente = queue_pop(cola_hilos);  // Siguiente hilo a comparar

            if (hilo_siguiente == NULL){ // Si hay un único hilo en la cola

                return hilo_actual;
            } 

            // Si la prioridad del siguiente es menor, actualizamos el hilo seleccionado
            if (hilo_siguiente->prioridad < hilo_actual->prioridad) {
                queue_push(cola_hilos, hilo_actual);  // Reinserta el actual
                hilo_actual = hilo_siguiente;  // Actualiza el hilo seleccionado
            } 
            else {
                queue_push(cola_hilos, hilo_siguiente);  // Reinserta el siguiente si no es seleccionado
            }
        }
        
        // El hilo actual es el de menor prioridad, ya no se reinserta
        return hilo_actual;
    }
    return NULL;
}


t_tcb*round_robbin(t_queue*cola_hilos,t_config*config){

    if (!queue_is_empty(cola_hilos)){
        int quantum=config_get_int_value(config,"QUANTUM");  // Cantidad máxima de tiempo que obtiene la CPU un proceso/hilo (EN MILISEGUNDOS)
        
        t_tcb*tcb=queue_pop(cola_hilos); // Sacar el primer hilo de la cola
        
        // Simular que el hilo está en ejecución durante el tiempo del quantum
        usleep(quantum * 1000); // usleep trata con microsegundos, 1 microsegundo es igual a 1000 milisegundos

        if (strcmp(tcb->estado,"EXIT")!=0){ // Si el hilo no terminó de realizar su tarea
            queue_push(cola_hilos,tcb); // lo reenviamos al final de la cola
        }

        return tcb;
        
    }
    return NULL;

}