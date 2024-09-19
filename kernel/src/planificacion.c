#include "includes/planificacion.h"

t_pcb *fifo_pcb(t_queue *cola_proceso)
{

    if (cola_proceso != NULL)
    {
        t_pcb *pcb = queue_pop(cola_proceso);
        return pcb;
    }
    return NULL;
}

t_tcb* fifo_tcb(t_queue * cola_hilos)
{

    if (cola_hilos!=NULL)
    {
        t_tcb*tcb=queue_pop(cola_hilos);
        return tcb;
    }
    return NULL;
}



t_tcb* prioridades(t_queue* cola_hilos) 
{
    if (!queue_is_empty(cola_hilos)) 
    {
        t_tcb* hilo_actual = queue_pop(cola_hilos);  // Sacamos el primer hilo (actual)

        // Recorremos el resto de la cola para comparar con el hilo actual
        for (int i = 0; i < queue_size(cola_hilos); i++) 
        {
            t_tcb* hilo_siguiente = queue_pop(cola_hilos);  // Siguiente hilo a comparar

            if (hilo_siguiente == NULL)
            { // Si hay un único hilo en la cola

                return hilo_actual;
            } 

            // Si la prioridad del siguiente es menor, actualizamos el hilo seleccionado
            if (hilo_siguiente->prioridad < hilo_actual->prioridad) 
            {
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


/// A ROUND ROBIN LE TENGO QUE AGREGAR COMO PARAMETRO LA PRIORIDAD DE LA COLA DE PRIORIDAD

t_tcb*round_robbin(t_cola_prioridad*cola_prioridad){

    if (!queue_is_empty(cola_prioridad->cola)){
        int quantum=config_get_int_value(config,"QUANTUM");  // Cantidad máxima de tiempo que obtiene la CPU un proceso/hilo (EN MILISEGUNDOS)
        
        t_tcb*tcb=queue_pop(cola_prioridad->cola); // Sacar el primer hilo de la cola
        
        


        //tcb->estado=TCB_EXECUTE;
        // Simular que el hilo está en ejecución durante el tiempo del quantum
        usleep(quantum * 1000); // usleep trata con microsegundos, 1 microsegundo es igual a 1000 milisegundos



        if (tcb->estado!=TCB_EXECUTE){ // Si el hilo no terminó de realizar su tarea
            queue_push(cola_prioridad->cola,tcb); // lo reenviamos al final de la cola
        }

        return tcb;
        
    }
    return NULL;

}


// Colas Multinivel
// Se elegirá al siguiente hilo a ejecutar según el siguiente esquema de colas multinivel:
// - Se tendrá una cola por cada nivel de prioridad existente entre los hilos del sistema.
// - El algoritmo entre colas es de prioridades sin desalojo.
// - Cada cola implementará un algoritmo Round Robin con un quantum (Q) definido por archivo de configuración.
// - Al llegar un hilo a ready se posiciona siempre al final de la cola que le corresponda.


// typedef struct
// {
//     int prioridad;
//     t_queue *cola;
// } t_cola_prioridad;


t_tcb*colas_multinivel(t_list * lista_colas_prioridad, t_config * config)
{
    for (int i = 0; i< list_size(lista_colas_prioridad); i++)
    {
        t_cola_prioridad * cola_prioridad_actual = cola_prioridad(lista_colas_prioridad, i);
        while (cola_prioridad_actual->cola!=NULL)
        {
            round_robbin(cola_prioridad_actual);
        }
    }

    return NULL;
}