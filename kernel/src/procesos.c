#include "includes/procesos.h"
#include "includes/funcionesAuxiliares.h"
#include "includes/planificacion.h"

sem_t semaforo_new_ready_procesos;
sem_t semaforo_cola_new_procesos;
sem_t semaforo_cola_exit_procesos;

sem_t semaforo_cola_exit_hilos;
sem_t sem_lista_prioridades;
sem_t sem_lista_pcbs;

sem_t sem_syscall;

t_queue *cola_ready_fifo;
t_list *lista_ready_prioridad;
t_list *colas_ready_prioridad;
t_queue *cola_exit;
t_queue *cola_exit_procesos;
t_tcb *hilo_exec;
t_queue *cola_new_procesos;
t_list* lista_bloqueados;

t_list *lista_tcbs;
t_list *lista_pcbs;
t_list *lista_mutex;

t_config *config;
t_log *logger;
sockets_kernel *sockets;

pthread_mutex_t mutex_lista_pcbs;
pthread_mutex_t mutex_cola_new_procesos;
pthread_mutex_t mutex_cola_exit_procesos;
pthread_mutex_t mutex_cola_exit_hilos;
pthread_mutex_t mutex_pthread_join;
pthread_mutex_t mutex_conexion_cpu;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_ready_fifo;
pthread_mutex_t mutex_ready_prioridades;
pthread_mutex_t mutex_ready_multinivel;
pthread_mutex_t mutex_cola_ready;

sem_t sem_desalojado;

bool desalojado;

t_pcb *crear_pcb()
{
    static int pid = 0;
    t_pcb *pcb = malloc(sizeof(t_pcb));
    t_list *lista_tids = list_create();
    pcb->pid = pid;
    pcb->tids = lista_tids;
    pid += 1;
    pcb ->contador_tid = 0;
    pcb ->contador_mutex = 0;
    inicializar_mutex_procesos(pcb);
    pthread_mutex_lock(&mutex_lista_pcbs);
    list_add(lista_pcbs, pcb);
    sem_post(&sem_lista_pcbs);
    pthread_mutex_unlock(&mutex_lista_pcbs);
    return pcb;
}

t_tcb *crear_tcb(t_pcb *pcb)
{

    t_tcb *tcb = malloc(sizeof(t_tcb));
    tcb->tid = pcb->contador_tid;
    int *tid = malloc(sizeof(int));
    *tid = tcb->tid;
    tcb->estado = TCB_NEW;
    tcb->pid = pcb->pid;
    pthread_mutex_lock(&pcb->mutex_tids);
    list_add(pcb->tids, tid);
    pthread_mutex_unlock(&pcb->mutex_tids);
    if (tcb->tid == 0)
    {
        tcb->prioridad = 0;
    }
    pcb->contador_tid += 1;
    return tcb;
}

void iniciar_kernel(char *archivo_pseudocodigo, int tamanio_proceso)
{
    t_pcb *pcb = crear_pcb();
    t_tcb* tcb = crear_tcb(pcb);
    tcb->pseudocodigo = archivo_pseudocodigo;
    pcb->tamanio_proceso = tamanio_proceso;
    tcb->prioridad = 0;
    pcb->tcb_main = tcb;
    pcb->estado = PCB_NEW;
    pthread_mutex_lock(&mutex_lista_pcbs);
    list_add(lista_pcbs,pcb);
    pthread_mutex_unlock(&mutex_lista_pcbs);

    pthread_mutex_lock(&mutex_cola_new_procesos);
    queue_push(cola_new_procesos, pcb);
    pthread_mutex_unlock(&mutex_cola_new_procesos);
    
    planificador_largo_plazo();
    planificador_corto_plazo();

    sem_post(&semaforo_cola_new_procesos);
 
}

void proceso_exit()
{ // elimina los procesos que estan en la cola exit

    
    sem_wait(&semaforo_cola_exit_procesos); //espera que haya elementos en la cola

    int respuesta;
    code_operacion cod_op = PROCESS_EXIT_AVISO;
    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    send(socket_memoria, &cod_op, sizeof(code_operacion), 0);
    recv(socket_memoria, &respuesta, sizeof(int), 0);
    close(socket_memoria);
    if (respuesta == -1)
    {
        printf("Error en la eliminación del proceso en memoria");
    }
    else
    {
        pthread_mutex_lock(&mutex_cola_exit_procesos);
        t_pcb *proceso = queue_pop(cola_exit_procesos);
        pthread_mutex_unlock(&mutex_cola_exit_procesos);
        liberar_proceso(proceso);
        sem_post(&semaforo_new_ready_procesos);
        
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

void hilo_exit()
{
   
    sem_wait(&semaforo_cola_exit_hilos);
    
    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    code_operacion cod_op = THREAD_ELIMINATE_AVISO;

    send(socket_memoria, &cod_op, sizeof(cod_op), 0);
    close(socket_memoria);
    pthread_mutex_lock(&mutex_cola_exit_hilos);
    t_tcb *hilo = queue_pop(cola_exit);
    pthread_mutex_unlock(&mutex_cola_exit_hilos);
    int tam_cola = queue_size(hilo->cola_hilos_bloqueados);
    if (tam_cola != 0)
    {

        for (int i = 0; i < tam_cola; i++)
        {
            pthread_mutex_lock(&hilo->mutex_cola_hilos_bloqueados);
            t_tcb *tcb = queue_pop(hilo->cola_hilos_bloqueados);
            pthread_mutex_unlock(&hilo->mutex_cola_hilos_bloqueados);
            tcb->estado = TCB_READY;
            buscar_y_eliminar_tcb(lista_bloqueados,tcb);
            pushear_cola_ready(tcb);
        }
    }
    liberar_tcb(hilo);
}



/*
Creación de procesos
Se tendrá una cola NEW que será administrada estrictamente por FIFO para la creación de procesos. 
Al llegar un nuevo proceso a esta cola y la misma esté vacía se enviará un pedido a Memoria para inicializar el mismo. 
Si la respuesta es positiva se crea el TID 0 de ese proceso y se lo pasa al estado READY y se sigue la misma lógica con el proceso que sigue. 
Si la respuesta es negativa (ya que la Memoria no tiene espacio suficiente para inicializarlo) se deberá esperar la finalización de otro proceso 
para volver a intentar inicializarlo.
Al llegar un proceso a esta cola y haya otros esperando, el mismo simplemente se encola.

*/
void new_a_ready_procesos() // Verificar contra la memoria si el proceso se puede inicializar, si es asi se envia el proceso a ready
{
    int respuesta = 1;
    
    sem_wait(&semaforo_cola_new_procesos);
    
    pthread_mutex_lock(&mutex_cola_new_procesos);
    t_pcb *pcb = queue_peek(cola_new_procesos);
    pthread_mutex_unlock(&mutex_cola_new_procesos);

    code_operacion cod_op = PROCESS_CREATE_AVISO;

    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    int tamanio_proceso = pcb -> tamanio_proceso;
    send(socket_memoria, &cod_op, sizeof(cod_op), 0);
    send(socket_memoria, &tamanio_proceso, sizeof(int),0);
    recv(socket_memoria, &respuesta, sizeof(int), 0);
    close(socket_memoria);
    if (respuesta == -1)
    {
        sem_wait(&semaforo_new_ready_procesos); //espera a que se libere un proceso
    }
    else
    {
        pthread_mutex_lock(&mutex_cola_new_procesos);
        pcb = queue_pop(cola_new_procesos);
        pthread_mutex_unlock(&mutex_cola_new_procesos);
        pcb->estado = PCB_READY;
        pcb->tcb_main->estado = TCB_READY;
        pushear_cola_ready(pcb->tcb_main);
    }
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
    t_tcb *tcb = crear_tcb(pcb);
    tcb->pseudocodigo = pseudocodigo;
    pcb->tamanio_proceso = tamanio_proceso;
    tcb->prioridad = prioridad;
    pcb->tcb_main = tcb;
    pcb->estado = PCB_NEW;

    log_info(logger, "## (%d:0) Se crea el proceso - Estado: NEW", pcb->pid);
    
    pthread_mutex_lock(&mutex_lista_pcbs);
    list_add(lista_pcbs,pcb);
    pthread_mutex_unlock(&mutex_lista_pcbs);

    pthread_mutex_lock(&mutex_cola_new_procesos);
    queue_push(cola_new_procesos, pcb);
    pthread_mutex_unlock(&mutex_cola_new_procesos);
    
    sem_post(&semaforo_cola_new_procesos);
    
}

/*
PROCESS_EXIT, esta syscall finalizará el PCB correspondiente al TCB que ejecutó la instrucción,
enviando todos sus TCBs asociados a la cola de EXIT. Esta instrucción sólo será llamada por el TID 0 del proceso
y le deberá indicar a la memoria la finalización de dicho proceso.
*/

void PROCESS_EXIT()
{
    if(hilo_exec->tid != 0){
        log_info(logger,"Error, se intento ejecutar la syscall PROCESS_EXIT con un TID que no era el TID 0");
    return;
    }
    t_pcb *pcb = buscar_pcb_por_pid(lista_pcbs, hilo_exec->pid);


        pcb->estado = PCB_EXIT;
        pthread_mutex_lock(&mutex_cola_exit_procesos);
        queue_push(cola_exit_procesos, pcb);
        pthread_mutex_unlock(&mutex_cola_exit_procesos);

        sem_post(&semaforo_cola_exit_procesos);
        sem_post(&sem_desalojado);
        desalojado = true;

    log_info(logger, "## Finaliza el proceso %d", pcb->pid);
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

    int resultado = 0;
    code_operacion cod_op = THREAD_CREATE_AVISO;

    int socket_memoria = cliente_Memoria_Kernel(logger, config);

    send(socket_memoria, &cod_op, sizeof(code_operacion), 0);
    recv(socket_memoria, &resultado, sizeof(int), 0);
    close(socket_memoria);

    if (resultado == -1)
    {
        log_error(logger, "## (%d:%d) - Error en la creación del hilo en Memoria", hilo_exec->pid, hilo_exec->tid);
        return;
    }
    else
    {
        t_pcb* pcb = buscar_pcb_por_pid(lista_pcbs,hilo_exec->pid);
        t_tcb *tcb = crear_tcb(pcb);
        tcb->prioridad = prioridad;
        tcb->estado = TCB_READY;
        tcb->pseudocodigo = pseudocodigo;
        pushear_cola_ready(tcb);

        log_info(logger, "## (%d:%d) Se crea el Hilo - Estado: READY", pcb->pid, tcb->tid);
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
    if (buscar_tcb_por_tid(lista_tcbs,tid) == NULL || buscar_tcb_por_tid(lista_bloqueados, tid) != NULL)
    {
        return;
    }
    t_tcb *tcb_aux = hilo_exec;

    code_operacion cod_op = THREAD_INTERRUPT;
    pthread_mutex_lock(&mutex_conexion_cpu);
    send(sockets->sockets_cliente_cpu->socket_Interrupt, &cod_op, sizeof(code_operacion), 0);
    recv(sockets->sockets_cliente_cpu->socket_Interrupt, &respuesta, sizeof(int), 0);
    pthread_mutex_unlock(&mutex_conexion_cpu);
    if (respuesta == -1)
    {
        log_info(logger, "## (%d:%d) - Error en el desalojo del proceso durante THREAD_JOIN", hilo_exec->pid, hilo_exec->tid);
        return;
    }
    hilo_exec = NULL;
    tcb_aux->estado = TCB_BLOCKED;
    list_add(lista_bloqueados, tcb_aux);

    log_info(logger, "## (%d:%d) - Bloqueado por: THREAD_JOIN", tcb_aux->pid, tcb_aux->tid);


    t_tcb *tcb_bloqueante = buscar_tcb_por_tid(lista_tcbs, tid);
    queue_push(tcb_bloqueante->cola_hilos_bloqueados, tcb_aux);
    sem_post(&sem_desalojado);
    desalojado = true;
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

    t_tcb *tcb = buscar_tcb_por_tid(lista_tcbs,tid); // Debido a que solamente hilos vinculados por un mismo proceso se pueden cancelar entre si, el tid a cancelar debe ser del proceso del hilo que llamo a la funcion

    if (tcb == NULL)
    {
        return;
    }

    if (buscar_tcb_por_tid(lista_tcbs,tid) == NULL || buscar_tcb_por_tid(lista_bloqueados, tid) != NULL)
    {
        return;
    }

    int socket_memoria = cliente_Memoria_Kernel(logger, config);

    send_operacion_entero(cod_op,tcb->tid,socket_memoria);
    recv(socket_memoria, &respuesta, sizeof(int), 0);

    close(socket_memoria);

    if (respuesta == -1)
    {
         log_info(logger, "## (%d:%d) - Error en la liberación de memoria del hilo", tcb->pid, tcb->tid);
    }
    else
    {

    if (tcb->estado == TCB_READY){
        char* algoritmo = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
        if(strcmp(algoritmo,"FIFO")== 0){
            pthread_mutex_lock(&mutex_cola_ready);
            sacar_tcb_de_cola(cola_ready_fifo,tcb);
            pthread_mutex_unlock(&mutex_cola_ready);
        }
        else if(strcmp(algoritmo,"PRIORIDADES")){
            pthread_mutex_lock(&mutex_cola_ready);
            sacar_tcb_de_lista(lista_ready_prioridad,tcb);
            pthread_mutex_unlock(&mutex_cola_ready);
        }
        else if(strcmp(algoritmo,"MULTINIVEL")){
            pthread_mutex_lock(&mutex_cola_ready);
            t_cola_prioridad* cola = cola_prioridad(colas_ready_prioridad,tcb->prioridad);
            sacar_tcb_de_cola(cola->cola,tcb);
            pthread_mutex_unlock(&mutex_cola_ready);
        }
    }
    else{
        sacar_tcb_de_lista(lista_bloqueados,tcb);
    }
    tcb->estado = TCB_EXIT;
    pthread_mutex_lock(&mutex_cola_exit_hilos);
    queue_push(cola_exit,tcb);
    pthread_mutex_unlock(&mutex_cola_exit_hilos);
    sem_post(&semaforo_cola_exit_hilos);

    }
} 


/* THREAD_EXIT, esta syscall finaliza al hilo que lo invocó, 
pasando el mismo al estado EXIT. Se deberá indicar a la Memoria 
la finalización de dicho hilo.
*/

void THREAD_EXIT()
{
    t_tcb* hilo = hilo_exec;
    hilo->estado = TCB_EXIT;
    hilo_exec = NULL;
    pthread_mutex_lock(&mutex_cola_exit_hilos);
    queue_push(cola_exit,hilo);
    pthread_mutex_unlock(&mutex_cola_exit_hilos);
    sem_post(&semaforo_cola_exit_hilos);
    sem_post(&sem_desalojado);
    desalojado = true;

    log_info(logger, "## (%d:%d) Finaliza el hilo", hilo->pid, hilo->tid);
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

void MUTEX_CREATE(char* recurso)//supongo que el recurso es el nombre del mutex
{
    t_pcb* proceso_asociado = buscar_pcb_por_pid(lista_pcbs,hilo_exec->pid);
    t_mutex *mutex = malloc(sizeof(t_mutex));
    mutex->estado = UNLOCKED;
    mutex->cola_tcbs = queue_create();
    mutex->nombre = recurso;

    list_add(lista_mutex, mutex);
    list_add(proceso_asociado->lista_mutex, mutex);
}

void MUTEX_LOCK(char* recurso)
{
    t_mutex *mutex_asociado = busqueda_mutex(lista_mutex, recurso);
    t_tcb* hilo_aux = hilo_exec;
    if (mutex_asociado == NULL)
    {
        log_info(logger, "## (%d:%d) - Bloqueado por: MUTEX", hilo_aux->pid, hilo_aux->tid);
        pthread_mutex_lock(&mutex_cola_exit_hilos);
        queue_push(cola_exit,hilo_aux);
        pthread_mutex_unlock(&mutex_cola_exit_hilos);
        sem_post(&sem_desalojado);
        desalojado = true;
        hilo_exec = NULL;
        return;
    }

    if (mutex_asociado->estado == UNLOCKED)
    {
        mutex_asociado->hilo = hilo_aux;
        mutex_asociado->estado = LOCKED;
    }
    else
    {
        hilo_aux->estado = TCB_BLOCKED_MUTEX;
        hilo_exec = NULL;
        list_add(lista_bloqueados,hilo_aux);
        queue_push(mutex_asociado->cola_tcbs, hilo_aux);
        sem_post(&sem_desalojado);
        desalojado = true;
    }
}

void MUTEX_UNLOCK(char* recurso)
{
    t_mutex *mutex_asociado = busqueda_mutex(lista_mutex, recurso);
    t_tcb* hilo_aux = hilo_exec;
    if (mutex_asociado == NULL)
    {
        pthread_mutex_lock(&mutex_cola_exit_hilos);
        queue_push(cola_exit,hilo_aux);
        pthread_mutex_unlock(&mutex_cola_exit_hilos);
        hilo_exec = NULL;
        sem_post(&sem_desalojado);
        desalojado = true;
        return;
    }

    if (mutex_asociado->hilo != hilo_exec)
    {
        return;
    }
    if (!queue_is_empty(mutex_asociado->cola_tcbs))
    {
        mutex_asociado->hilo = queue_pop(mutex_asociado->cola_tcbs);
        sacar_tcb_por_tid(lista_bloqueados,mutex_asociado->hilo->tid);
        mutex_asociado->hilo->estado = TCB_READY;
        t_tcb* tcb = mutex_asociado->hilo;
        pushear_cola_ready(tcb);
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
el cual atenderá las peticiones bajo el algoritmo FIFO. Para utilizar esta interfaz, se dispone de la syscall IO. Esta syscall recibe
como parámetro la cantidad de milisegundos que el hilo va a permanecer haciendo la operación de entrada/salida.
*/

void IO(int milisegundos)
{

    t_tcb *tcb = hilo_exec;

    // Cambiar el estado del hilo a BLOCKED
    tcb->estado = TCB_BLOCKED;
    hilo_exec = NULL;

    // Agregar el hilo a la lista de hilos bloqueados
    list_add(lista_bloqueados, tcb);

    log_info(logger, "## (%d:%d) - Bloqueado por: IO", tcb->pid, tcb->tid);

    // Simular la espera por E/S
    usleep(milisegundos * 1000);

    // Sacar el hilo de la lista de bloqueados
    find_and_remove_tcb_in_list(lista_bloqueados, tcb->tid);

    // Mover el hilo a la cola de READY
    tcb->estado = TCB_READY;
    pushear_cola_ready(tcb);
    
}

/* En este apartado solamente se tendrá la instrucción DUMP_MEMORY. Esta syscall le solicita a la memoria,
junto al PID y TID que lo solicitó, que haga un Dump del proceso.
Esta syscall bloqueará al hilo que la invocó hasta que el módulo memoria confirme la finalización de la operación,
en caso de error, el proceso se enviará a EXIT. Caso contrario, el hilo se desbloquea normalmente pasando a READY.
*/

void DUMP_MEMORY()
{

    t_tcb *tcb = hilo_exec;
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
    hilo_exec = NULL;
    tcb->estado = TCB_BLOCKED;

    list_add(lista_bloqueados, tcb);

    // Conectar con memoria

    int socket_memoria = cliente_Memoria_Kernel(logger, config);

    int pid = tcb->pid;
    int tid = tcb->tid;



    send_operacion_tid_pid(DUMP_MEMORIA,pid,tid,socket_memoria);
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
        pushear_cola_ready(tcb);
    }
}


/*Ejecución
Una vez seleccionado el siguiente hilo a ejecutar, se lo transicionará al estado EXEC y se enviará al módulo CPU el TID y su PID 
asociado a ejecutar a través del puerto de dispatch, quedando a la espera de recibir dicho TID después de la ejecución junto con un motivo por el cual fue devuelto.
En caso que el algoritmo requiera desalojar al hilo en ejecución, se enviará una interrupción a través de la conexión de interrupt para forzar el desalojo del mismo.
Al recibir el TID del hilo en ejecución, en caso de que el motivo de devolución implique replanificar, se seleccionará el siguiente hilo a ejecutar según indique 
el algoritmo. Durante este período la CPU se quedará esperando.
*/


/*
Creación de hilos
Para la creación de hilos, el Kernel deberá informar a la Memoria y luego ingresarlo
directamente a la cola de READY correspondiente, según su nivel de prioridad.

void new_a_ready_hilos(t_pcb *pcb)
{
    
    sem_wait(&semaforo_cola_new_hilos);
    
    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    code_operacion cod_op = THREAD_CREATE_AVISO;

    send(socket_memoria, &cod_op, sizeof(int), 0);
    close(socket_memoria);
    char *planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    t_tcb *hilo = queue_pop(pcb->cola_hilos_new);
    hilo->estado = TCB_READY;
    if (strcmp(planificacion, "FIFO") == 0){
        queue_push(cola_ready_fifo, hilo);
    } else if(strcmp(planificacion,"PRIORIDADES")==0)
    {
        list_add(lista_ready_prioridad,hilo);
        sem_post(&sem_lista_prioridades);
    }
    if (strcmp(planificacion, "MULTINIVEL") == 0)
    {
        t_cola_prioridad *cola = cola_prioridad(colas_ready_prioridad, hilo->prioridad);
        queue_push(cola->cola, hilo);
    }
}
*/