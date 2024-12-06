#include "includes/planificacion.h"


t_tcb *fifo_tcb()
{

    sem_wait(&semaforo_cola_ready);
    pthread_mutex_lock(&mutex_estado_kernel);
    if (estado_kernel == 0){
    pthread_mutex_unlock(&mutex_estado_kernel);
        return NULL;
    }
    pthread_mutex_unlock(&mutex_estado_kernel);
    
    pthread_mutex_lock(&mutex_log);
    log_info(logger,"Cola ready");
    pthread_mutex_unlock(&mutex_log);

    pthread_mutex_lock(&mutex_cola_ready);
    print_queue(cola_ready_fifo);
    pthread_mutex_unlock(&mutex_cola_ready);
    
    pthread_mutex_lock(&mutex_log);
    log_info(logger,"Lista bloqueados:");
    pthread_mutex_unlock(&mutex_log);

    pthread_mutex_lock(&mutex_lista_blocked);
    print_lista(lista_bloqueados);
    pthread_mutex_unlock(&mutex_lista_blocked);

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

    while (1)
    {
        new_a_ready_procesos();
        pthread_mutex_lock(&mutex_estado_kernel);
        if(estado_kernel == 0){
        pthread_mutex_unlock(&mutex_estado_kernel);
            break;
        }
        pthread_mutex_unlock(&mutex_estado_kernel);

    }

    

    sem_post(&sem_termina_hilo);
    return NULL;
}

void *funcion_procesos_exit(void *void_args)
{
    while (1)
    {
        proceso_exit();
        pthread_mutex_lock(&mutex_estado_kernel);
        if(estado_kernel == 0){
            pthread_mutex_unlock(&mutex_estado_kernel);
            break;
        }
        pthread_mutex_unlock(&mutex_estado_kernel);
    }
    sem_post(&sem_termina_hilo);
    return NULL;
}

void *funcion_hilos_exit(void *void_args)
{

    while (1)
    {
        hilo_exit();
        pthread_mutex_lock(&mutex_estado_kernel);
        if(estado_kernel == 0){
            pthread_mutex_unlock(&mutex_estado_kernel);
            break;
        }
        pthread_mutex_unlock(&mutex_estado_kernel);
    }
    sem_post(&sem_termina_hilo);
    return NULL;
}

void planificador_largo_plazo()
{

    int resultado;

    pthread_t hilo_plani_largo_plazo;
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
    pthread_t hilo_hilos_exit;
    pthread_t hilo_exit_procesos;


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

    
    pthread_detach(hilo_new_ready_procesos);
    pthread_detach(hilo_hilos_exit);
    pthread_detach(hilo_exit_procesos);
    

    return NULL;
}

void *atender_syscall(void *args) // recibir un paquete con un codigo de operacion, entrar al switch con dicho codigo de operacion y luego serializar el paquete
{

    while (1)
    {

        pthread_mutex_lock(&mutex_log);
        log_info(logger, "esperando syscall");
        pthread_mutex_unlock(&mutex_log);

        t_paquete_syscall *paquete = recibir_paquete_syscall(sockets->sockets_cliente_cpu->socket_Dispatch);

        pthread_mutex_lock(&mutex_log);
        log_info(logger, "recibi syscall");
        pthread_mutex_unlock(&mutex_log);

        if (paquete == NULL)
        {
            pthread_mutex_lock(&mutex_log);
            log_info(logger, "Paquete == NULL --> Conexion cerrada");
            pthread_mutex_unlock(&mutex_log);
            sem_post(&sem_termina_hilo);
            return NULL;
        }

        pthread_mutex_lock(&mutex_syscall_ejecutando);
        syscallEjecutando = true;
        pthread_mutex_unlock(&mutex_syscall_ejecutando);


        switch (paquete->syscall)
        {
        case ENUM_OK:

            pthread_mutex_lock(&mutex_log);
            log_info(logger,"RECIBI OK");
            pthread_mutex_unlock(&mutex_log);

            sem_post(&sem_recibi_ok);
            free(paquete->buffer);
            free(paquete);
            break;
        case ENUM_PROCESS_CREATE:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: PROCESS_CREATE", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            t_process_create *paramProcessCreate = parametros_process_create(paquete);
            
            PROCESS_CREATE(paramProcessCreate->nombreArchivo, paramProcessCreate->tamProceso, paramProcessCreate->prioridad);
            free(paramProcessCreate->nombreArchivo);
            free(paramProcessCreate);
            break;
        case ENUM_PROCESS_EXIT:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: PROCESS_EXIT", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);

            PROCESS_EXIT();
            
            free(paquete->buffer);
            free(paquete);

            break;
        case ENUM_THREAD_CREATE:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: THREAD_CREATE", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            t_thread_create *paramThreadCreate = parametros_thread_create(paquete);

            THREAD_CREATE(paramThreadCreate->nombreArchivo, paramThreadCreate->prioridad);
            
            free(paramThreadCreate->nombreArchivo);
            free(paramThreadCreate);
            break;
        case ENUM_THREAD_JOIN:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: THREAD_JOIN", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            int tid_thread_join = recibir_entero_paquete_syscall(paquete);
            
            THREAD_JOIN(tid_thread_join);
            
            break;
        case ENUM_THREAD_CANCEL:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: THREAD_CANCEL", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            int tid_thread_cancel = recibir_entero_paquete_syscall(paquete);
            
            THREAD_CANCEL(tid_thread_cancel);
            break;
        case ENUM_THREAD_EXIT:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: THREAD_EXIT", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            THREAD_EXIT();
            
            free(paquete->buffer);
            free(paquete);
            break;
        case ENUM_MUTEX_CREATE:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: MUTEX_CREATE", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            char *recurso = recibir_string_paquete_syscall(paquete);
            
            MUTEX_CREATE(recurso);
            free(recurso);
            break;
        case ENUM_MUTEX_LOCK:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: MUTEX_LOCK", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            char *recurso_a_bloquear = recibir_string_paquete_syscall(paquete);
            
            MUTEX_LOCK(recurso_a_bloquear);
            free(recurso_a_bloquear);
            break;
        case ENUM_MUTEX_UNLOCK:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: MUTEX_UNLOCK", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            char *recurso_a_desbloquear = recibir_string_paquete_syscall(paquete);
        
            MUTEX_UNLOCK(recurso_a_desbloquear);
            free(recurso_a_desbloquear);
            break;
        case ENUM_IO:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Solicitó syscall: IO", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            int milisegundos = recibir_entero_paquete_syscall(paquete);
            
            IO(milisegundos);
            
            break;
        case ENUM_DUMP_MEMORY:      
            pthread_mutex_lock(&mutex_log);      
            log_debug(logger, "## (%d:%d) - Solicitó syscall: DUMP_MEMORY", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            DUMP_MEMORY();

            free(paquete->buffer);
            free(paquete);
            break;
        case ENUM_SEGMENTATION_FAULT:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger,"(%d:%d) - Ocurrió Segmentation Fault",hilo_exec->pid,hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);

            atender_segmentation_fault();
            
            free(paquete->buffer);
            free(paquete);
            break;
        case ENUM_CICLO_NUEVO:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger,"Ciclo nuevo por parte de cpu");
            pthread_mutex_unlock(&mutex_log);
            sem_post(&sem_ciclo_nuevo);
            free(paquete->buffer);
            free(paquete);
            break;
        default:
            pthread_mutex_lock(&mutex_log);
            log_info(logger, "se recibio el codigo %d no valido", paquete->syscall);
            pthread_mutex_unlock(&mutex_log);
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

    while (1){

        syscalls code = recibir_sys(sockets->sockets_cliente_cpu->socket_Interrupt);

        if(code ==-1){
            pthread_mutex_lock(&mutex_log);
            log_info(logger,"Conexion cerrada con cpu");
            pthread_mutex_unlock(&mutex_log);
            return NULL;
        }

        int valor;
        sem_getvalue(&sem_recibi_ok,&valor);

        if (valor == 1){
            sem_wait(&sem_recibi_ok);
        }

        switch(code){
            case ENUM_DESALOJAR:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger,"LLEGÓ CÓDIGO ENUM_DESALOJAR");
            pthread_mutex_unlock(&mutex_log);
            sem_post(&sem_desalojado);   
            break;
            case ENUM_FIN_QUANTUM_RR:
            pthread_mutex_lock(&mutex_log);
            log_debug(logger,"LLEGÓ CÓDIGO ENUM_FIN_QUANTUM_RR");
            pthread_mutex_unlock(&mutex_log);
            sem_post(&sem_desalojado);
            break;
            default:
            pthread_mutex_lock(&mutex_log);
            log_error(logger,"CODIGO NO ESPERADO EN ATENDER INTERRUPT");
            pthread_mutex_unlock(&mutex_log);
            break;
            
        }

    }
    return NULL;
}

void* cortar_ejecucion_modulos(void*args){

    while (1){
        sem_wait(&sem_seguir_o_frenar);
        pthread_mutex_lock(&mutex_estado_kernel);
        if (estado_kernel == 0){
            pthread_mutex_unlock(&mutex_estado_kernel);
            sem_post(&sem_termina_hilo);
            return NULL;
        } 
        pthread_mutex_unlock(&mutex_estado_kernel);
        pthread_mutex_lock(&mutex_lista_pcbs);
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"LISTA PIDS EN CORTAR EJECUCION MODULOS");
        pthread_mutex_unlock(&mutex_log);
        for (int i = 0; i< list_size(lista_pcbs); i++){
            t_pcb*act=list_get(lista_pcbs,i);
            pthread_mutex_lock(&mutex_log);
            log_info(logger,"pid %d",act->pid);
            pthread_mutex_unlock(&mutex_log);
        }

        if (!list_is_empty(lista_pcbs)){
            pthread_mutex_unlock(&mutex_lista_pcbs);
        }
        else{
            pthread_mutex_unlock(&mutex_lista_pcbs);
            sem_post(&sem_fin_kernel);
        }
    }
    return NULL;
}


t_tcb* prioridades (){

    sem_wait(&semaforo_cola_ready);
    pthread_mutex_lock(&mutex_estado_kernel);
    if (estado_kernel == 0){
        pthread_mutex_unlock(&mutex_estado_kernel);
            return NULL;
    }
    pthread_mutex_unlock(&mutex_estado_kernel);

    pthread_mutex_lock(&mutex_cola_ready);
    t_tcb* tcb_prioritario = list_remove(lista_ready_prioridad,0);
    pthread_mutex_unlock(&mutex_cola_ready);

    return tcb_prioritario;

}

t_thread_create* parametros_thread_create(t_paquete_syscall*paquete){

    t_thread_create*info = malloc(sizeof(t_thread_create));
    
    void * stream = paquete->buffer->stream;

    int sizeNombreArchivo;
    // Deserializamos los campos que tenemos en el buffer
    memcpy(&sizeNombreArchivo, stream, sizeof(int)); // Recibimos el size del nombre del archivo de pseudocodigo
    stream += sizeof(int);
    
    info->nombreArchivo = malloc(sizeNombreArchivo);
    memcpy(info->nombreArchivo, stream, sizeNombreArchivo); // Primer parámetro para la syscall: nombre del archivo
    stream += sizeNombreArchivo;
    
    memcpy(&(info->prioridad), stream, sizeof(int));
    stream += sizeof(int);
    
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
    pthread_mutex_lock(&mutex_estado_kernel);
    if (estado_kernel == 0){
        pthread_mutex_unlock(&mutex_estado_kernel);
        return;
    }
    pthread_mutex_unlock(&mutex_estado_kernel);

    pthread_mutex_lock(&mutex_cola_ready);
    print_lista_prioridades(colas_ready_prioridad);
    pthread_mutex_unlock(&mutex_cola_ready);
    pthread_mutex_lock(&mutex_log);
    log_info(logger,"Lista bloqueados:\n");
    pthread_mutex_unlock(&mutex_log);
    pthread_mutex_lock(&mutex_lista_blocked);
    print_lista(lista_bloqueados);
    pthread_mutex_unlock(&mutex_lista_blocked);

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
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo_prioridades");
        pthread_mutex_unlock(&mutex_log);
        return;
    }
    pthread_detach(hilo_prioridades);
}

void* ordenamiento_continuo (void* void_args){

t_list* lista_prioridades = (t_list*)void_args;

while(1){

    sem_wait(&sem_lista_prioridades);
    pthread_mutex_lock(&mutex_estado_kernel);
    if (estado_kernel == 0){
        pthread_mutex_unlock(&mutex_estado_kernel);
        sem_post(&sem_termina_hilo);
        return NULL;
    }
    pthread_mutex_unlock(&mutex_estado_kernel);

    pthread_mutex_lock(&mutex_cola_ready);
    ordenar_por_prioridad(lista_prioridades);
    pthread_mutex_unlock(&mutex_cola_ready);
}
return NULL;
}

void *hilo_planificador_corto_plazo(void *arg)
{
    char*algoritmo=(char*)arg;

    while(1){
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"Esperando a planificar");
        pthread_mutex_unlock(&mutex_log);
        sem_wait(&sem_ciclo_nuevo);
        sem_wait(&sem_desalojado);

        pthread_mutex_lock(&mutex_estado_kernel);
        if (estado_kernel == 0){
            pthread_mutex_unlock(&mutex_estado_kernel);
            sem_post(&sem_termina_hilo);
            return NULL;
        }
        pthread_mutex_unlock(&mutex_estado_kernel);
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"Planificando");
        pthread_mutex_unlock(&mutex_log);
        aviso_cpu->desalojar = false;
        aviso_cpu->finQuantum = false;
        t_tcb* hilo_a_ejecutar;
        if (strings_iguales(algoritmo, "FIFO")){
        hilo_a_ejecutar = fifo_tcb();
        pthread_mutex_lock(&mutex_estado_kernel);
        if (estado_kernel == 0){
            pthread_mutex_unlock(&mutex_estado_kernel);
            goto final;
        }
        pthread_mutex_unlock(&mutex_estado_kernel);
        hilo_a_ejecutar->estado = TCB_EXECUTE;
        hilo_exec = hilo_a_ejecutar;
        ejecucion();
        
        } else if (strings_iguales(algoritmo, "PRIORIDADES"))
        {
        hilo_a_ejecutar = prioridades();
        pthread_mutex_lock(&mutex_estado_kernel);
        if (estado_kernel == 0){
            pthread_mutex_unlock(&mutex_estado_kernel);
            goto final;
        }
        pthread_mutex_unlock(&mutex_estado_kernel);
        hilo_a_ejecutar->estado = TCB_EXECUTE;
        hilo_exec = hilo_a_ejecutar;
        ejecucion();
        
        }

        else if (strings_iguales(algoritmo, "CMN"))
        {
            sem_init(&sem_fin_syscall,0,0);
            colas_multinivel();
        }
        final:
        pthread_mutex_lock(&mutex_estado_kernel);
        if (estado_kernel == 0){
            pthread_mutex_unlock(&mutex_estado_kernel);
            sem_post(&sem_termina_hilo);
            return NULL;
        }
        pthread_mutex_unlock(&mutex_estado_kernel);
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
    pthread_t hilo_cortar_modulos;
    int resultado = pthread_create(&hilo_ready_exec, NULL, hilo_planificador_corto_plazo, algoritmo);

    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo del planificador_corto_plazo");
        pthread_mutex_unlock(&mutex_log);
        return;
    }

    resultado = pthread_create(&hilo_atender_syscall, NULL, atender_syscall, NULL);

    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo para atender syscalls");
        pthread_mutex_unlock(&mutex_log);
        return;
    }

    resultado = pthread_create(&hilo_atender_interrupt, NULL, atender_interrupt, algoritmo);
    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo para atender interrupt");
        pthread_mutex_unlock(&mutex_log);
        return;
    }

    resultado = pthread_create(&hilo_cortar_modulos,NULL,cortar_ejecucion_modulos,NULL);
    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo para cortar la ejecución de los módulos");
        pthread_mutex_unlock(&mutex_log);
        return;
    }
    pthread_detach(hilo_atender_syscall);
    pthread_detach(hilo_ready_exec);
    pthread_detach(hilo_atender_interrupt);
    pthread_detach(hilo_cortar_modulos);
    
    
    
    
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

    pthread_mutex_lock(&mutex_log);
    log_debug(logger, "Esperando en select con timeout");
    pthread_mutex_unlock(&mutex_log);

    if (sockets->sockets_cliente_cpu->socket_Interrupt <= 0)
    {
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Descriptores de archivo no válidos");
        pthread_mutex_unlock(&mutex_log);
        sem_destroy(&sem_fin_syscall);
        return;
    }

    int resultado = select(max_fd, &read_fds, NULL, NULL, &timeout);

    if (resultado == -1)
    {
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error en select");
        pthread_mutex_unlock(&mutex_log);
    }
    else if (resultado == 0) // Si se termina el quantum
    {
        pthread_mutex_lock(&mutex_log);
        log_debug(logger, "Timeout alcanzado en select (terminó el quantum)");
        pthread_mutex_unlock(&mutex_log);
        // Tiempo del quantum agotado, desalojar por quantum
        
        pthread_mutex_lock(&mutex_syscall_ejecutando);
        if (syscallEjecutando)
        {
            esperando = true;
            pthread_mutex_unlock(&mutex_syscall_ejecutando);
            pthread_mutex_lock(&mutex_log);
            log_info(logger, "Syscall en curso. Esperando a que finalice antes de mandar fin de quantum.");
            pthread_mutex_unlock(&mutex_log);
            // Esperamos a que se termine la syscall
            sem_wait(&sem_fin_syscall);
            esperando = false;
            pthread_mutex_lock(&mutex_log);
            log_info(logger, "Syscall finalizada. Continuando con el desalojo por fin de quantum.");
            pthread_mutex_unlock(&mutex_log);
        }
        else
        {
            pthread_mutex_unlock(&mutex_syscall_ejecutando);
        }
        pthread_mutex_lock(&mutex_desalojo);
        if (!aviso_cpu->desalojar)
        {
            aviso_cpu->finQuantum = true;
            pthread_mutex_unlock(&mutex_desalojo);
            code_operacion cod_op = FIN_QUANTUM_RR;
            pthread_mutex_lock(&mutex_log);
            log_debug(logger, "## (%d:%d) - Desalojado por fin de Quantum", hilo_exec->pid, hilo_exec->tid);
            pthread_mutex_unlock(&mutex_log);
            pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
            send_code_operacion(cod_op, sockets->sockets_cliente_cpu->socket_Interrupt);
            pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);

            if (!tcb_metido_en_estructura(hilo_exec))
            {
                t_tcb *tcb = hilo_exec;
                tcb->estado = TCB_READY;
                pushear_cola_ready(tcb);
            }
        }
        else if (aviso_cpu->desalojar)
        {
            aviso_cpu->desalojar = false;
            pthread_mutex_unlock(&mutex_desalojo);
        }
    }
else if(resultado > 0){
    pthread_mutex_lock(&mutex_log);
    log_info(logger,"Llegada de interrupcion, finaliza clock");
    pthread_mutex_unlock(&mutex_log);
}
}
void pushear_cola_ready(t_tcb* hilo){
    
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
    sem_post(&semaforo_cola_ready);
}


// Una vez seleccionado el siguiente hilo a ejecutar, se lo transicionará al estado EXEC y se enviará al módulo CPU el TID y su PID asociado a ejecutar a través del puerto de dispatch, quedando a la espera de recibir dicho TID después de la ejecución junto con un motivo por el cual fue devuelto.
// En caso que el algoritmo requiera desalojar al hilo en ejecución, se enviará una interrupción a través de la conexión de interrupt para forzar el desalojo del mismo.
// Al recibir el TID del hilo en ejecución, en caso de que el motivo de devolución implique replanificar, se seleccionará el siguiente hilo a ejecutar según indique el algoritmo. Durante este período la CPU se quedará esperando.

void ejecucion()
{

    code_operacion cod_op = THREAD_EXECUTE_AVISO;
    pthread_mutex_lock(&mutex_log);
    log_info(logger, "ENVIANDO TID %d Y PID %d", hilo_exec->tid, hilo_exec->pid);
    pthread_mutex_unlock(&mutex_log);
    pthread_mutex_lock(&mutex_conexion_kernel_a_dispatch);
    send_operacion_tid_pid(cod_op, hilo_exec->tid, hilo_exec->pid, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_dispatch);

    char *algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if (strcmp(algoritmo, "CMN") == 0)
    {
        char *quantum_char = config_get_string_value(config, "QUANTUM");
        int quantum = atoi(quantum_char);
        espera_con_quantum(quantum);
    }
}
