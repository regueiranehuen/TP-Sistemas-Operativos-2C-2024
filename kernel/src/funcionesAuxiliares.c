#include "includes/funcionesAuxiliares.h"
#include "includes/procesos.h"

t_list* lista_tcbs;
t_queue* cola_exit;
sem_t semaforo_cola_exit_hilos;
sem_t sem_multinivel;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_cola_exit_hilos;

void inicializar_mutex_procesos(t_pcb* pcb) {
    sem_init(&pcb->sem_hilos_exit, 0, 0);
    pthread_mutex_init(&(pcb->mutex_lista_mutex), NULL);
    pthread_mutex_init(&(pcb->mutex_tids), NULL);
}

void destruir_mutex_procesos(t_pcb* pcb) {
    pthread_mutex_destroy(&(pcb->mutex_lista_mutex));
    pthread_mutex_destroy(&(pcb->mutex_tids));
}

void inicializar_mutex_hilo(t_tcb* tcb){
    pthread_mutex_init(&(tcb->mutex_cola_hilos_bloqueados), NULL);
}

void destruir_mutex_hilo(t_tcb* tcb){
    pthread_mutex_destroy(&(tcb->mutex_cola_hilos_bloqueados));
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

enviar_tcbs_a_cola_exit_por_pid(lista_tcbs, cola_exit, pcb->pid);
list_destroy(pcb->tids);
free(pcb);
}

void enviar_tcbs_a_cola_exit_por_pid(t_list* lista_tcbs, t_queue* cola_exit, int pid_buscado) {
    for (int i = 0; i < list_size(lista_tcbs); i++) {
        t_tcb* tcb_actual = list_get(lista_tcbs, i);  // Obtener el TCB de la lista
        if (tcb_actual->pid == pid_buscado) {
            // Remover el TCB de la lista y enviarlo a la cola EXIT
            t_tcb* tcb_a_mover = list_remove(lista_tcbs, i);
            pthread_mutex_lock(&mutex_cola_exit_hilos);
            queue_push(cola_exit, tcb_a_mover);  // Enviar a la cola EXIT
            pthread_mutex_unlock(&mutex_cola_exit_hilos);
            sem_post(&semaforo_cola_exit_hilos);
            i--;  // Ajustar el índice ya que la lista se reduce
        }
    }
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
/*
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
*/

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
/*
// Función principal para mover un TCB a la cola EXIT
void move_tcb_to_exit(t_tcb* tcb, t_queue* cola_new, t_queue* cola_ready_fifo, t_list* lista_ready_prioridades, t_list* colas_ready_prioridades, t_list* lista_blocked) {
    // Intentar encontrar y eliminar el TCB en la cola NEW
    tcb = find_and_remove_tcb_in_queue(cola_new, tcb->tid);

    // Si no lo encuentra, buscar en la cola FIFO
    if (tcb == NULL) {
        tcb = find_and_remove_tcb_in_queue(cola_ready_fifo, tcb->tid);
    }

    // Si no lo encuentra, buscar en la lista de prioridades
    if (tcb == NULL) {
        char *planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

        if (strcmp(planificacion, "PRIORIDADES") == 0) {
            // Buscar en la lista de prioridades
            tcb = find_and_remove_tcb_in_list(lista_ready_prioridades, tcb->tid);
        } else if (strcmp(planificacion, "MULTINIVEL") == 0) {
            // Buscar en la lista de colas por prioridad
            t_cola_prioridad* cola_prioridad = find_prioridad_in_list(colas_ready_prioridades, tcb->prioridad);
            if (cola_prioridad != NULL) {
                tcb = find_and_remove_tcb_in_queue(cola_prioridad->cola, tcb->tid);
            }
        }
    }

    // Si no lo encuentra, buscar en la lista BLOCKED
    if (tcb == NULL) {
        tcb = find_and_remove_tcb_in_list(lista_blocked, tcb->tid);
    }

    // Si lo encuentra, moverlo a la cola EXIT
    if (tcb != NULL) {
        queue_push(pcb->cola_hilos_exit, tcb);  // Asegúrate de pasar la cola de salida también
        tcb->estado = TCB_EXIT;
        printf("TCB con TID %d movido a EXIT\n", tcb->tid);
    } else {
        printf("TCB con TID %d no encontrado\n", tcb->tid);
    }
}
*/
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

t_mutex* busqueda_mutex(t_list* lista_mutex, char* recurso){

int tamanio_lista = list_size(lista_mutex);

for(int i=0; i< tamanio_lista;i++){
t_mutex* mutex_aux = list_get(lista_mutex,i);
if(strcmp(mutex_aux->nombre, recurso)== 0){
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
/*
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
}*/

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

t_cola_prioridad* obtener_cola_con_mayor_prioridad(t_list* colas_hilos_prioridad_ready) {
    if (list_is_empty(colas_hilos_prioridad_ready)) {
        sem_wait(&sem_multinivel)  // Espera hasta que haya elementos en alguna cola
    }

    t_cola_prioridad* cola_con_mayor_prioridad = NULL;

    // Iterar a través de la lista para encontrar la cola con la mayor prioridad y al menos un TCB
    for (int i = 0; i < list_size(colas_hilos_prioridad_ready); i++) {
        t_cola_prioridad* elemento = list_get(colas_hilos_prioridad_ready, i);

        if (!queue_is_empty(elemento->cola)) {
            if (cola_con_mayor_prioridad == NULL || elemento->prioridad < cola_con_mayor_prioridad->prioridad) {
                cola_con_mayor_prioridad = elemento;  // Actualiza la cola con mayor prioridad
            }
        }
    }

    return cola_con_mayor_prioridad;  // Devuelve la cola con la mayor prioridad encontrada o NULL si no hay colas con TCBs
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

void buscar_y_eliminar_tcb(t_list* lista_tcbs, t_tcb* tcb) {
    // Bloquear el mutex para asegurar acceso exclusivo a la lista

    for (int i = 0; i < list_size(lista_tcbs); i++) {
        t_tcb* tcb_actual = list_get(lista_tcbs, i);  // Obtener el TCB en la posición 'i'
        if (tcb_actual->tid == tcb->tid) {
            // Eliminar el TCB encontrado y retornarlo
            pthread_mutex_lock(&mutex_lista_blocked);
            list_remove(lista_tcbs, i);
            pthread_mutex_unlock(&mutex_lista_blocked);
            // Desbloquear el mutex antes de retornar
        }
    }    
}

t_tcb* buscar_tcb_por_tid(t_list* lista_tcbs, int tid_buscado) {
    for (int i = 0; i < list_size(lista_tcbs); i++) {
        t_tcb* tcb_actual = list_get(lista_tcbs, i);  // Obtener el TCB en la posición 'i'
        if (tcb_actual->tid == tid_buscado) {
            return tcb_actual;  // Devolver el TCB encontrado
        }
    }
    // Si no se encuentra, retornar NULL
    return NULL;
}


t_pcb* buscar_pcb_por_pid(t_list* lista_pcbs, int pid_buscado) {
    for (int i = 0; i < list_size(lista_pcbs); i++) {
        t_pcb* pcb_actual = list_get(lista_pcbs, i); // Obtener el PCB de la lista
        if (pcb_actual->pid == pid_buscado) {
            return pcb_actual; // Retorna el PCB si coincide el PID
        }
    }
    return NULL; // Retorna NULL si no encuentra coincidencia
}

t_tcb* sacar_tcb_de_cola(t_queue* cola, t_tcb* tcb_a_sacar) {
    if (cola == NULL || queue_is_empty(cola)) {
        return NULL;  // Retorna NULL si la cola es NULL o está vacía
    }

    t_queue* cola_temporal = queue_create();  // Crear una cola temporal
    t_tcb* tcb_encontrado = NULL;

    // Recorremos la cola original
    while (!queue_is_empty(cola)) {
        t_tcb* tcb_actual = queue_pop(cola);  // Sacamos el primer elemento

        if (tcb_actual == tcb_a_sacar) {
            tcb_encontrado = tcb_actual;  // Encontramos el TCB que queremos sacar
        } else {
            queue_push(cola_temporal, tcb_actual);  // Lo metemos en la cola temporal si no es el TCB que buscamos
        }
    }

    // Restauramos los elementos en la cola original, excepto el TCB eliminado
    while (!queue_is_empty(cola_temporal)) {
        queue_push(cola, queue_pop(cola_temporal));
    }

    queue_destroy(cola_temporal);  // Destruimos la cola temporal

    return tcb_encontrado;  // Devolvemos el TCB sacado, o NULL si no se encontró
}

t_tcb* sacar_tcb_de_lista(t_list* lista, t_tcb* tcb_a_sacar) {
    if (lista == NULL || list_is_empty(lista)) {
        return NULL;  // Retorna NULL si la lista es NULL o está vacía
    }

    t_tcb* tcb_encontrado = NULL;

    // Iterar sobre la lista para buscar el TCB específico
    for (int i = 0; i < list_size(lista); i++) {
        t_tcb* tcb_actual = list_get(lista, i);

        if (tcb_actual == tcb_a_sacar) {
            tcb_encontrado = list_remove(lista, i);  // Elimina el TCB de la lista y lo guarda
            break;  // Salimos del bucle una vez encontrado
        }
    }

    return tcb_encontrado;  // Retorna el TCB extraído, o NULL si no se encontró
}