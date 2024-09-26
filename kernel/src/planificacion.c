#include "includes/planificacion.h"

int estado_kernel;

t_pcb *fifo(t_queue *cola_proceso)
{

    if (cola_proceso != NULL)
    {
        t_pcb *pcb = queue_pop(cola_proceso);
        return pcb;
    }
    return NULL;
}


/*
Planificador de Largo Plazo
El Kernel será el encargado de gestionar las peticiones a la memoria para la creación y eliminación de procesos e hilos. 
Creación de procesos
Se tendrá una cola NEW que será administrada estrictamente por FIFO para la creación de procesos. 
Al llegar un nuevo proceso a esta cola y la misma esté vacía se enviará un pedido a Memoria para inicializar el mismo, 
si la respuesta es positiva se crea el TID 0 de ese proceso y se lo pasa al estado READY y se sigue la misma lógica con el proceso que sigue. 
Si la respuesta es negativa (ya que la Memoria no tiene espacio suficiente para inicializarlo) se deberá esperar la finalización de otro proceso 
para volver a intentar inicializarlo.
Al llegar un proceso a esta cola y haya otros esperando, el mismo simplemente se encola.
Finalización de procesos
Al momento de finalizar un proceso, el Kernel deberá informar a la Memoria la finalización del mismo y luego de recibir la confirmación por parte de 
la Memoria deberá liberar su PCB asociado e intentar inicializar uno de los que estén esperando en estado NEW si los hubiere.
Creación de hilos
Para la creación de hilos, el Kernel deberá informar a la Memoria y luego ingresarlo directamente a la cola de READY correspondiente, según su nivel de prioridad.
Finalización de hilos
Al momento de finalizar un hilo, el Kernel deberá informar a la Memoria la finalización del mismo, liberar su TCB asociado y 
deberá mover al estado READY a todos los hilos que se encontraban bloqueados por ese TID. De esta manera, se desbloquean aquellos hilos 
bloqueados por THREAD_JOIN o por mutex tomados por el hilo finalizado (en caso que hubiera).
*/

void* funcion_new_ready(void* void_args){

while(estado_kernel != 0){
    new_a_ready();
}
return NULL;
}

void* funcion_process_exit(void* void_args){
    while(estado_kernel != 0){
    proceso_exit();
    }
    return NULL;
}

void hilo_planificador_largo_plazo(){
    pthread_t hilo_plani_largo_plazo;
    int resultado;

    resultado = pthread_create(&hilo_plani_largo_plazo,NULL,planificador_largo_plazo,NULL);
     if(resultado != 0){
    log_error(logger,"Error al crear el hilo_planificador_largo_plazo");
    return;
}
    pthread_detach(hilo_plani_largo_plazo);
}

void* planificador_largo_plazo(void* void_args){
pthread_t hilo_new_ready;
pthread_t hilo_atender_syscalls;
pthread_t hilo_exit;

int resultado;

resultado = pthread_create (&hilo_new_ready,NULL,funcion_new_ready,NULL);

if(resultado != 0){
    log_error(logger,"Error al crear el hilo_new_ready");
    return NULL;
}

resultado = pthread_create(&hilo_atender_syscalls,NULL,atender_syscall,NULL);

if(resultado != 0){
    log_error(logger,"Error al crear el hilo_atender_syscall");
    return NULL;
}

resultado = pthread_create(&hilo_exit,NULL,funcion_process_exit,NULL);

if(resultado != 0){
    log_error(logger,"Error al crear el hilo_atender_syscall");
    return NULL;
}

pthread_detach(hilo_new_ready);
pthread_detach(hilo_atender_syscalls);
pthread_detach(hilo_exit);
    return NULL;
}

void* atender_syscall(void* void_args){//el problema es que todas las funciones que tengan recv con cpu tienen la posiblidad de recibir la syscall entonces hay que buscar la manera que solamente lo reciba el que corresponda

syscalls syscall;

while(estado_kernel != 0){

recv(sockets->sockets_cliente_cpu->socket_Dispatch,&syscall,sizeof(syscall),0);

switch(syscall){

  case ENUM_PROCESS_CREATE:
            //PROCESS_CREATE(); lo mismo que lo de abajo
            break;
        case ENUM_PROCESS_EXIT:
            PROCESS_EXIT();
            break;
        case ENUM_THREAD_CREATE:
            //THREAD_CREATE(); lo mismo que lo de abajo
            break;
        case ENUM_THREAD_JOIN:
            //THREAD_JOIN(tid);hay que ver como recibir el tid de cpu
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
            //IO(milisegundos); la misma situación
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
