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


t_tcb *fifo(t_queue *cola_hilo)
{

    if (cola_hilo!= NULL)
    {
        t_tcb *tcb = queue_pop(cola_hilo);
        return tcb;
    }
    return NULL;
}

// t_tcb* prioridades_sin_lista(t_queue* cola_hilos) {
//     //t_tcb* hilo_seleccionado = NULL;
//     t_tcb* hilo_actual = queue_pop(cola_hilos);
//     t_tcb* hilo_sig = queue_pop(cola_hilos);

//     for (int i = 0; i < queue_size(cola_hilos); i++) {
        
//         if (hilo_sig==NULL){
//             queue_push
//         }

//         // Seleccionamos el hilo con la prioridad más baja
//         if (hilo_actual->prioridad < hilo_seleccionado->prioridad) {
//             if (hilo_seleccionado != NULL) {
//                 queue_push(cola_hilo, hilo_seleccionado);  // Reinserta el anterior hilo seleccionado
//             }
//             hilo_seleccionado = hilo_actual;  // Actualizamos el hilo seleccionado
//         } else {
//             queue_push(cola_hilos, hilo_actual);  // Reinserta el hilo no seleccionado
//         }

//         hilo_actual = queue_pop(cola_hilos);
//         hilo_sig = queue_pop(cola_hilos);

       
//     }
    
//     return hilo_seleccionado;
// }

t_tcb* prioridades(t_queue* cola_hilos) {
    if (!queue_is_empty(cola_hilos)) {
        t_tcb* hilo_actual = queue_pop(cola_hilos);  // Sacamos el primer hilo (actual)

        // Recorremos el resto de la cola para comparar con el hilo actual
        for (int i = 0; i < queue_size(cola_hilos); i++) {
            t_tcb* hilo_siguiente = queue_pop(cola_hilos);  // Siguiente hilo a comparar

            // Si la prioridad del siguiente es menor, actualizamos el hilo seleccionado
            if (hilo_siguiente->prioridad < hilo_actual->prioridad) {
                queue_push(cola_hilos, hilo_actual);  // Reinserta el actual
                hilo_actual = hilo_siguiente;  // Actualiza el hilo seleccionado
            } else {
                queue_push(cola_hilos, hilo_siguiente);  // Reinserta el siguiente si no es seleccionado
            }
        }
        
        // El hilo actual es el de menor prioridad, ya no se reinserta
        return hilo_actual;
    }
    return NULL;
}




/*
void planificadorLargoPlazo()
{

}

void planificadorCortoPlazo()
{

}
*/
