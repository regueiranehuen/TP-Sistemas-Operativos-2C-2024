#include "includes/procesos.h"

sem_t semaforo;
t_queue *cola_new;
t_queue *cola_ready;
t_list *lista_pcbs;
pthread_mutex_t mutex_pthread_join;
t_pcb *proceso_exec;

t_pcb *crear_pcb(t_tcb*tcb)
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

void inicializar_estados_hilos(t_pcb *pcb)   
{
    pcb->cola_hilos_new = queue_create();
    pcb->colas_hilos_prioridad_ready = list_create();
    pcb->lista_hilos_blocked = list_create();
    pcb->cola_hilos_exit = queue_create();
}

t_tcb *crear_tcb(t_pcb *pcb)
{

    t_tcb *tcb = malloc(sizeof(t_tcb));
    int tamanio_lista = list_size(pcb->tids);
    tcb->tid = tamanio_lista;
    int *tid = malloc(sizeof(int));
    *tid = tcb->tid;
    tcb->estado = "NEW";
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

t_pcb *PROCESS_CREATE(char *pseudocodigo, int tamanio_proceso, int prioridad)  
{
    t_tcb tcb_0;
    t_pcb *pcb = crear_pcb(&tcb_0);
    pcb->estado = "NEW";
    pcb->pseudocodigo = pseudocodigo;
    pcb->tamanio_proceso = tamanio_proceso;
    pcb->prioridad = prioridad;
    queue_push(pcb->cola_hilos_new, tcb_0);  
    return pcb;
}

void new_a_ready(int socket_memoria) // Verificar contra la memoria si el proceso se puede inicializar, si es asi se envia el proceso a ready
{
    int pedido = 1;
    t_pcb *pcb = queue_peek(cola_new);

    // Hacer serializacion del tipo pcb
    send(socket_memoria, pcb, sizeof(t_pcb), 0); // Enviar pcb para que memoria verifique si tiene espacio para inicializar el proximo proceso
    recv(socket_memoria, &pedido, sizeof(int), 0);

    if (pedido == -1)
    {
        sem_wait(&semaforo);
    }
    else
    {
        pcb = queue_pop(cola_new);
        pcb->estado = "READY";
        queue_push(cola_ready, pcb);
    }
}

/*
PROCESS_EXIT, esta syscall finalizará el PCB correspondiente al TCB que ejecutó la instrucción,
enviando todos sus TCBs asociados a la cola de EXIT. Esta instrucción sólo será llamada por el TID 0 del proceso
y le deberá indicar a la memoria la finalización de dicho proceso.
*/

void PROCESS_EXIT(t_log *log, t_config *config)
{ // Me parece que va sin parametros pero no se como verga saber que hilo llamo a esta funcion, aparte diría que hay que crear una conexion con memoria adentro de la función
    t_pcb *pcb = proceso_exec;
    int pedido;
    char *puerto = config_get_string_value(config, "PUERTO_MEMORIA");
    int socket_memoria = crear_conexion(log, config, puerto);
    send(socket_memoria, pcb, sizeof(t_pcb), 0);
    recv(socket_memoria, &pedido, sizeof(int), 0);
    close(socket_memoria);
    if (pedido == -1)
    {
        printf("Memoria la concha de tu madre ");
    }
    else
    {
        liberar_proceso(pcb);
        sem_post(&semaforo);
    }
}

t_pcb *lista_pcb(t_list *lista_pcbs, int pid)
{

    int tamanio = list_size(lista_pcbs);
    int i;
    for (i = 0; i < tamanio; i++)
    {
        t_pcb *pcb = list_get(lista_pcbs, i);
        if (pcb->pid == pid)
        {
            return pcb;
        }
    }
    return NULL;
}

void iniciar_kernel(char *archivo_pseudocodigo, int tamanio_proceso)
{
    t_pcb *pcb = crear_pcb();
    t_tcb *tcb = crear_tcb(pcb);
    pcb->pseudocodigo = archivo_pseudocodigo;
    pcb->tamanio_proceso = tamanio_proceso;
    pcb->cola_hilos_new = tcb;
    list_add(lista_pcbs, pcb);
}

void liberar_proceso(t_pcb *pcb)
{

    // Mandar todos los hilos de la cola new a la cola exit y destruir la primera

    int tamanio_cola_new = queue_size(pcb->cola_hilos_new);

    for (int i = 0; i < tamanio_cola_new; i++)
    {
        t_tcb *tcb = queue_pop(pcb->cola_hilos_new);
        queue_push(pcb->cola_hilos_exit, tcb);
    }

    queue_destroy(pcb->cola_hilos_new);

    // Mandar todos los hilos de la cola ready a la cola exit y destruir la primera

    int tamanio_lista = list_size(pcb->colas_hilos_prioridad_ready);

    for (int i = 0; i < tamanio_lista; i++)
    {
        t_cola_prioridad *cola = list_get(pcb->colas_hilos_prioridad_ready, i);
        int tamanio_cola = queue_size(cola->cola);

        for (int j = 0; j < tamanio_cola; j++)
        {
            t_tcb *tcb = queue_pop(cola->cola);
            queue_push(pcb->cola_hilos_exit, tcb);
        }
        queue_destroy(cola->cola);
        free(cola);
    }
    list_destroy(pcb->colas_hilos_prioridad_ready);

    // Mandar todos los hilos de la lista blocked a la cola exit y destruir la primera

    int tamanio_lista_blocked = list_size(pcb->lista_hilos_blocked);

    for (int i = 0; i < tamanio_lista_blocked; i++)
    {
        t_tcb *tcb = list_get(pcb->lista_hilos_blocked, i);
        queue_push(pcb->cola_hilos_exit, tcb);
    }
    list_destroy(pcb->lista_hilos_blocked);
    if (strcmp(pcb->hilo_exec->estado, "EXEC") == 0)
    {
        // mandar interrupcion a cpu
        queue_push(pcb->cola_hilos_exit, pcb->hilo_exec);
    }

    // Destruir la cola exit, lista de tids y liberar el espacio del pcb

    queue_destroy(pcb->cola_hilos_exit);

    list_destroy(pcb->tids);

    free(pcb);
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

t_tcb *THREAD_CREATE(char *pseudocodigo, int prioridad, t_log *log, t_config *config)
{ // Habria que ver como saber el proceso asociado a dicho hilo, no puede estar como parametro "creo"

    t_pcb *pcb = proceso_exec;
    int resultado = 0;
    char *puerto = config_get_string_value(config, "PUERTO_MEMORIA");
    int socket_memoria = crear_conexion(log, config, puerto);
    send(socket_memoria, &resultado, sizeof(int), 0);
    recv(socket_memoria, &resultado, sizeof(int), 0);
    close(socket_memoria);
    if (resultado == -1)
    {
        printf("Re ortiva memoria");
        return NULL;
    }
    else
    {
        t_tcb *tcb = crear_tcb(pcb);
        tcb->prioridad = prioridad;
        tcb->estado = "READY";
        tcb->pseudocodigo = pseudocodigo;
        t_cola_prioridad *cola = cola_prioridad(pcb->colas_hilos_prioridad_ready, prioridad);
        queue_push(cola->cola, tcb);
        return tcb;
    }
}

t_cola_prioridad *cola_prioridad(t_list *lista_colas_prioridad, int prioridad)
{ // Busca la cola con la prioridad del parametro en la lista de colas, si la encuentra devuelve la info de la posición de dicha lista, si no crea una y la devuelve

    int tamanio = list_size(lista_colas_prioridad);
    int i;
    for (i = 0; i < tamanio; i++)
    {
        t_cola_prioridad *cola = list_get(lista_colas_prioridad, i);
        if (cola->prioridad == prioridad)
        {
            return cola;
        }
    }
    t_cola_prioridad *cola = malloc(sizeof(t_cola_prioridad));
    cola->cola = queue_create();
    cola->prioridad = prioridad;
    list_add(lista_colas_prioridad, cola);
    return cola;
}
/*
THREAD_JOIN, esta syscall recibe como parámetro un TID, mueve el hilo que la invocó al
estado BLOCK hasta que el TID pasado por parámetro finalice. En caso de que el TID pasado
 por parámetro no exista o ya haya finalizado,
esta syscall no hace nada y el hilo que la invocó continuará su ejecución.*/

void THREAD_JOIN(int tid)
{ // falta logica para desbloquear hilo cuando termina de ejecutar el hilo asociado

    if (lista_tcb(proceso_exec, tid) == -1 || tid_finalizado(proceso_exec, tid) == -1)
    {
        return;
    }
    t_tcb *tcb_aux = proceso_exec->hilo_exec;
    proceso_exec->hilo_exec = NULL;
    // mandar interrupción a cpu para que desaloje el hilo, me imagino que esta en ejecucion cuando llamo a esta función
    list_add(proceso_exec->lista_hilos_blocked, tcb_aux);
}

int lista_tcb(t_pcb *pcb, int tid)
{ // Busca un tid en una lista de tids

    int tamanio = list_size(pcb->tids);

    for (int i = 0; i < tamanio; i++)
    {
        int *tid_aux = list_get(pcb->tids, i);
        if (tid_aux == tid)
        {
            return 0;
        }
    }
    return -1;
}

int tid_finalizado(t_pcb *pcb, int tid)
{

    int tamanio = queue_size(pcb->cola_hilos_exit);
    for (int i = 0; i < tamanio; i++)
    {
        int tid_aux = queue_peek(pcb->cola_hilos_exit, i);
        if (tid_aux == tid)
        {
            return 0;
        }
    }
    return -1;
}
