#include "includes/funcionesAuxiliares.h"
#include "includes/procesos.h"

void inicializar_estados_hilos (t_pcb* pcb){
    pcb -> cola_hilos_new = queue_create();
    pcb -> cola_hilos_ready = queue_create();
    pcb -> lista_prioridad_ready = list_create();
    pcb -> colas_hilos_prioridad_ready = list_create();
    pcb -> lista_hilos_blocked = list_create();
    pcb -> cola_hilos_exit = queue_create();
}

t_pcb* lista_pcb(t_list* lista_pcbs, int pid){


int tamanio = list_size(lista_pcbs);
int i;
for(i=0;i<tamanio;i++){
t_pcb* pcb = list_get(lista_pcbs, i);
if(pcb->pid== pid){
    return pcb;
}
}
return NULL;
}

void liberar_proceso (t_pcb * pcb){

//Mandar todos los hilos de la cola new a la cola exit y destruir la primera

 int tamanio_cola_new = queue_size (pcb->cola_hilos_new);
    
    for(int i=0;i< tamanio_cola_new;i++){
        t_tcb* tcb = queue_pop (pcb->cola_hilos_new);
        queue_push(pcb->cola_hilos_exit,tcb);
    }

queue_destroy(pcb->cola_hilos_new);

//Mandar todos los hilos de la cola ready a la cola exit y destruir la primera 

int tamanio_lista = list_size(pcb->colas_hilos_prioridad_ready);


for(int i=0;i< tamanio_lista;i++){
    t_cola_prioridad* cola = list_get(pcb->colas_hilos_prioridad_ready,i);
    int tamanio_cola = queue_size(cola->cola);

for(int j=0;j< tamanio_cola;j++){
    t_tcb* tcb = queue_pop(cola->cola);
    queue_push(pcb->cola_hilos_exit,tcb);
}
queue_destroy(cola->cola);
free(cola);
}
list_destroy(pcb->colas_hilos_prioridad_ready);

//Mandar todos los hilos de la lista blocked a la cola exit y destruir la primera

int tamanio_lista_blocked = list_size(pcb->lista_hilos_blocked);

for(int i=0;i<tamanio_lista_blocked;i++){
t_tcb* tcb = list_get(pcb->lista_hilos_blocked,i);
queue_push(pcb->cola_hilos_exit,tcb);
}
list_destroy(pcb->lista_hilos_blocked);
if(pcb->hilo_exec->estado==TCB_EXECUTE){
    //mandar interrupcion a cpu
    queue_push(pcb->cola_hilos_exit,pcb->hilo_exec);
}

//Destruir la cola exit, lista de tids y liberar el espacio del pcb

queue_destroy(pcb->cola_hilos_exit);

list_destroy(pcb->tids);

free(pcb);

}

t_cola_prioridad* cola_prioridad(t_list* lista_colas_prioridad, int prioridad){ //Busca la cola con la prioridad del parametro en la lista de colas, si la encuentra devuelve la info de la posición de dicha lista, si no crea una y la devuelve


int tamanio = list_size(lista_colas_prioridad);
int i;
for(i=0;i<tamanio;i++){
t_cola_prioridad* cola = list_get(lista_colas_prioridad, i);
if(cola->prioridad== prioridad){
    return cola;
}
}
t_cola_prioridad* cola = malloc(sizeof(t_cola_prioridad));
cola->cola = queue_create();
cola->prioridad = prioridad;
list_add(lista_colas_prioridad,cola);
return cola;
}

int lista_tcb(t_pcb* pcb, int tid) { 
    // Busca un tid en una lista de tids
    int tamanio = list_size(pcb->tids);

    for (int i = 0; i < tamanio; i++) {
        int* tid_aux = list_get(pcb->tids, i);
        if (*tid_aux == tid) {  // Desreferenciamos el puntero para comparar el valor entero
            return 0;
        }
    }
    return -1;
}

int tid_finalizado(t_pcb* pcb, int tid) {
    // Accedemos directamente a la lista interna de la cola de EXIT
    int tamanio = queue_size(pcb->cola_hilos_exit);
    for (int i = 0; i < tamanio; i++) {
        t_tcb* tcb = list_get(pcb->cola_hilos_exit->elements, i);  // Obtenemos el TCB en la posición i
        if (tcb->tid == tid) {  // Verificamos si el TID coincide
            return -1;  // El TCB con el TID dado ya ha finalizado
        }
    }
    return 0;  // No se encontró el TID en la cola de EXIT
}

// Función auxiliar para buscar y remover un TCB de una cola
t_tcb* find_and_remove_tcb_in_queue(t_queue* queue, int tid) {
    t_tcb* tcb = NULL;

    for (int i = 0; i < queue_size(queue); i++) {
        t_tcb* current_tcb = list_get(queue->elements, i);  // Usamos list_get ya que queue->elements es un t_list
        if (current_tcb->tid == tid) {
            tcb = current_tcb;  // TCB encontrado
            list_remove(queue->elements, i);  // Remover TCB de la cola
            break;
        }
    }

    return tcb;  // Devuelve el TCB encontrado, o NULL si no lo encontró
}

// Función auxiliar para buscar y remover un TCB de una lista
t_tcb* find_and_remove_tcb_in_list(t_list* list, int tid) {
    t_tcb* tcb = NULL;

    for (int i = 0; i < list_size(list); i++) {
        t_tcb* current_tcb = list_get(list, i);
        if (current_tcb->tid == tid) {
            tcb = current_tcb;  // TCB encontrado
            list_remove(list, i);  // Remover TCB de la lista
            break;
        }
    }

    return tcb;  // Devuelve el TCB encontrado, o NULL si no lo encontró
}

// Función principal para mover un TCB a la cola EXIT
void move_tcb_to_exit(t_pcb* pcb, t_tcb* tcb) {
    
    // Buscar en la cola NEW
    tcb = find_and_remove_tcb_in_queue(pcb->cola_hilos_new, tcb->tid);

    // Si no lo encuentra, buscar en la cola READY
    if (tcb == NULL) {
    char *planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
         if (strcmp(planificacion, "FIFO") == 0 || strcmp(planificacion,"PRIORIDADES")==0) 
    {
        tcb = find_and_remove_tcb_in_queue(pcb->cola_hilos_ready, tcb->tid);
    }

    if (strcmp(planificacion, "MULTINIVEL") == 0)
    {
        t_cola_prioridad *cola = cola_prioridad(pcb->colas_hilos_prioridad_ready, tcb->prioridad);
        tcb = find_and_remove_tcb_in_queue(cola->cola, tcb->tid);
    }
    }

    // Si no lo encuentra, buscar en la lista BLOCKED
    if (tcb == NULL) {
        tcb = find_and_remove_tcb_in_list(pcb->lista_hilos_blocked, tcb->tid);
    }

    // Si lo encuentra, moverlo a la cola EXIT
    if (tcb != NULL) {
        queue_push(pcb->cola_hilos_exit, tcb);
        tcb->estado = TCB_EXIT;
        printf("TCB con TID %d movido a EXIT\n", tcb->tid);
    } else {
        printf("TCB con TID %d no encontrado\n", tcb->tid);
    }
}

// Función auxiliar para buscar un TCB en una cola
t_tcb* find_tcb_in_queue(t_queue* queue, int tid) {
    for (int i = 0; i < queue_size(queue); i++) {
        t_tcb* tcb = list_get(queue->elements, i);  // Usamos list_get ya que queue->elements es un t_list
        if (tcb->tid == tid) {
            return tcb;  // TCB encontrado
        }
    }
    return NULL;  // No encontrado
}

// Función auxiliar para buscar un TCB en una lista
t_tcb* find_tcb_in_list(t_list* list, int tid) {
    for (int i = 0; i < list_size(list); i++) {
        t_tcb* tcb = list_get(list, i);
        if (tcb->tid == tid) {
            return tcb;  // TCB encontrado
        }
    }
    return NULL;  // No encontrado
}

t_tcb* buscar_tcb(int tid, t_queue* queue_new, t_list* queue_ready, t_list* list_blocked) {
    t_tcb* tcb = NULL;

    // Buscar en la cola NEW
    tcb = find_tcb_in_queue(queue_new, tid);
    if (tcb != NULL) {
        return tcb;  // Encontrado en NEW
    }

    // Buscar en la cola READY (en queue_ready que es una lista de t_cola_prioridad)
    for (int i = 0; i < list_size(queue_ready); i++) {
        t_cola_prioridad* cola_prioridad = list_get(queue_ready, i);
        tcb = find_tcb_in_queue(cola_prioridad->cola, tid);
        if (tcb != NULL) {
            return tcb;  // Encontrado en READY
        }
    }

    // Buscar en la lista BLOCKED (en list_blocked que es una lista de t_cola_prioridad)
    for (int i = 0; i < list_size(list_blocked); i++) {
        t_cola_prioridad* cola_prioridad = list_get(list_blocked, i);
        tcb = find_tcb_in_queue(cola_prioridad->cola, tid);
        if (tcb != NULL) {
            return tcb;  // Encontrado en BLOCKED
        }
    }

    return NULL;  // No encontrado en ninguna cola/lista
}

t_mutex* busqueda_mutex(t_list* lista_mutex, int mutex_id){

int tamanio_lista = list_size(lista_mutex);

for(int i=0; i< tamanio_lista;i++){
t_mutex* mutex_aux = list_get(lista_mutex,i);
if(mutex_aux->mutex_id == mutex_id){
    return mutex_aux;
}
}
return NULL;
}

int suma_tam_hilos_colas_en_lista(t_list*list){
    int tam_total = 0;
    for (int i = 0; i< list_size(list); i++){
        t_cola_prioridad * cola_prioridad = list_get(list,i);
        tam_total+= queue_size(cola_prioridad->cola) * size_tcbs_queue(cola_prioridad->cola); 
    }
    return tam_total;  
}


int size_tcbs_queue(t_queue* queue) {
    int tam = 0;

    t_queue* queue_aux = queue_create();
    
    // Recorrer la cola original sin modificarla
    while (!queue_is_empty(queue)) {
        t_tcb* tcb = queue_pop(queue);
        
        // Sumar el tamaño del TCB
        tam += tam_tcb(tcb);
        
        // Volver a agregar el TCB a la cola auxiliar
        queue_push(queue_aux, tcb);
    }

    // Restaurar la cola original
    while (!queue_is_empty(queue_aux)) {
        t_tcb* tcb = queue_pop(queue_aux);
        queue_push(queue, tcb);
    }

    queue_destroy(queue_aux);

    return tam;
}



// Función auxiliar para conseguir el tamaño de un TCB
int tam_tcb(t_tcb * tcb){
    return 4*sizeof(int) + sizeof(estado_hilo) + tcb->pseudocodigo_length + size_tcbs_queue(tcb->cola_hilos_bloqueados);
}

void liberar_tcb(t_tcb* tcb) {
    if (tcb != NULL) {
        // Liberar el pseudocódigo si fue asignado
        if (tcb->pseudocodigo != NULL) {
            free(tcb->pseudocodigo);
            tcb->pseudocodigo = NULL;
        }

        // Liberar el propio tcb
        free(tcb);
    }
}

t_tcb* buscar_tcb_por_tid(t_pcb* pcb, int tid) {
    // Primero busca el tcb en la lista de tids para verificar si existe
    for (int i = 0; i < list_size(pcb->tids); i++) {
        t_tcb* tcb = (t_tcb*) list_get(pcb->tids, i);
        if (tcb->tid == tid) {
            // Verifica en qué estado se encuentra el tcb y busca en la cola correcta
            switch (tcb->estado) {
                case TCB_NEW:
                    for (int j = 0; j < queue_size(pcb->cola_hilos_new); j++) {
                        t_tcb* tcb_encontrado = (t_tcb*) queue_peek(pcb->cola_hilos_new); // Asumiendo que queue_peek devuelve el elemento en la posición 0
                        if (tcb_encontrado->tid == tid) {
                            return tcb_encontrado;
                        }
                    }
                    break;

                case TCB_READY:
                    for (int j = 0; j < queue_size(pcb->cola_hilos_ready); j++) {
                        t_tcb* tcb_encontrado = (t_tcb*) queue_peek(pcb->cola_hilos_ready); // Igualmente, esto asume un acceso similar
                        if (tcb_encontrado->tid == tid) {
                            return tcb_encontrado;
                        }
                    }
                    break;

                case TCB_EXECUTE:
                    if (pcb->hilo_exec != NULL && pcb->hilo_exec->tid == tid) {
                        return pcb->hilo_exec;
                    }
                    break;

                case TCB_BLOCKED:
                case TCB_BLOCKED_MUTEX:
                    for (int j = 0; j < list_size(pcb->lista_hilos_blocked); j++) {
                        t_tcb* tcb_encontrado = (t_tcb*) list_get(pcb->lista_hilos_blocked, j);
                        if (tcb_encontrado->tid == tid) {
                            return tcb_encontrado;
                        }
                    }
                    break;

                default:
                    return NULL; // Si el estado no es válido
            }
        }
    }

    // Si no se encontró el tcb con el tid dado
    return NULL;
}

// Función auxiliar para insertar ordenado un hilo en una cola
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


void* ordenar_cola(void*arg){
    t_queue*cola = (t_queue*)arg;

    t_queue *cola_temporal = queue_create();



    for (int i = 0; i < queue_size(cola) && queue_size(cola)>1; i++) {
        t_tcb *hilo_actual = queue_pop(cola);

        t_tcb*hilo_sig=queue_pop(cola);

        if (hilo_actual->prioridad <= hilo_sig->prioridad) {
            // Si la prioridad del hilo actual es mayor que la prioridad del nuevo hilo,
            // insertamos el nuevo hilo antes del hilo actual
            queue_push(cola_temporal, hilo_actual);
            queue_push(cola_temporal,hilo_sig);
        }
        else{
            queue_push(cola_temporal,hilo_sig);
            queue_push(cola_temporal, hilo_actual);
        }

        queue_push(cola_temporal, hilo_actual);
        queue_pop(cola);
    }

    

    while (!queue_is_empty(cola_temporal)) {
        queue_push(cola, queue_pop(cola_temporal));
    }

    queue_destroy(cola_temporal);

    return NULL;

}

bool strings_iguales(char*c1,char*c2){
    return strcmp(c1,c2)==0;
}

bool es_motivo_devolucion(code_operacion motivo_devolucion){
    return motivo_devolucion == INTERRUPCION || motivo_devolucion == INTERRUPCION_USUARIO || motivo_devolucion == ERROR || motivo_devolucion == LLAMADA_POR_INSTRUCCION;
}

int obtener_mayor_prioridad(t_list* lista_cola_prioridad) {
    if (list_is_empty(lista_cola_prioridad)) {
        return -1;  // Retorna -1 si la lista está vacía (puedes usar otro valor indicativo)
    }

    int mayor_prioridad = -1;

    // Iterar a través de la lista para encontrar la mayor prioridad
    for (int i = 0; i < list_size(lista_cola_prioridad); i++) {
        t_cola_prioridad* elemento = list_get(lista_cola_prioridad, i);
        
        if (mayor_prioridad == -1 && !queue_is_empty(elemento->cola)){
            mayor_prioridad = elemento->prioridad;
        }
        else if (elemento->prioridad < mayor_prioridad && !queue_is_empty(elemento->cola)) {
            mayor_prioridad = elemento->prioridad;  // Actualiza la mayor prioridad
        }
    }

    return mayor_prioridad;  // Retorna la mayor prioridad encontrada
}
t_cola_prioridad* obtener_cola_por_prioridad(t_list *colas_hilos_prioridad_ready, int prioridad_buscada)
{
    for (int i = 0; i < list_size(colas_hilos_prioridad_ready); i++)
    {
        t_cola_prioridad *cola_prioridad_i = list_get(colas_hilos_prioridad_ready, i);

        if (cola_prioridad_i != NULL && cola_prioridad_i->prioridad == prioridad_buscada && !queue_is_empty(cola_prioridad_i->cola))
        {
            return cola_prioridad_i; // Devuelve la estructura de la cola con la prioridad buscada
        }
    }

    return NULL; // No se encontró una cola con la prioridad buscada
}

void ordenar_por_prioridad(t_list* lista) {
    if (list_size(lista) <= 1) {
        return; // No hay nada que ordenar
    }

    t_list* lista_ordenada = list_create(); // Crear una nueva lista para almacenar los elementos ordenados

    while (list_size(lista) > 0) {
        // Encontrar el elemento con la prioridad más alta
        t_tcb* min_elem = list_get(lista, 0);
        int min_index = 0;

        for (int i = 1; i < list_size(lista); i++) {
            t_tcb* current_elem = list_get(lista, i);
            if (current_elem->prioridad < min_elem->prioridad) {
                min_elem = current_elem;
                min_index = i;
            }
        }

        // Eliminar el elemento de la lista original
        list_remove(lista, min_index);
        // Agregarlo a la lista ordenada
        list_add(lista_ordenada, min_elem);
    }

    // Reemplazar la lista original con la lista ordenada
    // Para esto, eliminamos los elementos de la lista original uno por uno
    while (list_size(lista_ordenada) > 0) {
        t_tcb* elem = list_get(lista_ordenada, 0); // Obtenemos el primer elemento
        list_add(lista, elem); // Agregar a la lista original
        list_remove(lista_ordenada, 0); // Eliminarlo de la lista ordenada
    }

    list_destroy(lista_ordenada); // Limpiar la lista ordenada si ya no es necesaria
}