#include "includes/planificacion.h"

int estado_kernel;
sem_t semaforo_new_ready_procesos;
sem_t semaforo_cola_new_procesos;
sem_t semaforo_cola_exit_procesos;
sem_t semaforo_cola_new_hilos;
sem_t semaforo_cola_exit_hilos;
sem_t sem_lista_prioridades;
t_queue *cola_new;
t_queue *cola_ready;
t_list *lista_pcbs;
pthread_mutex_t mutex_pthread_join;
t_pcb *proceso_exec;
t_config *config;
t_log *logger;
t_list *lista_mutex;
sockets_kernel *sockets;
pthread_mutex_t mutex_conexion_cpu;
t_queue *cola_exit;

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

/*
Planificador de Largo Plazo
El Kernel será el encargado de gestionar las peticiones a la memoria para la creación y eliminación
de procesos e hilos.
Creación de procesos
Se tendrá una cola NEW que será administrada estrictamente por FIFO para la creación de procesos.
Al llegar un nuevo proceso a esta cola y la misma esté vacía se enviará un pedido a
Memoria para inicializar el mismo,
si la respuesta es positiva se crea el TID 0 de ese proceso y se lo pasa al estado READY
y se sigue la misma lógica con el proceso que sigue.
Si la respuesta es negativa (ya que la Memoria no tiene espacio suficiente para inicializarlo)
se deberá esperar la finalización de otro proceso
para volver a intentar inicializarlo.
Al llegar un proceso a esta cola y haya otros esperando, el mismo simplemente se encola.
*/

void *funcion_new_ready_procesos(void *void_args)
{

    while (estado_kernel != 0)
    {
        new_a_ready_procesos();
    }
    return NULL;
}

void *funcion_procesos_exit(void *void_args)
{
    while (estado_kernel != 0)
    {
        proceso_exit();
    }
    return NULL;
}

void *funcion_hilos_exit(void *void_args)
{
    while (estado_kernel != 0)
    {
        // hilo_exit(pcb) Me imagino que habra que hacer un hilo por proceso
    }
    return NULL;
}

void *funcion_new_ready_hilos(void *void_args)
{
    while (estado_kernel != 0)
    {
        // new_a_ready_hilos(pcb) Lo mismo que arriba
    }
    return NULL;
}

void hilo_atender_syscalls()
{
    pthread_t hilo_syscalls;
    int resultado;

    resultado = pthread_create(&hilo_syscalls, NULL, atender_syscall, NULL);
    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo_atender_syscall");
        return;
    }
    pthread_detach(hilo_syscalls);
}

void hilo_planificador_largo_plazo()
{
    pthread_t hilo_plani_largo_plazo;
    int resultado;

    resultado = pthread_create(&hilo_plani_largo_plazo, NULL, planificador_largo_plazo, NULL);
    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo_planificador_largo_plazo");
        return;
    }
    pthread_detach(hilo_plani_largo_plazo);
}

void *planificador_largo_plazo(void *void_args)
{
    pthread_t hilo_new_ready_procesos;
    pthread_t hilo_exit_procesos;
    pthread_t hilo_new_ready_hilos;
    pthread_t hilo_exit_hilos;

    int resultado;

    resultado = pthread_create(&hilo_new_ready_procesos, NULL, funcion_new_ready_procesos, NULL);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo new_ready");
        return NULL;
    }

    resultado = pthread_create(&hilo_exit_procesos, NULL, funcion_procesos_exit, NULL);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo procesos_exit");
        return NULL;
    }

    resultado = pthread_create(&hilo_new_ready_hilos, NULL, funcion_new_ready_hilos, NULL);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo new_ready_hilos");
        return NULL;
    }

    resultado = pthread_create(&hilo_exit_hilos, NULL, funcion_hilos_exit, NULL);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo hilos_exit");
        return NULL;
    }

    pthread_detach(hilo_new_ready_procesos);
    pthread_detach(hilo_exit_procesos);
    pthread_detach(hilo_new_ready_hilos);
    pthread_detach(hilo_exit_hilos);
    return NULL;
}

void *atender_syscall(void *void_args)
{ // el problema es que todas las funciones que tengan recv con cpu tienen la posiblidad de recibir la syscall entonces hay que buscar la manera que solamente lo reciba el que corresponda

    syscalls syscall;

    while (estado_kernel != 0)
    {

        recv(sockets->sockets_cliente_cpu->socket_Dispatch, &syscall, sizeof(syscall), 0);

        switch (syscall)
        {

        case ENUM_PROCESS_CREATE:
            //PROCESS_CREATE(); 
            break;
        case ENUM_PROCESS_EXIT:
            PROCESS_EXIT();
            break;
        case ENUM_THREAD_CREATE:
            // THREAD_CREATE(); lo mismo que lo de abajo
            break;
        case ENUM_THREAD_JOIN:
            // THREAD_JOIN(tid);hay que ver como recibir el tid de cpu
            break;
        case ENUM_THREAD_CANCEL:
            // THREAD_CANCEL(tid);lo mismo que arriba
            break;
        case ENUM_MUTEX_CREATE:
            MUTEX_CREATE();
            break;
        case ENUM_MUTEX_LOCK:
            // MUTEX_LOCK(mutex_id);la misma situación
            break;
        case ENUM_MUTEX_UNLOCK:
            // MUTEX_UNLOCK(mutex_id); la misma situación
            break;
        case ENUM_IO:
            // IO(milisegundos); la misma situación
            break;
        case ENUM_DUMP_MEMORY:
            DUMP_MEMORY();
            break;
        default:
            printf("Syscall no válida.\n");
            break;
        }
    }
    return NULL;
}

t_tcb* prioridades (t_pcb* pcb){

if(!list_is_empty(pcb->lista_prioridad_ready)){

t_tcb* tcb_prioritario = list_remove(pcb->lista_prioridad_ready,0);

return tcb_prioritario;

}

return NULL;

}

void round_robin(t_queue *cola) {
    if (!queue_is_empty(cola)) {
        int quantum = config_get_int_value(config, "QUANTUM");
        t_tcb *tcb = queue_pop(cola); // Sacar el primer hilo de la cola

        tcb->estado = TCB_EXECUTE;

        int tiempo_ejecutado = 0;

        while (tiempo_ejecutado < quantum) {
            // Enviar el hilo a la CPU y esperar la devolución
            t_paquete *paquete = crear_paquete();
            agregar_a_paquete(paquete, &tcb->tid, sizeof(tcb->tid));
            agregar_a_paquete(paquete, &tcb->pid, sizeof(tcb->pid));
            enviar_paquete(paquete, sockets->sockets_cliente_cpu->socket_Dispatch);
            eliminar_paquete(paquete);

            // Esperar la devolución desde la CPU
            t_list *devolucionCPU = recibir_paquete(sockets->sockets_cliente_cpu->socket_Dispatch);
            code_operacion motivo_devolucion = *(code_operacion *)list_get(devolucionCPU, 0);

            if (motivo_devolucion == THREAD_EXIT_) {
                THREAD_EXIT();
                break;
            } else if (es_motivo_devolucion(motivo_devolucion)) {
                // Si se debe replanificar, cambiar estado y volver a poner en la cola
                tcb->estado = TCB_READY;
                queue_push(cola, tcb);
                break;
            }

            tiempo_ejecutado++;
            usleep(1000); // Dormir un corto período para no consumir CPU innecesariamente
        }

        // Si se alcanzó el quantum y el hilo sigue en ejecución, se cambia su estado
        if (tiempo_ejecutado >= quantum) {
            tcb->estado = TCB_READY;
            queue_push(cola, tcb);
        }
    }
}

/*
Colas Multinivel
Se elegirá al siguiente hilo a ejecutar según el siguiente esquema de colas multinivel:
- Se tendrá una cola por cada nivel de prioridad existente entre los hilos del sistema.
- El algoritmo entre colas es de prioridades sin desalojo.
- Cada cola implementará un algoritmo Round Robin con un quantum (Q) definido por archivo de configuración.
- Al llegar un hilo a ready se posiciona siempre al final de la cola que le corresponda.
*/
void colas_multinivel(t_pcb *pcb)
{
    
    if (!list_is_empty(pcb->colas_hilos_prioridad_ready))
    {
    int prioridad_mayor = obtener_menor_prioridad(pcb->colas_hilos_prioridad_ready);
    t_cola_prioridad* cola_prioritaria = obtener_cola_por_prioridad(pcb->colas_hilos_prioridad_ready, prioridad_mayor);
        if (cola_prioritaria != NULL && !queue_is_empty(cola_prioritaria->cola))
        {
            round_robin(cola_prioritaria->cola);
        }
        else
        {
        colas_multinivel(pcb);
        }
    }
}


void hilo_ordena_lista_prioridades(t_pcb *pcb)
{
    pthread_t hilo_prioridades;

    int resultado = pthread_create(&hilo_prioridades, NULL, ordenamiento_continuo, pcb->lista_prioridad_ready);
    
    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo_prioridades");
        return;
    }
    pthread_detach(hilo_prioridades);
}

void* ordenamiento_continuo (void* void_args){

t_list* lista_prioridades = (t_list*)void_args;

while(estado_kernel!=0){

    sem_wait(&sem_lista_prioridades);

    ordenar_por_prioridad(lista_prioridades);
}
return NULL;
}

void *funcion_ready_exec_hilos(void *arg)
{
    char*algoritmo=(char*)arg;
    while (estado_kernel != 0)
    { 
        if (strings_iguales(algoritmo, "FIFO")){
        proceso_exec->hilo_exec = fifo_tcb(proceso_exec);
        ejecucion(proceso_exec->hilo_exec,proceso_exec->cola_hilos_ready,sockets->sockets_cliente_cpu->socket_Dispatch);
        
        } else if (strings_iguales(algoritmo, "PRIORIDADES"))
        {
        proceso_exec->hilo_exec = prioridades(proceso_exec);
        ejecucion(proceso_exec->hilo_exec, proceso_exec->cola_hilos_ready, sockets->sockets_cliente_cpu->socket_Dispatch);
        }

        if (strings_iguales(algoritmo, "MULTINIVEL"))
        {
            colas_multinivel(proceso_exec);
           
        }
    }
    return NULL;
}

void planificador_corto_plazo(t_pcb *pcb) // Si llega un pcb nuevo a la cola ready y estoy en algoritmo de prioridades, el parámetro es necesario
{

    char *algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if (strings_iguales(algoritmo, "PRIORIDADES")){
        hilo_ordena_lista_prioridades(pcb);
    }
    pthread_t hilo_ready_exec;

    int resultado = pthread_create(&hilo_ready_exec, NULL, funcion_ready_exec_hilos, algoritmo);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo_ready_exec");
        return;
    }

    pthread_detach(hilo_ready_exec);
}
