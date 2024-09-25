#include "includes/procesos.h"
#include "includes/funcionesAuxiliares.h"

#include "includes/planificacion.h"

sem_t semaforo;
t_queue *cola_new;
t_queue *cola_ready;
t_list *lista_pcbs;
pthread_mutex_t mutex_pthread_join;
t_pcb *proceso_exec;
t_config *config;
t_log *logger;
t_list *lista_mutex;

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
de pseudocódigo que deberá ejecutar el proceso, el segundo parámeetro es l tamaño del proceso en Memoria y
el tercer parámetro es la prioridad del hilo main (TID 0).
El Kernel creará un nuevo PCB y un TCB asociado con TID 0 y lo dejará en estado NEW.
*/

void PROCESS_CREATE(char *pseudocodigo, int tamanio_proceso, int prioridad)
{

    t_pcb *pcb = crear_pcb();
    t_tcb *tcb = crear_tcb(pcb);
    pcb->estado = PCB_NEW;
    tcb->pseudocodigo = pseudocodigo;
    queue_push(cola_new, pcb);
    pcb->tamanio_proceso = tamanio_proceso;
    pcb->prioridad = prioridad;
}

void new_a_ready() // Verificar contra la memoria si el proceso se puede inicializar, si es asi se envia el proceso a ready
{
    int pedido = 1;
    t_pcb *pcb = queue_peek(cola_new);

    char *ip = config_get_string_value(config, "IP_MEMORIA");
    char *puerto = config_get_string_value(config, "PUERTO_MEMORIA");
    int socket_memoria = crear_conexion(logger, ip, puerto);
    // Hacer serializacion del tipo pcb
    code_operacion code = PEDIDO_MEMORIA_INICIALIZAR_PROCESO;
    send_pcb(pcb,code,socket_memoria);
    recv(socket_memoria, &pedido, sizeof(int), 0);
    close(socket_memoria);
    if (pedido == -1)
    {
        sem_wait(&semaforo);
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
{ // Me parece que va sin parametros pero no se como verga saber que hilo llamo a esta funcion, aparte diría que hay que crear una conexion con memoria adentro de la función

    t_pcb *pcb = proceso_exec;
    int pedido;
    char *puerto = config_get_string_value(config, "PUERTO_MEMORIA");
    char *ip = config_get_string_value(config, "IP_MEMORIA");
    int socket_memoria = crear_conexion(logger, ip, puerto);

    code_operacion code = PEDIDO_MEMORIA_TERMINAR_PROCESO;

    send_pcb(pcb,code,socket_memoria);
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
    tcb->pseudocodigo = archivo_pseudocodigo;
    pcb->tamanio_proceso = tamanio_proceso;
    queue_push(pcb->cola_hilos_new, tcb);
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

void THREAD_CREATE(char *pseudocodigo, int prioridad) 
{

    t_pcb *pcb = proceso_exec;
    int resultado = 0;

    char *puerto = config_get_string_value(config, "PUERTO_MEMORIA");
    char *ip = config_get_string_value(config, "IP_MEMORIA");

    int socket_memoria = crear_conexion(logger, ip, puerto);
    send_pcb(pcb,PEDIDO_MEMORIA_THREAD_CREATE,socket_memoria);
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
        t_cola_prioridad *cola = cola_prioridad(pcb->colas_hilos_prioridad_ready, prioridad);
        queue_push(cola->cola, tcb);
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

void THREAD_CANCEL(int tid)  
{ // suponiendo que el proceso main esta ejecutando

    t_cola_prioridad *cola = malloc(sizeof(t_cola_prioridad));
    int respuesta;

    t_tcb *tcb = buscar_tcb(tid, proceso_exec->cola_hilos_new, proceso_exec->colas_hilos_prioridad_ready, proceso_exec->lista_hilos_blocked); // busca el tcb asociado al tid dado por parametro

    if (tcb == NULL)
    {
        return;
    }

    if (lista_tcb(proceso_exec, tid) == -1 || tid_finalizado(proceso_exec, tid) == 0)
    {
        return;
    }



    // Podriamos hacer las 3 lineas de abajo en una unica funcion
    char *puerto = config_get_string_value(config, "PUERTO_MEMORIA");
    char *ip = config_get_string_value(config, "IP_MEMORIA");
    int socket_memoria = crear_conexion(logger, ip, puerto);


    send_tcb(tcb,PEDIDO_MEMORIA_THREAD_EXIT,socket_memoria);
    recv(socket_memoria, &respuesta, sizeof(int), 0);

    if (respuesta == -1)
    {
        log_info(logger, "Error en la liberacion de memoria del hilo");
        close(socket_memoria);
        return;
    }

    move_tcb_to_exit(proceso_exec->cola_hilos_new, cola->cola, proceso_exec->lista_hilos_blocked, proceso_exec->cola_hilos_exit, tid);

    close(socket_memoria);
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

void IO(int milisegundos) {


    t_tcb* tcb = proceso_exec->hilo_exec;

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
    t_cola_prioridad* cola = cola_prioridad(proceso_exec->colas_hilos_prioridad_ready,tcb->prioridad);
    queue_push(cola->cola, tcb);
}


/* En este apartado solamente se tendrá la instrucción DUMP_MEMORY. Esta syscall le solicita a la memoria, 
junto al PID y TID que lo solicitó, que haga un Dump del proceso.
Esta syscall bloqueará al hilo que la invocó hasta que el módulo memoria confirme la finalización de la operación, 
en caso de error, el proceso se enviará a EXIT. Caso contrario, el hilo se desbloquea normalmente pasando a READY.
*/

void DUMP_MEMORY(){

    t_tcb* tcb = proceso_exec->hilo_exec;
    int rta_cpu;

    char* puerto_cpu = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    char* ip_cpu = config_get_string_value(config, "IP_CPU");
    int socket_cpu = crear_conexion(logger, ip_cpu, puerto_cpu);

    code_operacion code = DUMP_MEMORIA;
    send_tcb(proceso_exec->hilo_exec,code,socket_cpu);
    recv(socket_cpu,&rta_cpu,sizeof(int),0);

    if(rta_cpu == -1){
        log_info(logger, "Error en el desalojo del hilo ");
        return;
    }
    close(socket_cpu);
    tcb->estado = TCB_BLOCKED;

    list_add(proceso_exec->lista_hilos_blocked,tcb);

    proceso_exec->hilo_exec = NULL;


    // Conectar con memoria
    char* puerto = config_get_string_value(config, "PUERTO_MEMORIA");
    char* ip = config_get_string_value(config, "IP_MEMORIA");
    int socket_memoria = crear_conexion(logger, ip, puerto);


    int pid = proceso_exec->pid;
    int tid = tcb->tid; 
    code_operacion codigo= DUMP_MEMORIA; ///////////////
    t_paquete *paquete_dump = crear_paquete();

    agregar_a_paquete(paquete_dump,&codigo,sizeof(int)); //////////////
    agregar_a_paquete(paquete_dump,&pid,sizeof(int));
    agregar_a_paquete(paquete_dump,&tid,sizeof(int));
    enviar_paquete(paquete_dump,socket_memoria);

    int rta_memoria;

    recv(socket_memoria,&rta_memoria,sizeof(int),0);
    close(socket_memoria);

    if(rta_memoria == -1){
        log_info(logger, "Error en el dump de memoria ");
        
        PROCESS_EXIT();
    }
    else{
        log_info(logger,"Dump de memoria exitoso");
        tcb->estado = TCB_READY;
        t_cola_prioridad* cola = cola_prioridad(proceso_exec->colas_hilos_prioridad_ready,tcb->prioridad);
        queue_push(cola->cola, tcb);
    }


    
}

// THREAD_EXIT, esta syscall finaliza al hilo que lo invocó, pasando el mismo al estado EXIT. Se deberá indicar a la Memoria la finalización de dicho hilo.

void THREAD_EXIT(){
    move_tcb_to_exit(proceso_exec->cola_hilos_new,proceso_exec->cola_hilos_ready,proceso_exec->lista_hilos_blocked,proceso_exec->cola_hilos_exit,proceso_exec->hilo_exec->tid);
}


void planificador_largo_plazo(char*pseudocodigo,int tam_proceso,int prioridad,code_operacion code){
    switch(code){
        case PEDIDO_MEMORIA_INICIALIZAR_PROCESO:
        new_a_ready();
        break;
        case PEDIDO_MEMORIA_TERMINAR_PROCESO:
        PROCESS_EXIT();
        new_a_ready();
        break;
        case PEDIDO_MEMORIA_THREAD_CREATE:
        THREAD_CREATE(pseudocodigo,prioridad);

        if (strcmp(config_get_string_value(config, "ALGORITMO_PLANIFICACION"), "FIFO")==0 || strcmp(config_get_string_value(config, "ALGORITMO_PLANIFICACION"), "PRIORIDADES")==0)
        queue_push(proceso_exec->cola_hilos_ready,proceso_exec->hilo_exec); // Capaz habria que hacer un insertar ordenado para la cola de ready en prioridades...

        if (strcmp(config_get_string_value(config, "ALGORITMO_PLANIFICACION"), "CMN")==0)
        queue_push(list_get(proceso_exec->colas_hilos_prioridad_ready,prioridad),proceso_exec->hilo_exec);
        
        break;
        case PEDIDO_MEMORIA_THREAD_EXIT:
        THREAD_EXIT();
        for (int i = 0; i < list_size(proceso_exec->lista_hilos_blocked); i++){
            queue_push(proceso_exec->cola_hilos_ready,list_get(proceso_exec->lista_hilos_blocked,i));
        }
        break;
        default:
        break;
    }

}



/* Faltaría la siguiente implementación:
En caso que el algoritmo requiera desalojar al hilo en ejecución, se enviará una interrupción a través de la conexión de interrupt para forzar el desalojo del mismo.*/
void planificador_corto_plazo(){

    if (strcmp(config_get_string_value(config, "ALGORITMO_PLANIFICACION"), "FIFO") == 0){
        proceso_exec->hilo_exec = fifo_tcb(proceso_exec); 
        ejecucion(proceso_exec->hilo_exec,proceso_exec->cola_hilos_ready,sockets->sockets_cliente_cpu->socket_Dispatch);

    }
    if (strcmp(config_get_string_value(config,"ALGORITMO_PLANIFICACION"),"PRIORIDADES")==0){

        proceso_exec->hilo_exec=prioridades(proceso_exec);
        ejecucion(proceso_exec->hilo_exec,proceso_exec->cola_hilos_ready,sockets->sockets_cliente_cpu->socket_Dispatch);

    }

    if (strcmp(config_get_string_value(config,"ALGORITMO_PLANIFICACION"),"CMN")==0){
        colas_multinivel(proceso_exec,0); 
    }


}

void ejecucion(t_tcb*hilo,t_queue*queue,int socket_dispatch){
    t_paquete*paquete = crear_paquete();

    hilo->estado=TCB_EXECUTE; // Una vez seleccionado el siguiente hilo a ejecutar, se lo transicionará al estado EXEC



    agregar_a_paquete(paquete,&hilo->tid,sizeof(hilo->tid));
    agregar_a_paquete(paquete,&hilo->pid,sizeof(hilo->pid));

    int rtaCPU =-1;

// Se enviará al módulo CPU el TID y su PID asociado a ejecutar a través del puerto de dispatch, quedando a la espera de recibir dicho TID después de la ejecución junto con un motivo por el cual fue devuelto.
    enviar_paquete(paquete,socket_dispatch); 
    recv(socket_dispatch,&rtaCPU,sizeof(rtaCPU),0);

    /*Agregué algunos de los códigos de operación subidos al módulo CPU. Había conflicto con algunos nombres por llamarse igual a algunas funciones que tenemos
    hechas, así que no agregué todos*/

    // en caso de que el motivo de devolución implique replanificar, se seleccionará el siguiente hilo a ejecutar según indique el algoritmo. Durante este período la CPU se quedará esperando.

    if (rtaCPU == INTERRUPCION || rtaCPU == INTERRUPCION_USUARIO || rtaCPU == ERROR || rtaCPU == LLAMADA_POR_INSTRUCCION){ // 
        hilo->estado=TCB_READY;
        
        // Antes de llamar al siguiente hilo, debo meter al actual en la cola de ready que le corresponda
        queue_push(queue,hilo);
        planificador_corto_plazo();
    }

    if (rtaCPU == THREAD_EXIT_){
        THREAD_EXIT();
    }

}

void insertar_ordenado(t_queue cola, t_tcb nuevo_hilo) {
    if (queue_is_empty(cola)) {
        queue_push(cola, nuevo_hilo);
        return;
    }

    t_queue *cola_temporal = queue_create();
    int inserted = 0;

    for (int i = 0; i < cola->size; i++) {
        t_tcb *hilo_actual = queue_peek(cola);

        if (hilo_actual->prioridad > nuevo_hilo->prioridad) {
            // Si la prioridad del hilo actual es mayor que la prioridad del nuevo hilo,
            // insertamos el nuevo hilo antes del hilo actual
            queue_push(cola_temporal, nuevo_hilo);
            inserted = 1;
        }

        queue_push(cola_temporal, hilo_actual);
        queue_pop(cola);
    }

    if (!inserted) {
        queue_push(cola_temporal, nuevo_hilo);
    }

    while (!queue_is_empty(cola_temporal)) {
        queue_push(cola, queue_pop(cola_temporal));
    }

    queue_destroy(cola_temporal);
}
