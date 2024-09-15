#include "includes/procesos.h"
#include "includes/funcionesAuxiliares.h"

sem_t semaforo;
t_queue *cola_new;
t_queue *cola_ready;
t_list *lista_pcbs;
pthread_mutex_t mutex_pthread_join;
t_pcb *proceso_exec;

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

    t_pcb *pcb = crear_pcb();
    pcb->estado = "NEW";
    pcb->pseudocodigo = pseudocodigo;
    pcb->tamanio_proceso = tamanio_proceso;
    pcb->prioridad = prioridad;
    return pcb;
}

void new_a_ready(int socket_memoria) // Verificar contra la memoria si el proceso se puede inicializar, si es asi se envia el proceso a ready
{
    int pedido = 1;
    t_pcb *pcb = queue_peek(cola_new);

    send_pcb(pcb,socket_memoria);
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
    char *ip = config_get_string_value(config, "IP_MEMORIA");
    int socket_memoria = crear_conexion(log, ip, puerto);

    send_pcb(pcb,socket_memoria);
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

void iniciar_kernel(char *archivo_pseudocodigo, int tamanio_proceso)
{
    t_pcb *pcb = crear_pcb();
    t_tcb *tcb = crear_tcb(pcb);
    pcb->pseudocodigo = archivo_pseudocodigo;
    pcb->tamanio_proceso = tamanio_proceso;
    pcb->cola_hilos_new = tcb;
    list_add(lista_pcbs, pcb);
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
    char *ip = config_get_string_value(config, "IP_MEMORIA");
    int socket_memoria = crear_conexion(log, ip, puerto);

    
    send(socket_memoria,&resultado,sizeof(int),0);
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

/*
THREAD_JOIN, esta syscall recibe como parámetro un TID, mueve el hilo que la invocó al
estado BLOCK hasta que el TID pasado por parámetro finalice. En caso de que el TID pasado
 por parámetro no exista o ya haya finalizado,
esta syscall no hace nada y el hilo que la invocó continuará su ejecución.*/

void THREAD_JOIN(int tid)
{ // falta logica para desbloquear hilo cuando termina de ejecutar el hilo asociado

    if (lista_tcb(proceso_exec, tid) == -1 || tid_finalizado(proceso_exec, tid) == 0)
    {
        return;
    }
    t_tcb *tcb_aux = proceso_exec->hilo_exec;
    proceso_exec->hilo_exec = NULL;
    // mandar interrupción a cpu para que desaloje el hilo, me imagino que esta en ejecucion cuando llamo a esta función
    list_add(proceso_exec->lista_hilos_blocked, tcb_aux);
}

/*
THREAD_CANCEL, esta syscall recibe como parámetro un TID con el objetivo de finalizarlo pasando al mismo al estado EXIT.
Se deberá indicar a la Memoria la finalización de dicho hilo. En caso de que el TID pasado por parámetro no exista o
ya haya finalizado, esta syscall no hace nada. Finalmente, el hilo que la invocó continuará su ejecución.
*/
void THREAD_CANCEL(int tid, t_config *config, t_log *log)
{ // suponiendo que el proceso main esta ejecutando

    t_cola_prioridad *cola = malloc(sizeof(t_cola_prioridad));
    int respuesta;

    t_tcb *tcb = buscar_tcb(tid, proceso_exec->cola_hilos_new, cola->cola, proceso_exec->lista_hilos_blocked);
    cola = cola_prioridad(proceso_exec->colas_hilos_prioridad_ready, tcb->prioridad);
    
    if (tcb == NULL)
    {
        return;
    }

    if (lista_tcb(proceso_exec, tid) == -1 || tid_finalizado(proceso_exec, tid) == 0)
    {
        return;
    }

    char *puerto = config_get_string_value(config, "PUERTO_MEMORIA");
    char *ip = config_get_string_value(config, "IP_MEMORIA");

    int socket_memoria = crear_conexion(log, ip, puerto);

    send_tcb(tcb,socket_memoria);
    recv(socket_memoria, &respuesta, sizeof(int), 0);

    if (respuesta == -1)
    {
        log_info(log, "Error en la liberacion de memoria del hilo");
        close(socket_memoria);
        return;
    }


    move_tcb_to_exit(proceso_exec->cola_hilos_new, cola->cola, proceso_exec->lista_hilos_blocked, proceso_exec->cola_hilos_exit, tid);


    close(socket_memoria);
}

///

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
    char *ip = config_get_string_value(config, "IP_MEMORIA");
    int socket_memoria = crear_conexion(log, ip, puerto);

    send_pcb(pcb,socket_memoria);
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




// En este apartado solamente se tendrá la instrucción DUMP_MEMORY. Esta syscall le solicita a la memoria, junto al PID y TID que lo solicitó, que haga un Dump del proceso.
// Esta syscall bloqueará al hilo que la invocó hasta que el módulo memoria confirme la finalización de la operación, en caso de error, el proceso se enviará a EXIT. Caso contrario, el hilo se desbloquea normalmente pasando a READY.


void DUMP_MEMORY(t_config*config,t_log*log){
    t_pcb *pcb = proceso_exec;
    t_tcb *tcb = pcb->hilo_exec;

    // Conectar con memoria
    char *puerto = config_get_string_value(config, "PUERTO_MEMORIA");
    char *ip = config_get_string_value(config, "IP_MEMORIA");
    int socket_memoria = crear_conexion(log, ip, puerto);

    int pid = pcb->pid;
    int tid = tcb->tid;

    char *mensaje = "DUMP_MEMORY";
    int tam_mensaje = strlen(mensaje) + 1;

    t_paquete *paquete_dump = crear_paquete();
    agregar_a_paquete(paquete_dump,&pid,sizeof(int));
    agregar_a_paquete(paquete_dump,&tid,sizeof(int));
    agregar_a_paquete(paquete_dump,mensaje,tam_mensaje);
    enviar_paquete(paquete_dump,socket_memoria);

    int rta;
    
    recv(socket_memoria,&rta,sizeof(int),0);
    
    if(rta == -1){
        log_info(log, "Error en el dump de memoria ");
        PROCESS_EXIT(log,config);
    }
    else{
        log_info(log,"Dump de memoria exitoso");
        tcb->estado = "READY";
    }
    
    close(socket_memoria);
}

// Entrada Salida
// Para la implementación de este trabajo práctico, el módulo Kernel simulará la existencia de un único dispositivo de Entrada Salida, el cual atenderá las peticiones bajo el algoritmo FIFO. Para “utilizar” esta interfaz, se dispone de la syscall IO. Esta syscall recibe como parámetro la cantidad de milisegundos que el hilo va a permanecer haciendo la operación de entrada/salida.

void IO(int milisegundos) {
    t_pcb *pcb = proceso_exec;
    t_tcb *tcb = proceso_exec->hilo_exec;

    // Cambiar el estado del hilo a BLOCKED
    tcb->estado = "BLOCKED";

    // Agregar el hilo a la lista de hilos bloqueados
    list_add(pcb->lista_hilos_blocked, tcb);

    // Simular la espera por E/S
    usleep(milisegundos * 1000);

    // Sacar el hilo de la lista de bloqueados
    find_and_remove_tcb_in_list(pcb->lista_hilos_blocked, tcb->tid);

    // Mover el hilo a la cola de READY
    tcb->estado = "READY";
    queue_push(cola_ready, tcb);
}