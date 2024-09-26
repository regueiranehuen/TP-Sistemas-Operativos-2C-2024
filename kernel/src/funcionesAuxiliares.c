#include "includes/funcionesAuxiliares.h"
#include "includes/procesos.h"

void inicializar_estados_hilos(t_pcb *pcb)
{
    pcb->cola_hilos_new = queue_create();
    pcb->colas_hilos_prioridad_ready = list_create();
    pcb->lista_hilos_blocked = list_create();
    pcb->cola_hilos_exit = queue_create();
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
    if (pcb->hilo_exec->estado == TCB_EXECUTE)
    {
        // mandar interrupcion a cpu
        queue_push(pcb->cola_hilos_exit, pcb->hilo_exec);
    }

    // Destruir la cola exit, lista de tids y liberar el espacio del pcb

    queue_destroy(pcb->cola_hilos_exit);

    list_destroy(pcb->tids);

    free(pcb);
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

int lista_tcb(t_pcb *pcb, int tid)
{
    // Busca un tid en una lista de tids
    int tamanio = list_size(pcb->tids);

    for (int i = 0; i < tamanio; i++)
    {
        int *tid_aux = list_get(pcb->tids, i);
        if (*tid_aux == tid)
        { // Desreferenciamos el puntero para comparar el valor entero
            return 0;
        }
    }
    return -1;
}

int tid_finalizado(t_pcb *pcb, int tid)
{
    // Accedemos directamente a la lista interna de la cola de EXIT
    int tamanio = queue_size(pcb->cola_hilos_exit);
    for (int i = 0; i < tamanio; i++)
    {
        t_tcb *tcb = list_get(pcb->cola_hilos_exit->elements, i); // Obtenemos el TCB en la posición i
        if (tcb->tid == tid)
        {              // Verificamos si el TID coincide
            return -1; // El TCB con el TID dado ya ha finalizado
        }
    }
    return 0; // No se encontró el TID en la cola de EXIT
}

// Función auxiliar para buscar y remover un TCB de una cola
t_tcb *find_and_remove_tcb_in_queue(t_queue *queue, int tid)
{
    t_tcb *tcb = NULL;

    for (int i = 0; i < queue_size(queue); i++)
    {
        t_tcb *current_tcb = list_get(queue->elements, i); // Usamos list_get ya que queue->elements es un t_list
        if (current_tcb->tid == tid)
        {
            tcb = current_tcb;               // TCB encontrado
            list_remove(queue->elements, i); // Remover TCB de la cola
            break;
        }
    }

    return tcb; // Devuelve el TCB encontrado, o NULL si no lo encontró
}

// Función auxiliar para buscar y remover un TCB de una lista
t_tcb *find_and_remove_tcb_in_list(t_list *list, int tid)
{
    t_tcb *tcb = NULL;

    for (int i = 0; i < list_size(list); i++)
    {
        t_tcb *current_tcb = list_get(list, i);
        if (current_tcb->tid == tid)
        {
            tcb = current_tcb;    // TCB encontrado
            list_remove(list, i); // Remover TCB de la lista
            break;
        }
    }

    return tcb; // Devuelve el TCB encontrado, o NULL si no lo encontró
}

// Función principal para mover un TCB a la cola EXIT
void move_tcb_to_exit(t_queue *queue_new, t_queue *queue_ready, t_list *list_blocked, t_queue *queue_exit, int tid)
{
    t_tcb *tcb = NULL;

    // Buscar en la cola NEW
    tcb = find_and_remove_tcb_in_queue(queue_new, tid);

    // Si no lo encuentra, buscar en la cola READY
    if (tcb == NULL)
    {
        tcb = find_and_remove_tcb_in_queue(queue_ready, tid);
    }

    // Si no lo encuentra, buscar en la lista BLOCKED
    if (tcb == NULL)
    {
        tcb = find_and_remove_tcb_in_list(list_blocked, tid);
    }

    // Si lo encuentra, moverlo a la cola EXIT
    if (tcb != NULL)
    {
        queue_push(queue_exit, tcb);
        printf("TCB con TID %d movido a EXIT\n", tid);
    }
    else
    {
        printf("TCB con TID %d no encontrado\n", tid);
    }
}

// Función auxiliar para buscar un TCB en una cola
t_tcb *find_tcb_in_queue(t_queue *queue, int tid)
{
    for (int i = 0; i < queue_size(queue); i++)
    {
        t_tcb *tcb = list_get(queue->elements, i); // Usamos list_get ya que queue->elements es un t_list
        if (tcb->tid == tid)
        {
            return tcb; // TCB encontrado
        }
    }
    return NULL; // No encontrado
}

// Función auxiliar para buscar un TCB en una lista
t_tcb *find_tcb_in_list(t_list *list, int tid)
{
    for (int i = 0; i < list_size(list); i++)
    {
        t_tcb *tcb = list_get(list, i);
        if (tcb->tid == tid)
        {
            return tcb; // TCB encontrado
        }
    }
    return NULL; // No encontrado
}

t_tcb *buscar_tcb(int tid, t_queue *queue_new, t_list *queue_ready, t_list *list_blocked)
{
    t_tcb *tcb = NULL;

    // Buscar en la cola NEW
    tcb = find_tcb_in_queue(queue_new, tid);
    if (tcb != NULL)
    {
        return tcb; // Encontrado en NEW
    }

    // Buscar en la cola READY (en queue_ready que es una lista de t_cola_prioridad)
    for (int i = 0; i < list_size(queue_ready); i++)
    {
        t_cola_prioridad *cola_prioridad = list_get(queue_ready, i);
        tcb = find_tcb_in_queue(cola_prioridad->cola, tid);
        if (tcb != NULL)
        {
            return tcb; // Encontrado en READY
        }
    }

    // Buscar en la lista BLOCKED (en list_blocked que es una lista de t_cola_prioridad)
    for (int i = 0; i < list_size(list_blocked); i++)
    {
        t_cola_prioridad *cola_prioridad = list_get(list_blocked, i);
        tcb = find_tcb_in_queue(cola_prioridad->cola, tid);
        if (tcb != NULL)
        {
            return tcb; // Encontrado en BLOCKED
        }
    }

    return NULL; // No encontrado en ninguna cola/lista
}

t_mutex *busqueda_mutex(t_list *lista_mutex, int mutex_id)
{

    int tamanio_lista = list_size(lista_mutex);

    for (int i = 0; i < tamanio_lista; i++)
    {
        t_mutex *mutex_aux = list_get(lista_mutex, i);
        if (mutex_aux->mutex_id == mutex_id)
        {
            return mutex_aux;
        }
    }
    return NULL;
}

// Función auxiliar para obtener el tamaño de la suma de los hilos que se encuentran en colas que a su vez estan en una lista
int suma_tam_hilos_colas_en_lista(t_list*list){
    int tam_total = 0;
    for (int i = 0; i< list_size(list); i++){
        t_cola_prioridad * cola_prioridad = list_get(list,i);
        tam_total+= queue_size(cola_prioridad->cola) * size_tcbs_queue(cola_prioridad->cola); 
    }
    return tam_total;  
}


// Funcion auxiliar para calcular el tamaño de una cola con tcb y prioridad
int size_tcbs_queue(t_queue*queue){
    int tam = 0;

    t_queue*queue_aux = queue_create();

    *queue_aux=*queue;
    
    while(!queue_is_empty(queue_aux)){
        t_tcb*tcb=queue_pop(queue_aux);
        tam+=tam_tcb(tcb);
        tam+=sizeof(int); // Por la prioridad del tcb
    }
    queue_destroy(queue_aux);
    
    return tam;
}

// Función auxiliar para conseguir el tamaño de un PCB
int tam_pcb(t_pcb*pcb){
    return 4*sizeof(int) + list_size(pcb->tids)*sizeof(int) + suma_tam_hilos_colas_en_lista(pcb->colas_hilos_prioridad_ready) + list_size(pcb->lista_hilos_blocked)*sizeof(pthread_t) + queue_size(pcb->cola_hilos_new)*sizeof(pthread_t)+queue_size(pcb->cola_hilos_exit)*sizeof(pthread_t)+queue_size(pcb->cola_hilos_ready)*sizeof(pthread_t)+tam_tcb(pcb->hilo_exec)+list_size(pcb->lista_mutex)*sizeof(int);
}



// Función auxiliar para conseguir el tamaño de un TCB
int tam_tcb(t_tcb * tcb){
    return 5*sizeof(int) + tcb->pseudocodigo_length;
}


// Función auxiliar para insertar ordenado en una cola
void insertar_ordenado(t_queue*cola, t_tcb* nuevo_hilo){
    if (queue_is_empty(cola)) {
        queue_push(cola, nuevo_hilo);
        return;
    }

    t_queue *cola_temporal = queue_create();
    bool inserted = false;

    for (int i = 0; i < queue_size(cola); i++) {
        t_tcb *hilo_actual = queue_peek(cola);

        if (hilo_actual->prioridad > nuevo_hilo->prioridad) {
            // Si la prioridad del hilo actual es mayor que la prioridad del nuevo hilo,
            // insertamos el nuevo hilo antes del hilo actual
            queue_push(cola_temporal, nuevo_hilo);
            inserted = true;
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

bool strings_iguales(char*c1,char*c2){
    return strcmp(c1,c2)==0;
}
