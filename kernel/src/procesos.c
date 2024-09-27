#include "includes/procesos.h"
#include "includes/funcionesAuxiliares.h"
#include "includes/planificacion.h"

sem_t semaforo_new_ready_procesos;
sem_t semaforo_cola_new_procesos;
sem_t semaforo_cola_exit_procesos;
sem_t semaforo_cola_new_hilos;
sem_t semaforo_cola_exit_hilos;
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


t_pcb *crear_pcb()
{
    static int pid = 0;
    t_pcb *pcb = malloc(sizeof(t_pcb));
    t_list *lista_tids = list_create();
    pcb->pid = pid;
    pcb->tids = lista_tids;
    pid += 1;
    inicializar_estados_hilos(pcb);
    list_add(lista_pcbs, pcb);
    t_tcb *tcb = crear_tcb(pcb);
    queue_push(pcb->cola_hilos_new, tcb);
    return pcb;
}

t_tcb *crear_tcb(t_pcb *pcb)
{

    t_tcb *tcb = malloc(sizeof(t_tcb));
    int tamanio_lista = list_size(pcb->tids);
    tcb->tid = tamanio_lista;
    int *tid = malloc(sizeof(int));
    *tid = tcb->tid;
    tcb->estado = TCB_NEW;
    tcb->pid = pcb->pid;
    list_add(pcb->tids, tid);
    if (tcb->tid == 0)
    {
        tcb->prioridad = 0;
    }
    return tcb;
}

/*
PROCESS_CREATE, esta syscall recibirá 3 parámetros de la CPU, el primero será el nombre del archivo
de pseudocódigo que deberá ejecutar el proceso, el segundo parámetro es el tamaño del proceso en Memoria y
el tercer parámetro es la prioridad del hilo main (TID 0).
El Kernel creará un nuevo PCB y un TCB asociado con TID 0 y lo dejará en estado NEW.
*/

void PROCESS_CREATE(char *pseudocodigo, int tamanio_proceso, int prioridad)
{

    t_pcb *pcb = crear_pcb();
    pcb->estado = PCB_NEW;
    t_tcb* tcb = queue_peek(pcb->cola_hilos_new);
    tcb->pseudocodigo = pseudocodigo;
    queue_push(cola_new, pcb);
    pcb->tamanio_proceso = tamanio_proceso;
    pcb->prioridad = prioridad;
    sem_post(&semaforo_cola_new_procesos);
}

void proceso_exit()
{ // elimina los procesos que estan en la cola exit

    if (queue_size(cola_exit) == 0)
    {
        sem_wait(&semaforo_cola_exit_procesos);
    }

    int respuesta;
    code_operacion cod_op = PROCESS_ELIMINATE_COLA;
    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    send(socket_memoria, &cod_op, sizeof(code_operacion), 0);
    recv(socket_memoria, &respuesta, sizeof(int), 0);
    close(socket_memoria);
    if (respuesta == -1)
    {
        printf("Memoria la concha de tu madre ");
    }
    else
    {
        t_pcb *proceso = queue_pop(cola_exit);
        liberar_proceso(proceso);
        if (queue_size(cola_new) > 0)
        {
            sem_post(&semaforo_new_ready_procesos);
        }
    }
}
/*
Finalización de hilos
Al momento de finalizar un hilo, el Kernel deberá informar a la Memoria la
finalización del mismo, liberar su TCB asociado y deberá mover al estado READY a
todos los hilos que se encontraban bloqueados por ese TID. De esta manera, se desbloquean
aquellos hilos bloqueados por THREAD_JOIN o por mutex tomados por el hilo
finalizado (en caso que hubiera).
*/

void hilo_exit(t_pcb *pcb)
{
    if (queue_size(pcb->cola_hilos_exit) == 0)
    {
        sem_wait(&semaforo_cola_exit_hilos);
    }
    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    code_operacion cod_op = THREAD_ELIMINATE_AVISO;

    send(socket_memoria, &cod_op, sizeof(int), 0);
    close(socket_memoria);

    t_tcb *hilo = queue_pop(pcb->cola_hilos_exit);
    int tam_cola = queue_size(hilo->cola_hilos_bloqueados);
    if (tam_cola != 0)
    {

        for (int i = 0; i < tam_cola; i++)
        {
            t_tcb *tcb = queue_pop(hilo->cola_hilos_bloqueados);
            tcb->estado = TCB_READY;
            t_pcb *pcb_asociado = lista_pcb(lista_pcbs, tcb->pid);
            char *planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

            if (strcmp(planificacion, "FIFO") == 0 || strcmp(planificacion, "PRIORIDADES")== 0)
            {
                queue_push(pcb_asociado->cola_hilos_ready, tcb);
            }
            else if (strcmp(planificacion, "MULTINIVEL") == 0)
            {
                t_cola_prioridad *cola = cola_prioridad(pcb_asociado->colas_hilos_prioridad_ready, hilo->prioridad);
                queue_push(cola->cola, tcb);
            }
        }
    }
    liberar_tcb(hilo);
}

/*
Creación de hilos
Para la creación de hilos, el Kernel deberá informar a la Memoria y luego ingresarlo
directamente a la cola de READY correspondiente, según su nivel de prioridad.
*/
void new_a_ready_hilos(t_pcb *pcb)
{
    if (queue_size(pcb->cola_hilos_new) == 0)
    {
        sem_wait(&semaforo_cola_new_hilos);
    }
    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    code_operacion cod_op = THREAD_CREATE_AVISO;

    send(socket_memoria, &cod_op, sizeof(int), 0);
    close(socket_memoria);
    char *planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    t_tcb *hilo = queue_pop(pcb->cola_hilos_new);
    hilo->estado = TCB_READY;
    if (strcmp(planificacion, "FIFO") == 0 || strcmp(planificacion,"PRIORIDADES")==0)
    {
        queue_push(pcb->cola_hilos_ready, hilo);
    }
    if (strcmp(planificacion, "MULTINIVEL") == 0)
    {
        t_cola_prioridad *cola = cola_prioridad(pcb->colas_hilos_prioridad_ready, hilo->prioridad);
        queue_push(cola->cola, hilo);
    }
}

void new_a_ready_procesos() // Verificar contra la memoria si el proceso se puede inicializar, si es asi se envia el proceso a ready
{
    int respuesta = 1;
    if (queue_size(cola_new) == 0)
    {
        sem_wait(&semaforo_cola_new_procesos);
    }
    t_pcb *pcb = queue_peek(cola_new);

    code_operacion cod_op = PROCESS_CREATE_AVISO;

    int socket_memoria = cliente_Memoria_Kernel(logger, config);

    send(socket_memoria, &cod_op, sizeof(code_operacion), 0);
    send_pcb(pcb, socket_memoria); // Enviar pcb para que memoria verifique si tiene espacio para inicializar el proximo proceso
    recv(socket_memoria, &respuesta, sizeof(int), 0);
    close(socket_memoria);
    if (respuesta == -1)
    {
        sem_wait(&semaforo_new_ready_procesos);
    }
    else
    {
        pcb = queue_pop(cola_new);
        pcb->estado = PCB_READY;
        queue_push(cola_ready, pcb);
    }
}

/*
PROCESS_EXIT, esta syscall finalizará el PCB correspondiente al TCB que ejecutó la instrucción,
enviando todos sus TCBs asociados a la cola de EXIT. Esta instrucción sólo será llamada por el TID 0 del proceso
y le deberá indicar a la memoria la finalización de dicho proceso.
*/

void PROCESS_EXIT()
{
    if(proceso_exec->hilo_exec->tid != 0){
        log_info(logger,"Error, se intento ejecutar la syscall PROCESS_EXIT con un TID que no era el TID 0");
    return;
    }
    t_pcb *pcb = proceso_exec;
    proceso_exec = NULL;
    int respuesta;
    code_operacion cod_op = PROCESS_INTERRUPT;
    pthread_mutex_lock(&mutex_conexion_cpu);
    send(sockets->sockets_cliente_cpu->socket_Interrupt, &cod_op, sizeof(code_operacion), 0);
    recv(sockets->sockets_cliente_cpu->socket_Interrupt, &respuesta, sizeof(int), 0);
    pthread_mutex_unlock(&mutex_conexion_cpu);
    if (respuesta == -1)
    {
        log_info(logger,"Error en el desalojo del proceso");
    }
    else
    {
        pcb->estado = PCB_EXIT;
        queue_push(cola_exit, pcb);
        sem_post(&semaforo_cola_exit_procesos);
        if (queue_size(cola_new) > 0)
        {
            sem_post(&semaforo_new_ready_procesos);
        }
    }
}

void iniciar_kernel(char *archivo_pseudocodigo, int tamanio_proceso)
{
    t_pcb *pcb = crear_pcb();
    t_tcb* tcb = queue_peek(pcb->cola_hilos_new);
    tcb->pseudocodigo = archivo_pseudocodigo;
    pcb->tamanio_proceso = tamanio_proceso;
    queue_push(pcb->cola_hilos_new, tcb);
    list_add(lista_pcbs, pcb);
    hilo_planificador_largo_plazo();
}

/*Creación de hilos
Para la creación de hilos, el Kernel deberá informar a la Memoria y luego ingresarlo directamente a la cola de READY correspondiente, según su nivel de prioridad.
Finalización de hilos
Al momento de finalizar un hilo, el Kernel deberá informar a la Memoria la finalización del mismo,
liberar su TCB asociado y deberá mover al estado READY a todos los hilos que se encontraban bloqueados por ese TID.
De esta manera, se desbloquean aquellos hilos bloqueados por THREAD_JOIN o por mutex tomados por el hilo finalizado (en caso que hubiera).
*/

/*
THREAD_CREATE, esta syscall recibirá como parámetro de la CPU el nombre del archivo de pseudocódigo que deberá ejecutar el hilo a crear y su prioridad.
Al momento de crear el nuevo hilo, deberá generar el nuevo TCB con un TID autoincremental y poner al mismo en el estado READY.
*/

void THREAD_CREATE(char *pseudocodigo, int prioridad)
{

    t_pcb *pcb = proceso_exec;
    int resultado = 0;
    code_operacion cod_op = THREAD_CREATE_AVISO;

    int socket_memoria = cliente_Memoria_Kernel(logger, config);

    send(socket_memoria, &cod_op, sizeof(code_operacion), 0);
    recv(socket_memoria, &resultado, sizeof(int), 0);
    close(socket_memoria);

    if (resultado == -1)
    {
        printf("Re ortiva memoria");
        return;
    }
    else
    {

        t_tcb *tcb = crear_tcb(pcb);
        tcb->prioridad = prioridad;
        tcb->estado = TCB_READY;
        tcb->pseudocodigo = pseudocodigo;
    }
}

/*
THREAD_JOIN, esta syscall recibe como parámetro un TID, mueve el hilo que la invocó al
estado BLOCK hasta que el TID pasado por parámetro finalice. En caso de que el TID pasado
 por parámetro no exista o ya haya finalizado,
esta syscall no hace nada y el hilo que la invocó continuará su ejecución.*/

void THREAD_JOIN(int tid)
{
    int respuesta;
    if (lista_tcb(proceso_exec, tid) == -1 || tid_finalizado(proceso_exec, tid) == 0)
    {
        return;
    }
    t_tcb *tcb_aux = proceso_exec->hilo_exec;

    code_operacion cod_op = THREAD_INTERRUPT;
    pthread_mutex_lock(&mutex_conexion_cpu);
    send(sockets->sockets_cliente_cpu->socket_Interrupt, &cod_op, sizeof(code_operacion), 0);
    recv(sockets->sockets_cliente_cpu->socket_Interrupt, &respuesta, sizeof(int), 0);
    pthread_mutex_unlock(&mutex_conexion_cpu);
    if (respuesta == -1)
    {
        log_info(logger,"Error en el desalojo del proceso");
        return;
    }
    proceso_exec->hilo_exec = NULL;
    tcb_aux->estado = TCB_BLOCKED;
    list_add(proceso_exec->lista_hilos_blocked, tcb_aux);

    t_tcb *tcb_bloqueante = buscar_tcb_por_tid(proceso_exec, tid);
    queue_push(tcb_bloqueante->cola_hilos_bloqueados, tcb_aux);
}

/*
THREAD_CANCEL, esta syscall recibe como parámetro un TID con el objetivo de finalizarlo pasando al mismo al estado EXIT.
Se deberá indicar a la Memoria la finalización de dicho hilo. En caso de que el TID pasado por parámetro no exista o
ya haya finalizado, esta syscall no hace nada. Finalmente, el hilo que la invocó continuará su ejecución.
*/

void THREAD_CANCEL(int tid)
{ // suponiendo que el proceso main esta ejecutando

    int respuesta;
    code_operacion cod_op = THREAD_CANCEL_AVISO;

    t_tcb *tcb = buscar_tcb_por_tid(proceso_exec,tid); // Debido a que solamente hilos vinculados por un mismo proceso se pueden cancelar entre si, el tid a cancelar debe ser del proceso del hilo que llamo a la funcion

    if (tcb == NULL)
    {
        return;
    }

    if (lista_tcb(proceso_exec, tid) == -1 || tid_finalizado(proceso_exec, tid) == 0)
    {
        return;
    }

    int socket_memoria = cliente_Memoria_Kernel(logger, config);

    send(socket_memoria, &cod_op, sizeof(code_operacion), 0);
    send_tcb(tcb, socket_memoria);
    recv(socket_memoria, &respuesta, sizeof(int), 0);

    close(socket_memoria);

    if (respuesta == -1)
    {
        log_info(logger, "Error en la liberacion de memoria del hilo");
    }
    else
    {
        if(tcb->estado == TCB_EXECUTE){
        pthread_mutex_lock(&mutex_conexion_cpu);
        send(sockets->sockets_cliente_cpu->socket_Interrupt, &cod_op, sizeof(code_operacion), 0);
        recv(sockets->sockets_cliente_cpu->socket_Interrupt, &respuesta, sizeof(int), 0);
        pthread_mutex_unlock(&mutex_conexion_cpu);
    if (respuesta == -1)
    {
        log_info(logger,"Error en el desalojo del proceso");
    }
    
    proceso_exec->hilo_exec = NULL;
    tcb->estado = TCB_EXIT;
    queue_push(proceso_exec->cola_hilos_exit,tcb);
    sem_post(&semaforo_cola_exit_hilos);
        }
        move_tcb_to_exit(proceso_exec,tcb);
    }
}

/* THREAD_EXIT, esta syscall finaliza al hilo que lo invocó, 
pasando el mismo al estado EXIT. Se deberá indicar a la Memoria 
la finalización de dicho hilo.
*/

void THREAD_EXIT()
{
    move_tcb_to_exit(proceso_exec,proceso_exec->hilo_exec);
    sem_post(&semaforo_cola_exit_hilos);
}

/*
Mutex
Las syscalls que puede atender el kernel referidas a threads son 3: MUTEX_CREATE ,
MUTEX_LOCK, MUTEX_UNLOCK. Para las operaciones de MUTEX_LOCK y MUTEX_UNLOCK donde no se cumpla que el recurso exista, se deberá enviar el hilo a EXIT.

MUTEX_CREATE, crea un nuevo mutex para el proceso sin asignar a ningún hilo.

MUTEX_LOCK, se deberá verificar primero que exista el mutex solicitado y en caso de que exista y
el mismo no se encuentre tomado se deberá asignar dicho mutex al hilo correspondiente. En caso de
que el mutex se encuentre tomado, el hilo que realizó MUTEX_LOCK se bloqueará en la cola de bloqueados correspondiente a dicho mutex.

MUTEX_UNLOCK, se deberá verificar primero que exista el mutex solicitado y esté tomado por el hilo
que realizó la syscall. En caso de que corresponda, se deberá desbloquear al primer hilo de la cola
de bloqueados de ese mutex y le asignará el mutex al hilo recién desbloqueado. Una vez hecho esto, se
devuelve la ejecución al hilo que realizó la syscall MUTEX_UNLOCK. En caso de que el hilo que realiza
la syscall no tenga asignado el mutex, no realizará ningún desbloqueo.
*/

void MUTEX_CREATE()
{

    static int mutex_id = 0;

    t_mutex *mutex = malloc(sizeof(t_mutex));
    mutex->mutex_id = mutex_id;
    mutex->estado = UNLOCKED;
    mutex->cola_tcbs = queue_create();

    list_add(lista_mutex, mutex);
    list_add(proceso_exec->lista_mutex, mutex);

    mutex_id++;
}

void MUTEX_LOCK(int mutex_id)
{
    t_mutex *mutex_asociado = busqueda_mutex(lista_mutex, mutex_id);

    if (mutex_asociado == NULL)
    {

        queue_push(proceso_exec->cola_hilos_exit, proceso_exec->hilo_exec);
        proceso_exec->hilo_exec = NULL;
        return;
    }

    if (mutex_asociado->estado == UNLOCKED)
    {
        mutex_asociado->hilo = proceso_exec->hilo_exec;
        mutex_asociado->estado = LOCKED;
    }
    else
    {
        proceso_exec->hilo_exec->estado = TCB_BLOCKED_MUTEX;
        queue_push(mutex_asociado->cola_tcbs, proceso_exec->hilo_exec);
    }
}

void MUTEX_UNLOCK(int mutex_id)
{
    t_mutex *mutex_asociado = busqueda_mutex(lista_mutex, mutex_id);

    if (mutex_asociado == NULL)
    {

        queue_push(proceso_exec->cola_hilos_exit, proceso_exec->hilo_exec);
        proceso_exec->hilo_exec = NULL;
        return;
    }

    if (mutex_asociado->hilo != proceso_exec->hilo_exec)
    {

        return;
    }
    if (!queue_is_empty(mutex_asociado->cola_tcbs))
    {
        mutex_asociado->hilo = queue_pop(mutex_asociado->cola_tcbs);
        mutex_asociado->hilo->estado = TCB_READY;
    }
    else
    {
        mutex_asociado->estado = UNLOCKED;
        mutex_asociado->hilo = NULL;
    }
}

/*
Entrada Salida
Para la implementación de este trabajo práctico, el módulo Kernel simulará la existencia de un único dispositivo de Entrada Salida,
el cual atenderá las peticiones bajo el algoritmo FIFO. Para “utilizar” esta interfaz, se dispone de la syscall IO. Esta syscall recibe
como parámetro la cantidad de milisegundos que el hilo va a permanecer haciendo la operación de entrada/salida.
*/

void IO(int milisegundos)
{

    t_tcb *tcb = proceso_exec->hilo_exec;

    // Cambiar el estado del hilo a BLOCKED
    tcb->estado = TCB_BLOCKED;
    proceso_exec->hilo_exec = NULL;

    // Agregar el hilo a la lista de hilos bloqueados
    list_add(proceso_exec->lista_hilos_blocked, tcb);

    // Simular la espera por E/S
    usleep(milisegundos * 1000);

    // Sacar el hilo de la lista de bloqueados
    find_and_remove_tcb_in_list(proceso_exec->lista_hilos_blocked, tcb->tid);

    // Mover el hilo a la cola de READY
    tcb->estado = TCB_READY;
    t_cola_prioridad *cola = cola_prioridad(proceso_exec->colas_hilos_prioridad_ready, tcb->prioridad);
    queue_push(cola->cola, tcb);
}

/* En este apartado solamente se tendrá la instrucción DUMP_MEMORY. Esta syscall le solicita a la memoria,
junto al PID y TID que lo solicitó, que haga un Dump del proceso.
Esta syscall bloqueará al hilo que la invocó hasta que el módulo memoria confirme la finalización de la operación,
en caso de error, el proceso se enviará a EXIT. Caso contrario, el hilo se desbloquea normalmente pasando a READY.
*/

void DUMP_MEMORY()
{

    t_tcb *tcb = proceso_exec->hilo_exec;
    int rta_cpu;
    code_operacion cod_op = DUMP_MEMORIA;
    pthread_mutex_lock(&mutex_conexion_cpu);
    send(sockets->sockets_cliente_cpu->socket_Interrupt, &cod_op, sizeof(code_operacion), 0);
    recv(sockets->sockets_cliente_cpu->socket_Interrupt, &rta_cpu, sizeof(int), 0);
    pthread_mutex_unlock(&mutex_conexion_cpu);
    if (rta_cpu == -1)
    {
        log_info(logger, "Error en el desalojo del hilo ");
        return;
    }

    tcb->estado = TCB_BLOCKED;

    list_add(proceso_exec->lista_hilos_blocked, tcb);

    proceso_exec->hilo_exec = NULL;

    // Conectar con memoria

    int socket_memoria = cliente_Memoria_Kernel(logger, config);

    int pid = proceso_exec->pid;
    int tid = tcb->tid;

    t_paquete *paquete_dump = crear_paquete();
    agregar_a_paquete(paquete_dump, &pid, sizeof(int));
    agregar_a_paquete(paquete_dump, &tid, sizeof(int));
    enviar_paquete(paquete_dump, socket_memoria);

    int rta_memoria;

    recv(socket_memoria, &rta_memoria, sizeof(int), 0);
    close(socket_memoria);

    if (rta_memoria == -1)
    {
        log_info(logger, "Error en el dump de memoria ");
        PROCESS_EXIT(logger, config);
    }
    else
    {
        log_info(logger, "Dump de memoria exitoso");
        tcb->estado = TCB_READY;
        t_cola_prioridad *cola = cola_prioridad(proceso_exec->colas_hilos_prioridad_ready, tcb->prioridad);
        queue_push(cola->cola, tcb);
    }
}


// Ejecución
// Una vez seleccionado el siguiente hilo a ejecutar, se lo transicionará al estado EXEC y se enviará al módulo CPU el TID y su PID asociado a ejecutar a través del puerto de dispatch, quedando a la espera de recibir dicho TID después de la ejecución junto con un motivo por el cual fue devuelto.
// En caso que el algoritmo requiera desalojar al hilo en ejecución, se enviará una interrupción a través de la conexión de interrupt para forzar el desalojo del mismo.
// Al recibir el TID del hilo en ejecución, en caso de que el motivo de devolución implique replanificar, se seleccionará el siguiente hilo a ejecutar según indique el algoritmo. Durante este período la CPU se quedará esperando.

void ejecucion(t_tcb *hilo, t_queue *queue, int socket_dispatch)
{


    t_paquete *paquete = crear_paquete();
    hilo->estado = TCB_EXECUTE; // Una vez seleccionado el siguiente hilo a ejecutar, se lo transicionará al estado EXEC

    agregar_a_paquete(paquete, &hilo->tid, sizeof(hilo->tid));
    agregar_a_paquete(paquete, &hilo->pid, sizeof(hilo->pid));

    //code_operacion rtaCPU;

    // Se enviará al módulo CPU el TID y su PID asociado a ejecutar a través del puerto de dispatch, quedando a la espera de recibir dicho TID después de la ejecución junto con un motivo por el cual fue devuelto.
    enviar_paquete(paquete, socket_dispatch);
    //recv(socket_dispatch, &rtaCPU, sizeof(rtaCPU), 0);
    
    t_list* devolucionCPU = recibir_paquete(socket_dispatch);   // HAY QUE AGREGAR EL UTILS SERVER

    // Hacer un paquete con un tid y con un enum
   
    
    code_operacion motivo_devolucion = *(code_operacion*)list_get(devolucionCPU, 0);

    /*Agregué algunos de los códigos de operación subidos al módulo CPU. Había conflicto con algunos nombres por llamarse igual a algunas funciones que tenemos
    hechas, así que no agregué todos*/

    // en caso de que el motivo de devolución implique replanificar, se seleccionará el siguiente hilo a ejecutar según indique el algoritmo. Durante este período la CPU se quedará esperando.

    if (es_motivo_devolucion(motivo_devolucion))
    { //

        hilo->estado = TCB_READY;

        
        queue_push(queue, hilo);
        
    }

    if (motivo_devolucion == THREAD_EXIT_)
    {
        THREAD_EXIT();
    }
    
    eliminar_paquete(paquete);
    list_destroy(devolucionCPU);
}

