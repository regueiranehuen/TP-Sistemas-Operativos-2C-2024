#include "includes/planificacion.h"



t_tcb *fifo_tcb()
{

    sem_wait(&semaforo_cola_ready);
    log_info(logger, "Se tomó el semáforo (cola_ready)");

    
    pthread_mutex_lock(&mutex_cola_ready);

    t_tcb *tcb = queue_pop(cola_ready_fifo);
    if (tcb == NULL){
        pthread_mutex_unlock(&mutex_cola_ready);
        return NULL;
    }
    pthread_mutex_unlock(&mutex_cola_ready);
    return tcb;

    
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
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo_planificador_largo_plazo");
        pthread_mutex_unlock(&mutex_log);
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
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo new_ready");
        pthread_mutex_unlock(&mutex_log);
        return NULL;
    }

    resultado = pthread_create(&hilo_exit_procesos, NULL, funcion_procesos_exit, NULL);

    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo procesos_exit");
        pthread_mutex_unlock(&mutex_log);
        return NULL;
    }

    resultado = pthread_create(&hilo_hilos_exit, NULL, funcion_hilos_exit, NULL);
  
    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo hilos_exit");
        pthread_mutex_unlock(&mutex_log);
        return NULL;
    }

    pthread_detach(hilo_hilos_exit);
    pthread_detach(hilo_new_ready_procesos);
    pthread_detach(hilo_exit_procesos);

    return NULL;
}

void *atender_syscall(void *args) // recibir un paquete con un codigo de operacion, entrar al switch con dicho codigo de operacion y luego serializar el paquete
{

    while (estado_kernel != 0)
    {

        printf("esperando syscall\n");
        t_paquete_syscall *paquete = recibir_paquete_syscall(sockets->sockets_cliente_cpu->socket_Dispatch);
        printf("recibi syscall\n");
        if (paquete == NULL)
        {
            pthread_mutex_lock(&mutex_log);
            log_info(logger, "Paquete == NULL --> Conexion cerrada");
            pthread_mutex_unlock(&mutex_log);
            break;
        }

        pthread_mutex_lock(&mutex_syscall_ejecutando);
        syscallEjecutando = true;
        pthread_mutex_unlock(&mutex_syscall_ejecutando);

        switch (paquete->syscall)
        {

        case ENUM_PROCESS_CREATE:

            log_info(logger, "## (%d:%d) - Solicitó syscall: PROCESS_CREATE", hilo_exec->pid, hilo_exec->tid);
            t_process_create *paramProcessCreate = parametros_process_create(paquete);
            log_info(logger, "pseudocodigo:%s, Tamanio:%d, Prioridad:%d", paramProcessCreate->nombreArchivo, paramProcessCreate->tamProceso, paramProcessCreate->prioridad);
            PROCESS_CREATE(paramProcessCreate->nombreArchivo, paramProcessCreate->tamProceso, paramProcessCreate->prioridad);
            
            break;
        case ENUM_PROCESS_EXIT:
            log_info(logger, "## (%d:%d) - Solicitó syscall: PROCESS_EXIT", hilo_exec->pid, hilo_exec->tid);
            
            PROCESS_EXIT();
            
            free(paquete->buffer);
            free(paquete);

            break;
        case ENUM_THREAD_CREATE:
            log_info(logger, "## (%d:%d) - Solicitó syscall: THREAD_CREATE", hilo_exec->pid, hilo_exec->tid);
            t_thread_create *paramThreadCreate = parametros_thread_create(paquete);
            log_info(logger, "NOMBRE PSEUDOCODIGO RECIBIDO THREAD CREATE: %s", paramThreadCreate->nombreArchivo);
            log_info(logger, "VOY A ENTRAR A THREAD CREATE");
            
            THREAD_CREATE(paramThreadCreate->nombreArchivo, paramThreadCreate->prioridad);
            
            sem_post(&sem_fin_syscall);
            free(paramThreadCreate);
            break;
        case ENUM_THREAD_JOIN:
            log_info(logger, "## (%d:%d) - Solicitó syscall: THREAD_JOIN", hilo_exec->pid, hilo_exec->tid);
            int tid_thread_join = recibir_entero_paquete_syscall(paquete);
            
            THREAD_JOIN(tid_thread_join);
            
            break;
        case ENUM_THREAD_CANCEL:
            log_info(logger, "## (%d:%d) - Solicitó syscall: THREAD_CANCEL", hilo_exec->pid, hilo_exec->tid);
            int tid_thread_cancel = recibir_entero_paquete_syscall(paquete);
            
            THREAD_CANCEL(tid_thread_cancel);
            break;
        case ENUM_THREAD_EXIT:
            log_info(logger, "## (%d:%d) - Solicitó syscall: THREAD_EXIT", hilo_exec->pid, hilo_exec->tid);
            
            THREAD_EXIT();
            
            free(paquete->buffer);
            free(paquete);
            break;
        case ENUM_MUTEX_CREATE:
            log_info(logger, "## (%d:%d) - Solicitó syscall: MUTEX_CREATE", hilo_exec->pid, hilo_exec->tid);
            char *recurso = recibir_string_paquete_syscall(paquete);
            log_info(logger, "RECURSO MUTEX CREATE: %s", recurso);
            
            MUTEX_CREATE(recurso);
           
            break;
        case ENUM_MUTEX_LOCK:
            log_info(logger, "## (%d:%d) - Solicitó syscall: MUTEX_LOCK", hilo_exec->pid, hilo_exec->tid);
            char *recurso_a_bloquear = recibir_string_paquete_syscall(paquete);
            log_info(logger, "RECURSO MUTEX LOCK: %s", recurso);
            
            MUTEX_LOCK(recurso_a_bloquear);
            
            break;
        case ENUM_MUTEX_UNLOCK:
            log_info(logger, "## (%d:%d) - Solicitó syscall: MUTEX_UNLOCK", hilo_exec->pid, hilo_exec->tid);
            char *recurso_a_desbloquear = recibir_string_paquete_syscall(paquete);
            log_info(logger, "RECURSO MUTEX UNLOCK: %s", recurso);
        
            MUTEX_UNLOCK(recurso_a_desbloquear);
            
            break;
        case ENUM_IO:
            log_info(logger, "## (%d:%d) - Solicitó syscall: IO", hilo_exec->pid, hilo_exec->tid);
            int milisegundos = recibir_entero_paquete_syscall(paquete);
            log_info(logger, "ENTRAMOS A SYSCALL IO, MILISEGUNDOS: %d", milisegundos);
            
            IO(milisegundos);
            
            break;
        case ENUM_DUMP_MEMORY:
            log_info(logger, "## (%d:%d) - Solicitó syscall: DUMP_MEMORY", hilo_exec->pid, hilo_exec->tid);
            
            DUMP_MEMORY();
            
            free(paquete->buffer);
            free(paquete);
            break;
        case ENUM_SEGMENTATION_FAULT:
            t_pcb *pcb = buscar_pcb_por_pid(lista_pcbs, hilo_exec->pid);
            pcb->estado = PCB_EXIT;
            pthread_mutex_lock(&mutex_cola_exit_procesos);
            queue_push(cola_exit_procesos, pcb);
            pthread_mutex_unlock(&mutex_cola_exit_procesos);
        
            send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Interrupt);
            
            free(paquete->buffer);
            free(paquete);
            break;/*
        case ENUM_FIN_QUANTUM_RR:
            log_info(logger, "SE RECIBIÓ EL CÓDIGO ENUM_FIN_QUANTUM_RR");
            sem_post(&sem_desalojado);

            free(paquete->buffer);
            free(paquete);
            break;
        case ENUM_DESALOJAR:
            log_info(logger, "SE RECIBIÓ EL CÓDIGO ENUM_DESALOJAR");
            sem_post(&sem_desalojado);
            free(paquete->buffer);
            free(paquete);
            break;*/
        case ENUM_CICLO_NUEVO:
            log_info(logger,"Ciclo nuevo por parte de cpu");
            sem_post(&sem_ciclo_nuevo);
            break;
        default:
            log_info(logger, "se recibio el codigo %d no valido", paquete->syscall);
            log_info(logger, "Syscall no válida.\n");
            // send_code_operacion(TERMINAR,sockets->sockets_cliente_cpu->socket_Interrupt);
            free(paquete->buffer);
            free(paquete);
            break;
        }
        pthread_mutex_lock(&mutex_syscall_ejecutando);
        syscallEjecutando = false;
        if(esperando){
        pthread_mutex_unlock(&mutex_syscall_ejecutando);
        sem_post(&sem_fin_syscall);
        }
        else{
        pthread_mutex_unlock(&mutex_syscall_ejecutando);
        }
    }
    return NULL;
}

void*atender_interrupt(void*args){

    while (estado_kernel!=0){

        code_operacion code = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Interrupt);

        if(code ==-1){
            log_info(logger,"Conexion cerrada con cpu");
            break;
        }

        switch(code){
            case ENUM_DESALOJAR:
            log_debug(logger,"LLEGÓ CÓDIGO ENUM_DESALOJAR");
            sem_post(&sem_desalojado);    
            break;
            case ENUM_FIN_QUANTUM_RR:
            log_debug(logger,"LLEGÓ CÓDIGO ENUM_FIN_QUANTUM_RR");

            sem_post(&sem_desalojado);
            break;
            default:
            log_error(logger,"CODIGO NO ESPERADO EN ATENDER INTERRUPT");
            break;
            
        }

    }
    return NULL;
}


t_tcb* prioridades (){

sem_wait(&semaforo_cola_ready);
log_info(logger, "Se tomó el semáforo (cola_ready)");


pthread_mutex_lock(&mutex_cola_ready);
t_tcb* tcb_prioritario = list_remove(lista_ready_prioridad,0);
pthread_mutex_unlock(&mutex_cola_ready);

return tcb_prioritario;

}

t_thread_create* parametros_thread_create(t_paquete_syscall*paquete){
    log_info(logger,"VOY A RECIBIR LOS PARAMETROS THREAD CREATE QUE EMOPCION");
    t_thread_create*info = malloc(sizeof(t_thread_create));
    
    void * stream = paquete->buffer->stream;

    int sizeNombreArchivo;
    // Deserializamos los campos que tenemos en el buffer
    memcpy(&sizeNombreArchivo, stream, sizeof(int)); // Recibimos el size del nombre del archivo de pseudocodigo
    stream += sizeof(int);
    log_info(logger,"SIze nombre archivo: %d",sizeNombreArchivo);
    info->nombreArchivo = malloc(sizeNombreArchivo);
    memcpy(info->nombreArchivo, stream, sizeNombreArchivo); // Primer parámetro para la syscall: nombre del archivo
    stream += sizeNombreArchivo;
    log_info(logger,"nombre archivo: %s",info->nombreArchivo);
    memcpy(&(info->prioridad), stream, sizeof(int));
    stream += sizeof(int);
    log_info(logger,"Prioridad: %d",info->prioridad);
    eliminar_paquete_syscall(paquete);

    return info;

}


void round_robin(t_queue *cola_ready_prioridad)
{
    pthread_mutex_lock(&mutex_cola_ready);
    if (!queue_is_empty(cola_ready_prioridad))
    {
        t_tcb *tcb = queue_pop(cola_ready_prioridad); // Sacar el primer hilo de la cola
        pthread_mutex_unlock(&mutex_cola_ready);
        tcb->estado = TCB_EXECUTE;
        hilo_exec = tcb;
        ejecucion();
    }
    else{
        pthread_mutex_unlock(&mutex_cola_ready);
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
    
    sem_wait(&semaforo_cola_ready);
    
    print_lista_prioridades(colas_ready_prioridad);
    printf("Lista bloqueados:\n");
    print_lista(lista_bloqueados);

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
        log_info(logger,"Esperando a planificar");
        sem_wait(&sem_ciclo_nuevo);
        sem_wait(&sem_desalojado);
        log_info(logger,"Planificando");
        aviso_cpu->desalojar = false;
        aviso_cpu->finQuantum = false;
        t_tcb* hilo_a_ejecutar;
        if (strings_iguales(algoritmo, "FIFO")){
        hilo_a_ejecutar = fifo_tcb();
        hilo_a_ejecutar->estado = TCB_EXECUTE;
        hilo_exec = hilo_a_ejecutar;
        ejecucion();
        
        } else if (strings_iguales(algoritmo, "PRIORIDADES"))
        {
        hilo_a_ejecutar = prioridades();
        hilo_a_ejecutar->estado = TCB_EXECUTE;
        hilo_exec = hilo_a_ejecutar;
        ejecucion();
        
        }

        else if (strings_iguales(algoritmo, "CMN"))
        {
            sem_init(&sem_fin_syscall,0,0);
            colas_multinivel();
        }
    }
    return NULL;
}


void planificador_corto_plazo() // Si llega un pcb nuevo a la cola ready y estoy en algoritmo de prioridades, el parámetro es necesario
{
    aviso_cpu = malloc(sizeof(t_aviso_cpu));
    aviso_cpu->desalojar = false;
    aviso_cpu->finQuantum = false;

    char *algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if (strings_iguales(algoritmo, "PRIORIDADES")){
        hilo_ordena_lista_prioridades();
    }
    pthread_t hilo_ready_exec;
    pthread_t hilo_atender_syscall;
    pthread_t hilo_atender_interrupt;

    int resultado = pthread_create(&hilo_ready_exec, NULL, hilo_planificador_corto_plazo, algoritmo);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo del planificador_corto_plazo");
        return;
    }

    resultado = pthread_create(&hilo_atender_syscall, NULL, atender_syscall, NULL);

    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo para atender syscalls");
        return;
    }

    resultado = pthread_create(&hilo_atender_interrupt, NULL, atender_interrupt, algoritmo);
    if (resultado != 0)
    {
        log_error(logger, "Error al crear el hilo para atender interrupt");
        return;
    }


    pthread_detach(hilo_atender_syscall);
    pthread_detach(hilo_ready_exec);
    pthread_detach(hilo_atender_interrupt);
}

/*Ejecución
Una vez seleccionado el siguiente hilo a ejecutar, se lo transicionará al estado EXEC y se enviará al módulo CPU el TID y su PID 
asociado a ejecutar a través del puerto de dispatch, quedando a la espera de recibir dicho TID después de la ejecución junto con un motivo por el cual fue devuelto.
En caso que el algoritmo requiera desalojar al hilo en ejecución, se enviará una interrupción a través de la conexión de interrupt para forzar el desalojo del mismo.
Al recibir el TID del hilo en ejecución, en caso de que el motivo de devolución implique replanificar, se seleccionará el siguiente hilo a ejecutar según indique 
el algoritmo. Durante este período la CPU se quedará esperando.
*/

void espera_con_quantum(int quantum)
{
    
    fd_set read_fds;
    struct timeval timeout;

    timeout.tv_sec = quantum / 1000;           // Convertir a segundos
    timeout.tv_usec = (quantum % 1000) * 1000; // Convertir a microsegundos

    FD_ZERO(&read_fds);
    FD_SET(sockets->sockets_cliente_cpu->socket_Interrupt, &read_fds); 

    int max_fd = sockets->sockets_cliente_cpu->socket_Interrupt + 1; // Determinar descriptor máximo

    log_debug(logger, "Esperando en select con timeout");

    if (sockets->sockets_cliente_cpu->socket_Interrupt <= 0)
    {
        log_error(logger, "Descriptores de archivo no válidos");
        sem_destroy(&sem_fin_syscall);
        return;
    }

    int resultado = select(max_fd, &read_fds, NULL, NULL, &timeout);

    if (resultado == -1)
    {
        log_error(logger, "Error en select");
    }
    else if (resultado == 0) // Si se termina el quantum antes de que termine de ejecutar una syscall
    {
    
        log_debug(logger, "Timeout alcanzado en select (terminó el quantum)");
        // Tiempo del quantum agotado, desalojar por quantum
            pthread_mutex_lock(&mutex_desalojo);
            pthread_mutex_lock(&mutex_syscall_ejecutando);
            if (syscallEjecutando)
        {
            esperando = true;
            pthread_mutex_unlock(&mutex_syscall_ejecutando);
            log_info(logger, "Syscall en curso. Esperando a que finalice antes de mandar fin de quantum.");
            // Esperamos a que se termine la syscall
            sem_wait(&sem_fin_syscall);
            esperando = false;
            log_info(logger, "Syscall finalizada. Continuando con el desalojo por fin de quantum.");
        }
        else{
            pthread_mutex_unlock(&mutex_syscall_ejecutando);
        }
            if(!aviso_cpu->desalojar){
            aviso_cpu->finQuantum = true;
            pthread_mutex_unlock(&mutex_desalojo);
            code_operacion cod_op = FIN_QUANTUM_RR;

            log_info(logger, "## (%d:%d) - Desalojado por fin de Quantum", hilo_exec->pid, hilo_exec->tid);
            log_info(logger, "Voy a mandar el código correspondiente a fin quantum (%d)", FIN_QUANTUM_RR);

            pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
            send_code_operacion(cod_op, sockets->sockets_cliente_cpu->socket_Interrupt);
            pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);

            log_info(logger, "Enviado fin quantum!");
        if(!tcb_metido_en_estructura(hilo_exec)){
            t_tcb* tcb = hilo_exec;
            tcb->estado = TCB_READY;
            pushear_cola_ready(tcb);
        }
        }
        else if(aviso_cpu->desalojar){
            aviso_cpu->desalojar = false;
            pthread_mutex_unlock(&mutex_desalojo);
        }
}
else if(resultado > 0){
    log_info(logger,"Llegada de interrupcion, finaliza clock");
}
}
void pushear_cola_ready(t_tcb* hilo){

    log_info(logger, "FUNCION PUSHEAR COLA READY LLEGA HILO CON TID %d",hilo->tid);

    char* planificacion = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    
    if (strcmp(planificacion, "FIFO") == 0){
        pthread_mutex_lock(&mutex_cola_ready);
        queue_push(cola_ready_fifo, hilo);
        pthread_mutex_unlock(&mutex_cola_ready);

    } else if(strcmp(planificacion,"PRIORIDADES")==0)
    {
        pthread_mutex_lock(&mutex_cola_ready);
        list_add(lista_ready_prioridad,hilo);
        pthread_mutex_unlock(&mutex_cola_ready);
        sem_post(&sem_lista_prioridades);
    }
    if (strcmp(planificacion, "CMN") == 0)
    {
        pthread_mutex_lock(&mutex_cola_ready);
        t_cola_prioridad *cola = cola_prioridad(colas_ready_prioridad, hilo->prioridad);
        queue_push(cola->cola, hilo);
        pthread_mutex_unlock(&mutex_cola_ready);
    }
    log_info(logger, "Signal enviado al semáforo (cola_ready)");
    sem_post(&semaforo_cola_ready);
    int valor;
    sem_getvalue(&semaforo_cola_ready,&valor);
    log_warning(logger,"supervalor: %d",valor);


}


// Una vez seleccionado el siguiente hilo a ejecutar, se lo transicionará al estado EXEC y se enviará al módulo CPU el TID y su PID asociado a ejecutar a través del puerto de dispatch, quedando a la espera de recibir dicho TID después de la ejecución junto con un motivo por el cual fue devuelto.
// En caso que el algoritmo requiera desalojar al hilo en ejecución, se enviará una interrupción a través de la conexión de interrupt para forzar el desalojo del mismo.
// Al recibir el TID del hilo en ejecución, en caso de que el motivo de devolución implique replanificar, se seleccionará el siguiente hilo a ejecutar según indique el algoritmo. Durante este período la CPU se quedará esperando.

void ejecucion()
{

code_operacion cod_op = THREAD_EXECUTE_AVISO;

log_info(logger,"ENVIANDO TID %d Y PID %d",hilo_exec->tid,hilo_exec->pid);

pthread_mutex_lock(&mutex_conexion_kernel_a_dispatch);
send_operacion_tid_pid(cod_op, hilo_exec->tid, hilo_exec->pid, sockets->sockets_cliente_cpu->socket_Dispatch);
pthread_mutex_unlock(&mutex_conexion_kernel_a_dispatch);

    char* algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");

if(strcmp(algoritmo,"CMN")==0){
    char* quantum_char = config_get_string_value(config,"QUANTUM");
    int quantum = atoi(quantum_char);
    espera_con_quantum(quantum);
}

}

