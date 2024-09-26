#include "includes/planificacion.h"

int estado_kernel;
pthread_mutex_t mutex_conexion_cpu;

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

void* funcion_new_ready_procesos(void* void_args){

while(estado_kernel != 0){
    new_a_ready_procesos();
}
return NULL;
}

void* funcion_procesos_exit(void* void_args){
    while(estado_kernel != 0){
    proceso_exit();
    }
    return NULL;
}

void* funcion_hilos_exit(void* void_args){
    while(estado_kernel != 0){
        //hilo_exit(pcb) Me imagino que habra que hacer un hilo por proceso
    }
    return NULL;
}

void* funcion_new_ready_hilos(void* void_args){
    while(estado_kernel != 0){
        //new_a_ready_hilos(pcb) Lo mismo que arriba
    }
    return NULL;
}

void hilo_atender_syscalls(){
    pthread_t hilo_syscalls;
    int resultado;

    resultado = pthread_create(&hilo_syscalls,NULL,atender_syscall,NULL);
     if(resultado != 0){
    log_error(logger,"Error al crear el hilo_planificador_largo_plazo");
    return;
}
    pthread_detach(hilo_syscalls);
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
pthread_t hilo_new_ready_procesos;
pthread_t hilo_exit_procesos;
pthread_t hilo_new_ready_hilos;
pthread_t hilo_exit_hilos;

int resultado;

resultado = pthread_create (&hilo_new_ready_procesos,NULL,funcion_new_ready_procesos,NULL);

if(resultado != 0){
    log_error(logger,"Error al crear el hilo new_ready");
    return NULL;
}

resultado = pthread_create(&hilo_exit_procesos,NULL,funcion_procesos_exit,NULL);

if(resultado != 0){
    log_error(logger,"Error al crear el hilo procesos_exit");
    return NULL;
}

resultado = pthread_create(&hilo_new_ready_hilos,NULL,funcion_new_ready_hilos,NULL);

if(resultado != 0){
    log_error(logger,"Error al crear el hilo new_ready_hilos");
    return NULL;
}

resultado = pthread_create(&hilo_exit_hilos,NULL,funcion_hilos_exit,NULL);

if(resultado != 0){
    log_error(logger,"Error al crear el hilo hilos_exit");
    return NULL;
}

pthread_detach(hilo_new_ready_procesos);
pthread_detach(hilo_exit_procesos);
pthread_detach(hilo_new_ready_hilos);
pthread_detach(hilo_exit_hilos);
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