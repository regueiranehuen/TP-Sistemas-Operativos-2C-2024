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

t_tcb *fifo_tcb(t_pcb *pcb)
{

    if (pcb->cola_hilos_ready != NULL)
    {
        t_tcb *tcb = queue_pop(pcb->cola_hilos_ready);
        return tcb;
    }
    return NULL;
}

// Prioridades
// Se elegirá al siguiente hilo a ejecutar según cual tenga el número de prioridad más bajo, siendo 0 la máxima prioridad. En caso de tener varios hilos con la misma prioridad más alta, se desempata por FIFO. Se pide implementar este esquema sin desalojo.

t_tcb *prioridades(t_pcb *pcb)
{
    if (!queue_is_empty(pcb->cola_hilos_ready))
    {
        t_tcb *hilo_actual = queue_pop(pcb->cola_hilos_ready); // Sacamos el primer hilo (actual)

        // Recorremos el resto de la cola para comparar con el hilo actual
        for (int i = 0; i < queue_size(pcb->cola_hilos_ready); i++)
        {
            t_tcb *hilo_siguiente = queue_pop(pcb->cola_hilos_ready); // Siguiente hilo a comparar

            if (hilo_siguiente == NULL)
            { // Si hay un único hilo en la cola

                return hilo_actual;
            }

            // Si la prioridad del siguiente es menor, actualizamos el hilo seleccionado
            if (hilo_siguiente->prioridad < hilo_actual->prioridad)
            {
                queue_push(pcb->cola_hilos_ready, hilo_actual); // Reinserta el actual
                hilo_actual = hilo_siguiente;                   // Actualiza el hilo seleccionado
            }
            else
            {
                queue_push(pcb->cola_hilos_ready, hilo_siguiente); // Reinserta el siguiente si no es seleccionado
            }
        }

        // El hilo actual es el de menor prioridad, ya no se reinserta
        return hilo_actual;
    }
    return NULL;
}

void round_robin(t_queue *cola)
{
    if (!queue_is_empty(cola))
    {
        int quantum = config_get_int_value(config, "QUANTUM"); // Cantidad máxima de tiempo que obtiene la CPU un proceso/hilo (EN MILISEGUNDOS)

        t_tcb *tcb = queue_pop(cola); // Sacar el primer hilo de la cola

        ejecucion(tcb,cola,sockets->sockets_cliente_cpu->socket_Dispatch);

        // Simular que el hilo está en ejecución durante el tiempo del quantum
        usleep(quantum * 1000); // usleep trata con microsegundos, 1 microsegundo es igual a 1000 milisegundos
        
        code_operacion rtaCPU;
        code_operacion fin_quantum_rr=FIN_QUANTUM_RR;
        send(sockets->sockets_cliente_cpu->socket_Interrupt,&fin_quantum_rr,sizeof(fin_quantum_rr),0);


        // atenderSyscall()


        recv(sockets->sockets_cliente_cpu->socket_Interrupt,&rtaCPU,sizeof(rtaCPU),0);


        

        if (rtaCPU == THREAD_EXIT_) // Al código de operación que está en la branch memoria_cpu le agregué un guión bajo pq se llamaría igual que la syscall. Ojo con eso
        {
            THREAD_EXIT();
        }
        else
        {
            tcb->estado = TCB_READY;
            queue_push(cola, tcb); // Lo reinsertas si no ha terminado
        }
    }
}

void colas_multinivel(t_pcb *pcb, int prioridad)
{

    if (list_is_empty(pcb->colas_hilos_prioridad_ready))
    {
        return;
    }

    else
    {
        int priori = nueva_prioridad(pcb->colas_hilos_prioridad_ready, prioridad);
        t_cola_prioridad *cola_prioridad_actual = cola_prioridad(pcb->colas_hilos_prioridad_ready, priori);
        if (!queue_is_empty(cola_prioridad_actual->cola))
        {
            round_robin(cola_prioridad_actual->cola);
        }
        else{
            int prioridadSig = priori + 1;
            colas_multinivel(pcb, prioridadSig);
        }
        
    }
}

int nueva_prioridad(t_list *colas_hilos_prioridad_ready, int priori_actual)
{

    for (int i = 0; i <= priori_actual && i < list_size(colas_hilos_prioridad_ready); i++)
    {
        t_cola_prioridad *cola_prioridad_i = list_get(colas_hilos_prioridad_ready, i);

        if (cola_prioridad_i != NULL && !queue_is_empty(cola_prioridad_i->cola))
        {
            return i;
        }
    }

    return -1; // no deberia pasar
}