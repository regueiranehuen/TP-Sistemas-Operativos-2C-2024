#include "includes/planificacion.h"

t_tcb *fifo_tcb()
{

    if (queue_size(cola_ready_fifo) > 0)
    {
        pthread_mutex_lock(&mutex_cola_ready);
        t_tcb *tcb = queue_pop(cola_ready_fifo);
        pthread_mutex_unlock(&mutex_cola_ready);
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
        
        hilo_exit();
    }
    return NULL;
}

void planificador_largo_plazo()
{
    pthread_t hilo_plani_largo_plazo;
    int resultado;

    resultado = pthread_create(&hilo_plani_largo_plazo, NULL, hilo_planificador_largo_plazo, NULL);
    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo_planificador_largo_plazo");
        return;
    }
    pthread_detach(hilo_plani_largo_plazo);
}

void *hilo_planificador_largo_plazo(void *void_args)
{
    pthread_t hilo_new_ready_procesos;
    pthread_t hilo_exit_procesos;
    pthread_t hilo_hilos_exit;

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

    resultado = pthread_create(&hilo_hilos_exit, NULL, funcion_hilos_exit, NULL);
  
    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo hilos_exit");
        return NULL;
    }

    pthread_detach(hilo_hilos_exit);
    pthread_detach(hilo_new_ready_procesos);
    pthread_detach(hilo_exit_procesos);

    return NULL;
}


void atender_syscall()
{
    syscalls syscall; 
        pthread_mutex_unlock(&mutex_conexion_cpu);
        recv(sockets->sockets_cliente_cpu->socket_Dispatch, &syscall, sizeof(syscall), 0);
        pthread_mutex_unlock(&mutex_conexion_cpu);
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
        case ENUM_SEGMENTATION_FAULT:
            break;
        default:
            printf("Syscall no válida.\n");
            break;
        }
    }


t_tcb* prioridades (){

if(!list_is_empty(lista_ready_prioridad)){
pthread_mutex_lock(&mutex_cola_ready);
t_tcb* tcb_prioritario = list_remove(lista_ready_prioridad,0);
pthread_mutex_unlock(&mutex_cola_ready);
return tcb_prioritario;
}

return NULL;

}

void round_robin(t_queue *cola_ready_prioridad)
{
    if (!queue_is_empty(cola_ready_prioridad))
    {
        pthread_mutex_lock(&mutex_cola_ready);
        t_tcb *tcb = queue_pop(cola_ready_prioridad); // Sacar el primer hilo de la cola
        pthread_mutex_unlock(&mutex_cola_ready);
        tcb->estado = TCB_EXECUTE;
        hilo_exec = tcb;
        ejecucion(tcb);
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
void colas_multinivel(){
    pthread_mutex_lock(&mutex_cola_ready);
    t_cola_prioridad* cola_prioritaria = obtener_cola_con_mayor_prioridad(colas_ready_prioridad);
    pthread_mutex_unlock(&mutex_cola_ready);
    round_robin(cola_prioritaria->cola);
        
}

void hilo_ordena_lista_prioridades()
{
    pthread_t hilo_prioridades;

    int resultado = pthread_create(&hilo_prioridades, NULL, ordenamiento_continuo, lista_ready_prioridad);
    
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
    pthread_mutex_lock(&mutex_cola_ready);
    ordenar_por_prioridad(lista_prioridades);
    pthread_mutex_unlock(&mutex_cola_ready);
}
return NULL;
}

void *hilo_planificador_corto_plazo(void *arg)
{
    char*algoritmo=(char*)arg;

    while(estado_kernel!=0){

    sem_wait(&sem_desalojado);// tiene que empezar con valor 1 

        t_tcb* hilo_a_ejecutar;
        if (strings_iguales(algoritmo, "FIFO")){
        hilo_a_ejecutar = fifo_tcb();
        hilo_a_ejecutar->estado = TCB_EXECUTE;
        hilo_exec = hilo_a_ejecutar;
        ejecucion(hilo_a_ejecutar);
        
        } else if (strings_iguales(algoritmo, "PRIORIDADES"))
        {
        hilo_a_ejecutar = prioridades();
        hilo_a_ejecutar->estado = TCB_EXECUTE;
        hilo_exec = hilo_a_ejecutar;
        ejecucion(hilo_a_ejecutar);
        }

        if (strings_iguales(algoritmo, "MULTINIVEL"))
        {
            colas_multinivel();
        }
    }
    return NULL;
}

void planificador_corto_plazo() // Si llega un pcb nuevo a la cola ready y estoy en algoritmo de prioridades, el parámetro es necesario
{

    char *algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if (strings_iguales(algoritmo, "PRIORIDADES")){
        hilo_ordena_lista_prioridades();
    }
    pthread_t hilo_ready_exec;

    int resultado = pthread_create(&hilo_ready_exec, NULL, hilo_planificador_corto_plazo, algoritmo);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo del planificador_corto_plazo");
        return;
    }

    pthread_detach(hilo_ready_exec);
}
