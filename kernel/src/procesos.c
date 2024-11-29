#include "includes/procesos.h"
#include "includes/funcionesAuxiliares.h"
#include "includes/planificacion.h"

t_aviso_cpu *aviso_cpu;

pthread_mutex_t mutex_desalojo;

sem_t semaforo_new_ready_procesos;
sem_t semaforo_cola_new_procesos;
sem_t semaforo_cola_exit_procesos;
sem_t sem_ciclo_nuevo;

sem_t semaforo_cola_ready;

sem_t sem_desalojado;

sem_t sem_cola_IO;

sem_t semaforo_cola_exit_hilos;
sem_t sem_lista_prioridades;

sem_t sem_fin_kernel;

sem_t semaforo_cola_exit_hilos_process_exit;
sem_t semaforo_cola_exit_hilo_exec_process_exit;
sem_t sem_fin_syscall;
sem_t sem_espera_interrupcion_cpu;
sem_t sem_ok_desalojo_cpu;
sem_t sem_termina_cmn;
sem_t sem_seguir_o_frenar;
sem_t sem_seguir;
sem_t sem_modulo_terminado;
sem_t sem_termina_hilo;


t_queue *cola_new_procesos;
t_queue *cola_exit_procesos;

t_queue *cola_IO;

t_queue *cola_ready_fifo;
t_list *lista_ready_prioridad;
t_list *colas_ready_prioridad;
t_list *lista_bloqueados;
t_tcb *hilo_exec;
t_queue *cola_exit;

t_list *lista_tcbs;
t_list *lista_pcbs;
t_list *lista_mutex;

t_config *config;
t_log *logger;
sockets_kernel *sockets;

bool esperando;

pthread_mutex_t mutex_log;
pthread_mutex_t mutex_lista_pcbs;
pthread_mutex_t mutex_lista_tcbs;
pthread_mutex_t mutex_cola_new_procesos;
pthread_mutex_t mutex_cola_exit_procesos;
pthread_mutex_t mutex_cola_exit_hilos;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_conexion_kernel_a_dispatch;
pthread_mutex_t mutex_conexion_kernel_a_interrupt;
pthread_mutex_t mutex_syscall_ejecutando;

bool desalojado = false;

bool syscallEjecutando = false;


t_pcb *crear_pcb()
{
    static int pid = 0;
    t_pcb *pcb = malloc(sizeof(t_pcb));
    t_list *lista_tids = list_create();
    t_list *lista_mutex = list_create();
    pcb->pid = pid;
    pcb->tids = lista_tids;
    pcb->lista_mutex = lista_mutex;
    sem_init(&pcb->sem_hilos_eliminar,0,0);
    pid += 1;
    pcb->contador_tid = 0;
    pcb->contador_mutex = 0;
    pthread_mutex_lock(&mutex_lista_pcbs);
    list_add(lista_pcbs, pcb);
    log_info(logger,"pids en crear_pcb, lista_pcbs");
    for (int i = 0; i<list_size(lista_pcbs);i++){
        t_pcb*act=list_get(lista_pcbs,i);
        log_info(logger,"pid guardado: %d",act->pid);
    }

    pthread_mutex_unlock(&mutex_lista_pcbs);

    

    pthread_mutex_init(&pcb->mutex_lista_mutex, NULL);
    pthread_mutex_init(&pcb->mutex_tids, NULL);
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
    list_add(lista_tcbs, tcb);
    if (tcb->tid == 0)
    {
        tcb->prioridad = 0;
    }
    tcb->cola_hilos_bloqueados = queue_create();
    pthread_mutex_init(&tcb->mutex_cola_hilos_bloqueados, NULL);
    pcb->contador_tid += 1;
    return tcb;
}

void iniciar_kernel(char *archivo_pseudocodigo, int tamanio_proceso)
{
    t_pcb *pcb = crear_pcb();
    t_tcb *tcb = crear_tcb(pcb);
    tcb->pseudocodigo = malloc(strlen(archivo_pseudocodigo) + 1);
    strcpy(tcb->pseudocodigo, archivo_pseudocodigo);
    pcb->tamanio_proceso = tamanio_proceso;
    tcb->prioridad = 0;
    pcb->tcb_main = tcb;
    pcb->estado = PCB_NEW;
    pthread_mutex_lock(&mutex_log);
    log_info(logger, "## (%d:0) Se crea el proceso - Estado: NEW", pcb->pid);
    pthread_mutex_unlock(&mutex_log);
    pthread_mutex_lock(&mutex_cola_new_procesos);
    queue_push(cola_new_procesos, pcb);
    pthread_mutex_unlock(&mutex_cola_new_procesos);

    sem_post(&semaforo_cola_new_procesos);

    planificador_largo_plazo();
    planificador_corto_plazo();
    dispositivo_IO();

    // no se en que momento termina de ejecutar kernel
}

void proceso_exit()
{ // elimina los procesos que estan en la cola exit

    sem_wait(&semaforo_cola_exit_procesos); // espera que haya elementos en la cola
    if (estado_kernel == 0){
        return;
    }
    pthread_mutex_lock(&mutex_cola_exit_procesos);
    t_pcb *proceso = queue_pop(cola_exit_procesos);
    pthread_mutex_unlock(&mutex_cola_exit_procesos);

    int pid = proceso->pid;
    liberar_proceso(proceso);

    int respuesta;
    code_operacion cod_op = PROCESS_EXIT_AVISO;

    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    send_operacion_pid(cod_op, pid, socket_memoria);
    recv(socket_memoria, &respuesta, sizeof(int), 0);
    close(socket_memoria);
    if (respuesta != OK)
    {
        printf("Error en la eliminación del proceso en memoria");
    }
    else if (respuesta == OK)
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "## Finaliza el proceso %d", pid);
        pthread_mutex_unlock(&mutex_log);
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
    if (estado_kernel == 0){
        return;
    }

    pthread_mutex_lock(&mutex_cola_exit_hilos);
    t_tcb *hilo = queue_pop(cola_exit);
    printf("tid:%d\n", hilo->tid);
    pthread_mutex_unlock(&mutex_cola_exit_hilos);

    pthread_mutex_lock(&hilo->mutex_cola_hilos_bloqueados);
    while (!queue_is_empty(hilo->cola_hilos_bloqueados))
    {

        t_tcb *tcb = queue_pop(hilo->cola_hilos_bloqueados);

        pthread_mutex_lock(&mutex_log);
        log_info(logger, "TID del tcb bloqueado por el hilo %d: %d", hilo->tid, tcb->tid);
        pthread_mutex_unlock(&mutex_log);

        pthread_mutex_lock(&mutex_lista_blocked);
        list_remove_element(lista_bloqueados, tcb);
        pthread_mutex_unlock(&mutex_lista_blocked);

        if (!hilo_esta_en_cola(cola_exit, tcb->tid, tcb->pid))
        {
            tcb->estado = TCB_READY;

            pushear_cola_ready(tcb);
        }
    }
    pthread_mutex_unlock(&hilo->mutex_cola_hilos_bloqueados);

    pthread_mutex_lock(&mutex_log);
    log_info(logger, "## (%d:%d) Finaliza el hilo", hilo->pid, hilo->tid);
    pthread_mutex_unlock(&mutex_log);

    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    int respuesta;
    send_operacion_tid_pid(THREAD_ELIMINATE_AVISO, hilo->tid, hilo->pid, socket_memoria);
    recv(socket_memoria, &respuesta, sizeof(int), 0);
    close(socket_memoria);
    if (respuesta == -1)
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "Error en la liberacion de memoria del hilo");
        pthread_mutex_unlock(&mutex_log);
    }

    pthread_mutex_lock(&mutex_lista_tcbs);
    list_remove_element(lista_tcbs, hilo);
    pthread_mutex_unlock(&mutex_lista_tcbs);


    int pid = hilo->pid;
    liberar_tcb(hilo);

    
    pthread_mutex_lock(&mutex_lista_pcbs);
    t_pcb* proceso = buscar_pcb_por_pid(lista_pcbs,pid);
    pthread_mutex_unlock(&mutex_lista_pcbs);
    log_info(logger,"Proceso:%d Tids:%d",proceso->pid,proceso->contador_tid);
    sem_post(&proceso->sem_hilos_eliminar);

}

void hilo_exec_exit_tras_process_exit() // El hilo que estaba en exec y ahora esta en cola exit se lo elimina, al igual que a los hilos bloqueados por el mismo
{
    // sem_wait(&semaforo_cola_exit_hilo_exec_process_exit);

    // pthread_mutex_lock(&mutex_cola_exit_hilos);
    t_tcb *hilo = queue_pop(cola_exit);
    printf("tid:%d\n", hilo->tid);
    // pthread_mutex_unlock(&mutex_cola_exit_hilos);

    pthread_mutex_lock(&hilo->mutex_cola_hilos_bloqueados);
    while (!queue_is_empty(hilo->cola_hilos_bloqueados))
    {
        t_tcb *tcb = queue_pop(hilo->cola_hilos_bloqueados);
        tcb->estado = TCB_EXIT;

        pthread_mutex_lock(&mutex_lista_blocked);
        buscar_y_eliminar_tcb(lista_bloqueados, tcb);
        pthread_mutex_unlock(&mutex_lista_blocked);
    }

    pthread_mutex_unlock(&hilo->mutex_cola_hilos_bloqueados);
    pthread_mutex_lock(&mutex_log);
    log_info(logger, "## (%d:%d) Finaliza el hilo", hilo->pid, hilo->tid);
    pthread_mutex_unlock(&mutex_log);
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
    if (estado_kernel == 0){
        return;
    }

    pthread_mutex_lock(&mutex_cola_new_procesos);
    t_pcb *pcb = queue_peek(cola_new_procesos);
    pthread_mutex_unlock(&mutex_cola_new_procesos);

    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    send_inicializacion_proceso(pcb->pid, pcb->tcb_main->pseudocodigo, pcb->tamanio_proceso, socket_memoria);
    recv(socket_memoria, &respuesta, sizeof(int), 0);

    close(socket_memoria);
    if (respuesta == -1)
    {
        sem_wait(&semaforo_new_ready_procesos); // espera a que se libere un proceso
    }
    else
    {
        pthread_mutex_lock(&mutex_cola_new_procesos);
        pcb = queue_pop(cola_new_procesos);
        pthread_mutex_unlock(&mutex_cola_new_procesos);
        pcb->estado = PCB_READY;

        int socket_memoria = cliente_Memoria_Kernel(logger, config);
        int resultado;

        send_inicializacion_hilo(pcb->tcb_main->tid, pcb->pid, pcb->tcb_main->pseudocodigo, socket_memoria);
        recv(socket_memoria, &resultado, sizeof(int), 0);
        close(socket_memoria);

        if (resultado == -1)
        {
            pthread_mutex_lock(&mutex_log);
            log_info(logger, "Error en la creacion del hilo");
            pthread_mutex_unlock(&mutex_log);
            return;
        }
        else
        {
            pcb->tcb_main->estado = TCB_READY;
            pthread_mutex_lock(&mutex_log);
            log_info(logger, "## (%d:%d) Se crea el Hilo - Estado: READY", pcb->pid, pcb->tcb_main->tid);
            pthread_mutex_unlock(&mutex_log);
            pushear_cola_ready(pcb->tcb_main);
        }
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
    tcb->pseudocodigo = malloc(strlen(pseudocodigo) + 1);
    strcpy(tcb->pseudocodigo, pseudocodigo);
    pcb->tamanio_proceso = tamanio_proceso;
    tcb->prioridad = prioridad;
    pcb->tcb_main = tcb;
    pcb->estado = PCB_NEW;
    pthread_mutex_lock(&mutex_log);
    log_info(logger, "## (%d:0) Se crea el proceso - Estado: NEW", pcb->pid);
    pthread_mutex_unlock(&mutex_log);

    pthread_mutex_lock(&mutex_cola_new_procesos);
    queue_push(cola_new_procesos, pcb);
    pthread_mutex_unlock(&mutex_cola_new_procesos);

    sem_post(&semaforo_cola_new_procesos);

    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
}

/*
PROCESS_EXIT, esta syscall finalizará el PCB correspondiente al TCB que ejecutó la instrucción,
enviando todos sus TCBs asociados a la cola de EXIT. Esta instrucción sólo será llamada por el TID 0 del proceso
y le deberá indicar a la memoria la finalización de dicho proceso.
*/

void PROCESS_EXIT()
{
    log_info(logger, "ENTRAMOS A LA SYSCALL PROCESS_EXIT");
    if (hilo_exec->tid != 0)
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "Error, se intento ejecutar la syscall PROCESS_EXIT con un TID que no era el TID 0");
        pthread_mutex_unlock(&mutex_log);
        pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
        send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
        pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
        return;
    }
    pthread_mutex_lock(&mutex_lista_pcbs);
    t_pcb *pcb = buscar_pcb_por_pid(lista_pcbs, hilo_exec->pid);
    pthread_mutex_unlock(&mutex_lista_pcbs);

    log_info(logger,"PID PCB EN PROCESS EXIT: %d",pcb->pid);
    pcb->estado = PCB_EXIT;

    pthread_mutex_lock(&mutex_cola_exit_procesos);
    queue_push(cola_exit_procesos, pcb);
    pthread_mutex_unlock(&mutex_cola_exit_procesos);

    sem_post(&semaforo_cola_exit_procesos);

    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    pthread_mutex_lock(&mutex_desalojo);
    if (!aviso_cpu->finQuantum)
    {
        send_code_operacion(DESALOJAR, sockets->sockets_cliente_cpu->socket_Interrupt);
        aviso_cpu->desalojar = true;
        log_info(logger, "envio desalojar");
        code_operacion codigo = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Dispatch); // confirmar que cpu recibio la interrupción antes de continuar
        if (codigo != OK)
        {
            log_info(logger, "CPU no proceso la interrupción correctamente");
        }
        log_info(logger, "recibi la confirmacion");
    }
    else if (aviso_cpu->finQuantum)
    {
        aviso_cpu->finQuantum = false;
    }
    pthread_mutex_unlock(&mutex_desalojo);
    log_info(logger, "le mando a cpu el OK nashei");
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);

    //fin_syscall_desalojo_cmn();
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

    pthread_mutex_lock(&mutex_lista_pcbs);
    t_pcb *pcb = buscar_pcb_por_pid(lista_pcbs, hilo_exec->pid);
    pthread_mutex_unlock(&mutex_lista_pcbs);

    // Creo la estructura del tcb
    t_tcb *tcb = crear_tcb(pcb);
    tcb->prioridad = prioridad;
    tcb->estado = TCB_READY;

    tcb->pseudocodigo = malloc(strlen(pseudocodigo) + 1);
    strcpy(tcb->pseudocodigo, pseudocodigo);
    // Solicito a memoria que se inicialice el hilo
    int socket_memoria = cliente_Memoria_Kernel(logger, config);
    send_inicializacion_hilo(tcb->tid, tcb->pid, pseudocodigo, socket_memoria);
    recv(socket_memoria, &resultado, sizeof(int), 0);
    close(socket_memoria);

    // Si la memoria no pudo inicializar el hilo
    if (resultado == -1)
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "Error en la creacion del hilo");
        pthread_mutex_unlock(&mutex_log);
        liberar_tcb(tcb);
    }
    else
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "## (%d:%d) Se crea el Hilo - Estado: READY", pcb->pid, tcb->tid);
        pthread_mutex_unlock(&mutex_log);
        pushear_cola_ready(tcb);
    }
    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
}

/*
THREAD_JOIN, esta syscall recibe como parámetro un TID, mueve el hilo que la invocó al
estado BLOCK hasta que el TID pasado por parámetro finalice. En caso de que el TID pasado
 por parámetro no exista o ya haya finalizado,
esta syscall no hace nada y el hilo que la invocó continuará su ejecución.*/

void THREAD_JOIN(int tid)
{

    if (buscar_tcb_por_tid(lista_tcbs, tid, hilo_exec) == NULL || buscar_tcb(tid, hilo_exec) == NULL)
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "## (%d:%d) - El hilo pasado por parámetro no existe/ya finalizó", hilo_exec->pid, hilo_exec->tid);
        pthread_mutex_unlock(&mutex_log);
        pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
        send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
        pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
        return;
    }
    t_tcb *tcb_aux = hilo_exec;

    pthread_mutex_lock(&mutex_lista_tcbs); // Por las dudas cuack cuack. En el if de arriba capaz habria q agregar mutex tmb
    t_tcb *hilo_a_bancar = buscar_tcb_por_tid_pid(tid, hilo_exec->pid, lista_tcbs);
    pthread_mutex_unlock(&mutex_lista_tcbs);

    if (hilo_a_bancar == NULL)
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "## (%d:%d) - El hilo pasado por parámetro no existe/ya finalizó", hilo_exec->pid, hilo_exec->tid);
        pthread_mutex_unlock(&mutex_log);
        pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
        send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
        pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
        return;
    }

    // Cuando se accede a algo del pid/tid en ejecucion tambien habria q poner mutex?
    pthread_mutex_lock(&hilo_a_bancar->mutex_cola_hilos_bloqueados);
    queue_push(hilo_a_bancar->cola_hilos_bloqueados, tcb_aux); // Agregar el hilo a bloquear a la cola de bloqueados del hilo en ejecucion
    pthread_mutex_unlock(&hilo_a_bancar->mutex_cola_hilos_bloqueados);

    // hilo_exec = NULL;
    tcb_aux->estado = TCB_BLOCKED;

    pthread_mutex_lock(&mutex_lista_blocked);
    list_add(lista_bloqueados, tcb_aux);
    pthread_mutex_unlock(&mutex_lista_blocked);

    pthread_mutex_lock(&mutex_log);
    log_info(logger, "## (%d:%d) - Bloqueado por: PTHREAD_JOIN", tcb_aux->pid, tcb_aux->tid);
    pthread_mutex_unlock(&mutex_log);

    sacar_tcb_ready(tcb_aux);

    /*t_tcb* tcb_bloqueante = buscar_tcb_por_tid(lista_tcbs, tid,tcb_aux);
    queue_push(tcb_bloqueante->cola_hilos_bloqueados, tcb_aux);*/
    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    pthread_mutex_lock(&mutex_desalojo);
    if (!aviso_cpu->finQuantum)
    {
        send_code_operacion(DESALOJAR, sockets->sockets_cliente_cpu->socket_Interrupt);
        aviso_cpu->desalojar = true;
        code_operacion codigo = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Dispatch); // confirmar que cpu recibio la interrupción antes de continuar
        if (codigo != OK)
        {
            log_info(logger, "CPU no proceso la interrupción correctamente");
        }
    }
    else if (aviso_cpu->finQuantum)
    {
        aviso_cpu->finQuantum = false;
    }
    pthread_mutex_unlock(&mutex_desalojo);
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);

    //fin_syscall_desalojo_cmn();
}

/*
THREAD_CANCEL, esta syscall recibe como parámetro un TID con el objetivo de finalizarlo pasando al mismo al estado EXIT.
Se deberá indicar a la Memoria la finalización de dicho hilo. En caso de que el TID pasado por parámetro no exista o
ya haya finalizado, esta syscall no hace nada. Finalmente, el hilo que la invocó continuará su ejecución.
*/

void THREAD_CANCEL(int tid)
{ // suponiendo que el proceso main esta ejecutando

    log_info(logger, "Llega thread cancel para hilo %d", tid);

    t_tcb *tcb = buscar_tcb_por_tid(lista_tcbs, tid, hilo_exec); // Debido a que solamente hilos vinculados por un mismo proceso se pueden cancelar entre si, el tid a cancelar debe ser del proceso del hilo que llamo a la funcion

    if (tcb == NULL || buscar_tcb(tid, hilo_exec) == NULL)
    {
        log_info(logger, "El tid %d pasado por parámetro no pertenece al proceso en ejecución/no existe/ya finalizó", tid);
        pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
        send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
        pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);

        return;
    }

    if (tcb->estado == TCB_READY)
    {
        sacar_tcb_ready(tcb);
    }
    else
    {
        pthread_mutex_lock(&mutex_lista_blocked);
        sacar_tcb_de_lista(lista_bloqueados, tcb);
        pthread_mutex_unlock(&mutex_lista_blocked);
    }
    tcb->estado = TCB_EXIT;
    pthread_mutex_lock(&mutex_cola_exit_hilos);
    queue_push(cola_exit, tcb);
    pthread_mutex_unlock(&mutex_cola_exit_hilos);
    sem_post(&semaforo_cola_exit_hilos);

    log_info(logger,"MANDO OK");
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
}


/* THREAD_EXIT, esta syscall finaliza al hilo que lo invocó,
pasando el mismo al estado EXIT. Se deberá indicar a la Memoria
la finalización de dicho hilo.
*/

void THREAD_EXIT() // AVISO A MEMORIA
{
    t_tcb *hilo = hilo_exec;
    hilo->estado = TCB_EXIT;

    // Hilo exec lo establezco en NULL despues
    sacar_tcb_ready(hilo);
    log_info(logger,"TID DEL HILO EN THREAD EXIT: %d",hilo->tid);
    pthread_mutex_lock(&mutex_cola_exit_hilos);
    queue_push(cola_exit, hilo);
    pthread_mutex_unlock(&mutex_cola_exit_hilos);

    sem_post(&semaforo_cola_exit_hilos);

    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    pthread_mutex_lock(&mutex_desalojo);
    if (!aviso_cpu->finQuantum)
    {
        log_info(logger,"MANDO DESALOJAR");
        send_code_operacion(DESALOJAR, sockets->sockets_cliente_cpu->socket_Interrupt);
        aviso_cpu->desalojar = true;
        code_operacion codigo = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Dispatch); // confirmar que cpu recibio la interrupción antes de continuar
        if (codigo != OK)
        {
            log_info(logger, "CPU no proceso la interrupción correctamente");
        }
    }
    else if (aviso_cpu->finQuantum)
    {
        aviso_cpu->finQuantum = false;
    }
    pthread_mutex_unlock(&mutex_desalojo);
    log_info(logger,"MANDO OK");
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
    //fin_syscall_desalojo_cmn();
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

void MUTEX_CREATE(char *recurso) // supongo que el recurso es el nombre del mutex
{
    pthread_mutex_lock(&mutex_lista_pcbs);
    t_pcb *proceso_asociado = buscar_pcb_por_pid(lista_pcbs, hilo_exec->pid);
    pthread_mutex_unlock(&mutex_lista_pcbs);
    t_mutex *mutex = malloc(sizeof(t_mutex));
    mutex->estado = UNLOCKED;
    mutex->cola_tcbs = queue_create();
    mutex->nombre = malloc(strlen(recurso) + 1);
    strcpy(mutex->nombre, recurso);

    list_add(lista_mutex, mutex);

    list_add(proceso_asociado->lista_mutex, mutex);

    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
}

void MUTEX_LOCK(char *recurso)
{
    pthread_mutex_lock(&mutex_lista_pcbs);
    t_pcb *proceso_asociado = buscar_pcb_por_pid(lista_pcbs, hilo_exec->pid);
    pthread_mutex_unlock(&mutex_lista_pcbs);
    t_mutex *mutex_asociado = busqueda_mutex(proceso_asociado->lista_mutex, recurso);
    t_tcb *hilo_aux = hilo_exec;
    if (mutex_asociado == NULL)
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "NO SE ENCONTRÓ EL MUTEX %s", recurso);
        pthread_mutex_unlock(&mutex_log);
        sacar_tcb_ready(hilo_aux);
        pthread_mutex_lock(&mutex_cola_exit_hilos);

        if (hilo_aux->tid != 0){
            pthread_mutex_lock(&mutex_cola_exit_hilos);
            queue_push(cola_exit, hilo_aux);
            pthread_mutex_unlock(&mutex_cola_exit_hilos);
            sem_post(&semaforo_cola_exit_hilos);
        }
        else{
            pthread_mutex_lock(&mutex_cola_exit_procesos);
            queue_push(cola_exit_procesos,proceso_asociado);
            pthread_mutex_unlock(&mutex_cola_exit_procesos);
            sem_post(&semaforo_cola_exit_procesos);
        }

            
        

        pthread_mutex_unlock(&mutex_cola_exit_hilos);
        pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
        pthread_mutex_lock(&mutex_desalojo);
        if (!aviso_cpu->finQuantum)
        {
            send_code_operacion(DESALOJAR, sockets->sockets_cliente_cpu->socket_Interrupt);
            aviso_cpu->desalojar = true;
            code_operacion codigo = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Dispatch); // confirmar que cpu recibio la interrupción antes de continuar
            if (codigo != OK)
            {
                log_info(logger, "CPU no proceso la interrupción correctamente");
            }
        }
        else if (aviso_cpu->finQuantum)
        {
            aviso_cpu->finQuantum = false;
        }
        pthread_mutex_unlock(&mutex_desalojo);
        send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
        pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);

        //fin_syscall_desalojo_cmn();
        return;
    }

    if (mutex_asociado->estado == UNLOCKED)
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "SE ENCONTRÓ EL MUTEX %s y está libre, lo tomamos", recurso);
        pthread_mutex_unlock(&mutex_log);
        mutex_asociado->hilo = hilo_aux;
        mutex_asociado->estado = LOCKED;
        pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
        send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
        pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
    }
    else
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "SE ENCONTRÓ EL MUTEX %s y está ocupado, bloqueamos el hilo", recurso);
        pthread_mutex_unlock(&mutex_log);
        hilo_aux->estado = TCB_BLOCKED_MUTEX;
        // hilo_exec = NULL;
        sacar_tcb_ready(hilo_aux);
        pthread_mutex_lock(&mutex_lista_blocked);
        list_add(lista_bloqueados, hilo_aux);
        pthread_mutex_unlock(&mutex_lista_blocked);

        pthread_mutex_lock(&mutex_log);
        log_info(logger, "## (%d:%d) - Bloqueado por: %s", hilo_aux->pid, hilo_aux->tid, recurso);
        pthread_mutex_unlock(&mutex_log);
        queue_push(mutex_asociado->cola_tcbs, hilo_aux);
        pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
        pthread_mutex_lock(&mutex_desalojo);
        if (!aviso_cpu->finQuantum)
        {
            send_code_operacion(DESALOJAR, sockets->sockets_cliente_cpu->socket_Interrupt);
            aviso_cpu->desalojar = true;
            code_operacion codigo = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Dispatch); // confirmar que cpu recibio la interrupción antes de continuar
            if (codigo != OK)
            {
                log_info(logger, "CPU no proceso la interrupción correctamente");
            }
        }
        else if (aviso_cpu->finQuantum)
        {
            aviso_cpu->finQuantum = false;
        }
        pthread_mutex_unlock(&mutex_desalojo);
        send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
        pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
        //fin_syscall_desalojo_cmn();
    }
}

void MUTEX_UNLOCK(char *recurso)
{
    pthread_mutex_lock(&mutex_lista_pcbs);
    t_pcb *proceso_asociado = buscar_pcb_por_pid(lista_pcbs, hilo_exec->pid);
    pthread_mutex_unlock(&mutex_lista_pcbs);
    t_mutex *mutex_asociado = busqueda_mutex(proceso_asociado->lista_mutex, recurso);
    t_tcb *hilo_aux = hilo_exec;
    if (mutex_asociado == NULL)
    {
        if (hilo_aux->tid != 0){
            pthread_mutex_lock(&mutex_cola_exit_hilos);
            queue_push(cola_exit, hilo_aux);
            pthread_mutex_unlock(&mutex_cola_exit_hilos);
            sem_post(&semaforo_cola_exit_hilos);
        }
        else{
            pthread_mutex_lock(&mutex_cola_exit_procesos);
            queue_push(cola_exit_procesos,proceso_asociado);
            pthread_mutex_unlock(&mutex_cola_exit_procesos);
            sem_post(&semaforo_cola_exit_procesos);
        }

        
        pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
        pthread_mutex_lock(&mutex_desalojo);
        if (!aviso_cpu->finQuantum)
        {
            send_code_operacion(DESALOJAR, sockets->sockets_cliente_cpu->socket_Interrupt);
            aviso_cpu->desalojar = true;
            code_operacion codigo = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Dispatch); // confirmar que cpu recibio la interrupción antes de continuar
            if (codigo != OK)
            {
                log_info(logger, "CPU no proceso la interrupción correctamente");
            }
        }
        else if (aviso_cpu->finQuantum)
        {
            aviso_cpu->finQuantum = false;
        }
        pthread_mutex_unlock(&mutex_desalojo);
        send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
        pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
        //fin_syscall_desalojo_cmn();
        return;
    }
    
    if (mutex_asociado->hilo != hilo_exec)
    {
        log_info(logger, "El hilo que realizó la syscall no es el que tiene tomado el recurso");
        pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
        send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
        pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
        return;
    }
    if (!queue_is_empty(mutex_asociado->cola_tcbs))
    {
        mutex_asociado->hilo = queue_pop(mutex_asociado->cola_tcbs);

        pthread_mutex_lock(&mutex_lista_blocked);
        sacar_tcb_de_lista(lista_bloqueados, mutex_asociado->hilo);
        pthread_mutex_unlock(&mutex_lista_blocked);

        mutex_asociado->hilo->estado = TCB_READY;
        pushear_cola_ready(mutex_asociado->hilo);
    }
    else
    {
        mutex_asociado->estado = UNLOCKED;
        mutex_asociado->hilo = NULL;
    }
    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
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

    sacar_tcb_ready(tcb);

    // Cambiar el estado del hilo a BLOCKED
    tcb->estado = TCB_BLOCKED;
    // hilo_exec = NULL;
    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    pthread_mutex_lock(&mutex_desalojo);
    if (!aviso_cpu->finQuantum)
    {
        aviso_cpu->desalojar = true;
        send_code_operacion(DESALOJAR, sockets->sockets_cliente_cpu->socket_Interrupt);
        code_operacion codigo = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Dispatch); // confirmar que cpu recibio la interrupción antes de continuar
        if (codigo != OK)
        {
            log_info(logger, "CPU no proceso la interrupción correctamente");
        }
    }
    else if (aviso_cpu->finQuantum)
    {
        aviso_cpu->finQuantum = false;
    }
    pthread_mutex_unlock(&mutex_desalojo);
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);

    // Agregar el hilo a la lista de hilos bloqueados
    list_add(lista_bloqueados, tcb);
    pthread_mutex_lock(&mutex_log);
    log_info(logger, "## (%d:%d) - Bloqueado por: IO", tcb->pid, tcb->tid);
    pthread_mutex_unlock(&mutex_log);

    t_nodo_cola_IO *nodo_cola_hilo = malloc(sizeof(t_nodo_cola_IO));
    nodo_cola_hilo->hilo = tcb;
    nodo_cola_hilo->milisegundos = milisegundos;
    queue_push(cola_IO, nodo_cola_hilo);
    pthread_mutex_lock(&mutex_log);
    log_info(logger, "PUSHEÉ EL TCB DEL HILO CON TID %d A LA COLA IO", tcb->tid);
    pthread_mutex_unlock(&mutex_log);
    sem_post(&sem_cola_IO);
    //fin_syscall_desalojo_cmn();
}

/* En este apartado solamente se tendrá la instrucción DUMP_MEMORY. Esta syscall le solicita a la memoria,
junto al PID y TID que lo solicitó, que haga un Dump del proceso.
Esta syscall bloqueará al hilo que la invocó hasta que el módulo memoria confirme la finalización de la operación,
en caso de error, el proceso se enviará a EXIT. Caso contrario, el hilo se desbloquea normalmente pasando a READY.
*/

void dispositivo_IO()
{

    int resultado = pthread_create(&hilo_IO, NULL, hilo_dispositivo_IO, NULL);

    if (resultado != 0)
    {
        pthread_mutex_lock(&mutex_log);
        log_error(logger, "Error al crear el hilo del dispositivo IO");
        pthread_mutex_unlock(&mutex_log);
        return;
    }

    
}

void *hilo_dispositivo_IO(void *args)
{

    while (1)
    {

        sem_wait(&sem_cola_IO); // espera que haya elementos en la cola
        if (estado_kernel == 0){
            sem_post(&sem_termina_hilo);
            return NULL;
        }
        log_info(logger, "LLEGÓ SIGNAL SEM COLA IO");
        t_nodo_cola_IO *info = queue_pop(cola_IO);
        log_info(logger, "MILISEGUNDOS IO: %d", info->milisegundos);
        usleep(info->milisegundos * 1000); // operacion IO

        pthread_mutex_lock(&mutex_lista_blocked);
        bool listado = esta_en_lista_blocked(info->hilo);

        if (listado)
        {
            pthread_mutex_lock(&mutex_log);
            log_info(logger, "## (%d:%d) finalizó IO y pasa a READY", info->hilo->pid, info->hilo->tid);
            pthread_mutex_unlock(&mutex_log);
            list_remove_element(lista_bloqueados,info->hilo);
            pthread_mutex_unlock(&mutex_lista_blocked);

            info->hilo->estado = TCB_READY;

            pushear_cola_ready(info->hilo);
        }
        else
        {
            pthread_mutex_unlock(&mutex_lista_blocked);
            log_info(logger, "El hilo no está bloqueado");
        }
        
        free(info);
    }
    return NULL;
}

void DUMP_MEMORY()
{

    t_tcb *tcb = hilo_exec;

    code_operacion cod_op = DUMP_MEMORIA;

    // hilo_exec = NULL;
    tcb->estado = TCB_BLOCKED;
    desalojado = true;
    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    pthread_mutex_lock(&mutex_desalojo);
    if (!aviso_cpu->finQuantum)
    {
        send_code_operacion(DESALOJAR, sockets->sockets_cliente_cpu->socket_Interrupt);
        aviso_cpu->desalojar = true;
        code_operacion codigo = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Dispatch); // confirmar que cpu recibio la interrupción antes de continuar
        if (codigo != OK)
        {
            log_info(logger, "CPU no proceso la interrupción correctamente");
        }
    }
    else if (aviso_cpu->finQuantum)
    {
        aviso_cpu->finQuantum = false;
    }
    pthread_mutex_unlock(&mutex_desalojo);
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);

    list_add(lista_bloqueados, tcb);

    // Conectar con memoria

    int socket_memoria = cliente_Memoria_Kernel(logger, config);

    int pid = tcb->pid;
    int tid = tcb->tid;

    send_operacion_tid_pid(cod_op, tid, pid, socket_memoria);

    log_info(logger,"ENVIO DE OPERACION MEMORIA");
    int rta_memoria;

    recv(socket_memoria, &rta_memoria, sizeof(int), 0);
    close(socket_memoria);

    if (rta_memoria == -1)
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "Error en el dump de memoria ");
        pthread_mutex_unlock(&mutex_log);
        pthread_mutex_lock(&mutex_lista_pcbs);
        t_pcb *pcb = buscar_pcb_por_pid(lista_pcbs, tcb->pid);
        pthread_mutex_unlock(&mutex_lista_pcbs);
        pcb->estado = PCB_EXIT;
        pthread_mutex_lock(&mutex_cola_exit_procesos);
        queue_push(cola_exit_procesos, pcb);
        pthread_mutex_unlock(&mutex_cola_exit_procesos);
    }
    else
    {
        pthread_mutex_lock(&mutex_log);
        log_info(logger, "Dump de memoria exitoso");
        pthread_mutex_unlock(&mutex_log);
        tcb->estado = TCB_READY;
        pushear_cola_ready(tcb);
    }
}

void atender_segmentation_fault(){
    pthread_mutex_lock(&mutex_lista_pcbs);
    t_pcb *pcb = buscar_pcb_por_pid(lista_pcbs, hilo_exec->pid);
    pthread_mutex_unlock(&mutex_lista_pcbs);
    pcb->estado = PCB_EXIT;
    pthread_mutex_lock(&mutex_cola_exit_procesos);
    queue_push(cola_exit_procesos, pcb);
    pthread_mutex_unlock(&mutex_cola_exit_procesos);

    sem_post(&semaforo_cola_exit_procesos);

    pthread_mutex_lock(&mutex_conexion_kernel_a_interrupt);
    pthread_mutex_lock(&mutex_desalojo);
    if (!aviso_cpu->finQuantum)
    {
        send_code_operacion(DESALOJAR, sockets->sockets_cliente_cpu->socket_Interrupt);
        aviso_cpu->desalojar = true;
        log_info(logger, "envio desalojar");
        code_operacion codigo = recibir_code_operacion(sockets->sockets_cliente_cpu->socket_Dispatch); // confirmar que cpu recibio la interrupción antes de continuar
        if (codigo != OK)
        {
            log_info(logger, "CPU no proceso la interrupción correctamente");
        }
        log_info(logger, "recibi la confirmacion");
    }
    else if (aviso_cpu->finQuantum)
    {
        aviso_cpu->finQuantum = false;
    }
    pthread_mutex_unlock(&mutex_desalojo);
    log_info(logger, "le mando a cpu el OK nashei");
    send_code_operacion(OK, sockets->sockets_cliente_cpu->socket_Dispatch);
    pthread_mutex_unlock(&mutex_conexion_kernel_a_interrupt);
}