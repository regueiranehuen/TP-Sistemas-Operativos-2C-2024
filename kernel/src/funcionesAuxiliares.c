#include "includes/funcionesAuxiliares.h"


void inicializar_estados() {
    // Inicialización de colas
    cola_new_procesos = queue_create();
    cola_ready_fifo = queue_create();
    cola_exit = queue_create();
    cola_IO = queue_create();
    cola_exit_procesos = queue_create();

    // Inicialización de listas
    lista_ready_prioridad = list_create();
    colas_ready_prioridad = list_create();
    lista_bloqueados = list_create();
    lista_tcbs = list_create();
    lista_pcbs = list_create();
    lista_mutex = list_create();

}

void destruir_estados() {
    // Destrucción de colas
    queue_destroy(cola_new_procesos);
    queue_destroy(cola_ready_fifo);
    queue_destroy(cola_exit);
    queue_destroy(cola_IO);
    queue_destroy(cola_exit_procesos);

    // Destrucción de listas
    list_destroy(lista_ready_prioridad);
    for(int i = 0; i< list_size(colas_ready_prioridad); i++){
        t_cola_prioridad* cola = list_get(colas_ready_prioridad,i);
        queue_destroy(cola->cola);
        free(cola);
    }
    list_destroy(colas_ready_prioridad);
    list_destroy(lista_bloqueados);
    list_destroy(lista_tcbs);
    list_destroy(lista_pcbs);
    list_destroy(lista_mutex);
}

void inicializar_semaforos() {
    sem_init(&sem_cola_IO,0,0);
    sem_init(&semaforo_cola_ready,0,0);
    sem_init(&sem_fin_kernel,0,0);
    sem_init(&semaforo_new_ready_procesos, 0, 0);   // Inicializa en 0
    sem_init(&semaforo_cola_new_procesos, 0, 0);    // Inicializa en 0
    sem_init(&semaforo_cola_exit_procesos, 0, 0);   // Inicializa en 0
    sem_init(&sem_desalojado, 0, 1);                // Inicializa en 1
    sem_init(&semaforo_cola_exit_hilos, 0, 0);      // Inicializa en 0
    sem_init(&sem_lista_prioridades, 0, 0);         // Inicializa en 0
    sem_init(&sem_ciclo_nuevo,0,0);
    sem_init(&sem_seguir_o_frenar,0,0);                    
    sem_init(&sem_modulo_terminado,0,0);
    sem_init(&sem_termina_hilo,0,0);
    sem_init(&sem_recibi_ok,0,0);
}

void destruir_semaforos() {
    sem_destroy(&sem_cola_IO);
    sem_destroy(&semaforo_cola_ready);
    sem_destroy(&sem_fin_kernel);
    sem_destroy(&semaforo_new_ready_procesos);
    sem_destroy(&semaforo_cola_new_procesos);
    sem_destroy(&semaforo_cola_exit_procesos);
    sem_destroy(&sem_desalojado);
    sem_destroy(&semaforo_cola_exit_hilos);
    sem_destroy(&sem_lista_prioridades);
    sem_destroy(&sem_ciclo_nuevo);
    sem_destroy(&sem_seguir_o_frenar);
    sem_destroy(&sem_modulo_terminado);
    sem_destroy(&sem_termina_hilo);
    sem_destroy(&sem_recibi_ok);
}

void inicializar_mutex() {
    pthread_mutex_init(&mutex_lista_pcbs, NULL);
    pthread_mutex_init(&mutex_lista_tcbs,NULL);
    pthread_mutex_init(&mutex_cola_new_procesos, NULL);
    pthread_mutex_init(&mutex_cola_exit_procesos, NULL);
    pthread_mutex_init(&mutex_cola_exit_hilos, NULL);
    pthread_mutex_init(&mutex_cola_ready, NULL);
    pthread_mutex_init(&mutex_conexion_kernel_a_dispatch,NULL);
    pthread_mutex_init(&mutex_conexion_kernel_a_interrupt,NULL);
    pthread_mutex_init(&mutex_log,NULL);
    pthread_mutex_init(&mutex_lista_blocked,NULL);
    pthread_mutex_init(&mutex_syscall_ejecutando,NULL);
    pthread_mutex_init(&mutex_desalojo,NULL);
    pthread_mutex_init(&mutex_estado_kernel, NULL);
}

void destruir_mutex() {
    pthread_mutex_destroy(&mutex_lista_pcbs);
    pthread_mutex_destroy(&mutex_lista_tcbs);
    pthread_mutex_destroy(&mutex_cola_new_procesos);
    pthread_mutex_destroy(&mutex_cola_exit_procesos);
    pthread_mutex_destroy(&mutex_cola_exit_hilos);
    pthread_mutex_destroy(&mutex_cola_ready);
    pthread_mutex_destroy(&mutex_conexion_kernel_a_dispatch);
    pthread_mutex_destroy(&mutex_conexion_kernel_a_interrupt);
    pthread_mutex_destroy(&mutex_log);
    pthread_mutex_destroy(&mutex_lista_blocked);
    pthread_mutex_destroy(&mutex_syscall_ejecutando);
    pthread_mutex_destroy(&mutex_desalojo);
    pthread_mutex_destroy(&mutex_estado_kernel);
}


void liberar_proceso(t_pcb *pcb)
{
    char *algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    if (strings_iguales(algoritmo, "FIFO"))
    {   
        pthread_mutex_lock(&mutex_cola_ready);
        sacar_tcbs_de_cola_ready_fifo(cola_ready_fifo, pcb->pid);
        pthread_mutex_unlock(&mutex_cola_ready);
    }
    else if (strings_iguales(algoritmo, "PRIORIDADES")){
        pthread_mutex_lock(&mutex_cola_ready);
        sacar_tcbs_de_lista_ready_prioridades(lista_ready_prioridad,pcb->pid);
        pthread_mutex_unlock(&mutex_cola_ready);
    }
    else if (strings_iguales(algoritmo, "CMN")){
        pthread_mutex_lock(&mutex_cola_ready);
        sacar_tcbs_de_colas_ready_multinivel(colas_ready_prioridad,pcb->pid);
        pthread_mutex_unlock(&mutex_cola_ready);
    }
    
    pthread_mutex_lock(&mutex_lista_blocked);
    sacar_tcbs_lista_blocked(lista_bloqueados, pcb->pid);
    pthread_mutex_unlock(&mutex_lista_blocked);
    
    pthread_mutex_lock(&mutex_cola_exit_hilos);
    enviar_tcbs_a_cola_exit_por_pid(lista_tcbs, cola_exit, pcb->pid);
    pthread_mutex_unlock(&mutex_cola_exit_hilos);
    
    for (int i = 0; i<list_size(pcb->tids); i++){
        sem_wait(&pcb->sem_hilos_eliminar);
    }

    pthread_mutex_lock(&mutex_lista_pcbs);
    list_remove_element(lista_pcbs,pcb);
    pthread_mutex_unlock(&mutex_lista_pcbs);

    sem_destroy(&pcb->sem_hilos_eliminar);
    
    list_destroy_and_destroy_elements(pcb->tids, free);
    
    for (int i = 0; i < list_size(pcb->lista_mutex); i++){
        t_mutex* actual = list_get(pcb->lista_mutex,i);

        queue_destroy(actual->cola_tcbs);
        free(actual->nombre);
        free(actual);
    }
    list_destroy(pcb->lista_mutex);

    pthread_mutex_destroy(&pcb->mutex_lista_mutex);
    pthread_mutex_destroy(&pcb->mutex_tids);
    free(pcb);

}

void sacar_tcbs_de_cola_ready_fifo(t_queue* cola_ready_fifo,int pid_buscado){
    for (int i = 0; i < list_size(cola_ready_fifo->elements); i++) {
        t_tcb* tcb_actual = list_get(cola_ready_fifo->elements, i);
        if (tcb_actual->pid == pid_buscado && hilo_esta_en_ready(tcb_actual)) {
            // Remover el TCB de la cola ready de fifo
           sacar_tcb_de_cola(cola_ready_fifo,tcb_actual);
           sem_wait(&semaforo_cola_ready); // Hay que restar los signal hechos por cada hilo asociado al proceso así no entra a FIFO después
        }
    }
}

void sacar_tcbs_de_lista_ready_prioridades(t_list* lista_prioridades,int pid_buscado){
    for (int i = 0; i < list_size(lista_prioridades); i++) {
        t_tcb* tcb_actual = list_get(lista_prioridades, i);
        if (tcb_actual->pid == pid_buscado && hilo_esta_en_ready(tcb_actual)) {
            // Remover el TCB de la cola ready de prioridades
           list_remove_element(lista_prioridades,tcb_actual);
           sem_wait(&semaforo_cola_ready); // Hay que restar los signal hechos por cada hilo asociado al proceso
        }
    }
}



void sacar_tcbs_de_colas_ready_multinivel(t_list *lista_prioridades, int pid_buscado)
{
    for (int i = 0; i < list_size(colas_ready_prioridad); i++) {
            t_cola_prioridad* cola_prioridad = (t_cola_prioridad*) list_get(colas_ready_prioridad, i);

            // Iterar en la cola de esa prioridad
            for (int j = 0; j < queue_size(cola_prioridad->cola); j++) {
                t_tcb* hilo = (t_tcb*) list_get(cola_prioridad->cola->elements, j);
                if (hilo->pid == pid_buscado) {
                    sem_wait(&semaforo_cola_ready);
                    sacar_tcb_de_cola(cola_prioridad->cola,hilo);
                    j--;
                }
            }
        }
}

void sacar_tcbs_lista_blocked(t_list*lista_bloqueados,int pid_buscado){
    for (int i = 0; i < list_size(lista_bloqueados); i++) {
        t_tcb* tcb_actual = list_get(lista_bloqueados, i);
        if (tcb_actual->pid == pid_buscado) {
            // Remover el TCB de la cola de bloqueados
            list_remove_element(lista_bloqueados,tcb_actual);
        }
    }
}




void enviar_tcbs_a_cola_exit_por_pid(t_list* lista_tcbs, t_queue* cola_exit, int pid_buscado) {
    int i = 0;

    while (!list_is_empty(lista_tcbs) && i < list_size(lista_tcbs)) {  // Condición doble: lista no vacía y índice válido
        t_tcb* tcb_actual = list_get(lista_tcbs, i);  // Obtener el TCB en el índice actual

        if (tcb_actual == NULL) {
            pthread_mutex_lock(&mutex_log);
            log_error(logger, "Error: TCB nulo en índice %d. Abortando.", i);
            pthread_mutex_unlock(&mutex_log);
            break;
        }
    
        if (tcb_actual->pid == pid_buscado) {
            t_tcb* tcb_a_mover = list_remove(lista_tcbs, i);
            if (tcb_a_mover == NULL) {
                pthread_mutex_lock(&mutex_log);
                log_error(logger, "Error: No se pudo remover el TCB del índice %d. Abortando.", i);
                pthread_mutex_unlock(&mutex_log);
                break;
            }

            queue_push(cola_exit, tcb_a_mover);  // Enviar a la cola EXIT
            sem_post(&semaforo_cola_exit_hilos);

            // No incrementar el índice porque la lista se reduce
        } else {
            i++;  // Solo avanzar si no se eliminó un elemento
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

t_tcb* sacar_tcb_ready(t_tcb* tcb) {
    // Busca la cola correspondiente por prioridad
    if(strcmp(config_get_string_value(config,"ALGORITMO_PLANIFICACION"),"FIFO")==0){
    pthread_mutex_lock(&mutex_cola_ready);
    t_tcb* aux = sacar_tcb_de_cola(cola_ready_fifo,tcb);
    pthread_mutex_unlock(&mutex_cola_ready);
    if (aux!=NULL)
        sem_wait(&semaforo_cola_ready);
    return tcb;
    }
    else if(strcmp(config_get_string_value(config,"ALGORITMO_PLANIFICACION"),"PRIORIDADES")==0){
    pthread_mutex_lock(&mutex_cola_ready);
    t_tcb* aux =sacar_tcb_de_lista(lista_ready_prioridad,tcb);
    pthread_mutex_unlock(&mutex_cola_ready);
    if (aux!=NULL)
        sem_wait(&semaforo_cola_ready);
    return tcb;
    }
    else if(strcmp(config_get_string_value(config,"ALGORITMO_PLANIFICACION"),"CMN")==0){
    pthread_mutex_lock(&mutex_cola_ready);
    int tamanio = list_size(colas_ready_prioridad);
    for (int i = 0; i < tamanio; i++) {
        t_cola_prioridad* cola = list_get(colas_ready_prioridad, i);
        if (cola->prioridad == tcb->prioridad) {
            // Itera sobre la cola para buscar el TCB con el ID dado
            t_list* elementos_cola = cola->cola->elements;
            for (int j = 0; j < list_size(elementos_cola); j++) {
                t_tcb* tcb_aux = list_get(elementos_cola, j);
                if (tcb_aux->tid == tcb->tid && tcb_aux->pid == tcb->pid) {
                    // Eliminar el TCB de la cola
                    list_remove(elementos_cola, j); // Lo quita de la lista interna
                    sem_wait(&semaforo_cola_ready); 
                    pthread_mutex_unlock(&mutex_cola_ready);
                    return tcb_aux; // Devuelve el TCB encontrado
                }
            }
        }
    }
    pthread_mutex_unlock(&mutex_cola_ready);
    }
    return NULL; // Si no encuentra la cola o el TCB, devuelve NULL
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

void liberar_tcb(t_tcb* tcb) {
    if (tcb != NULL) {
        // Liberar el pseudocódigo si fue asignado
        if (tcb->pseudocodigo != NULL) {
            free(tcb->pseudocodigo);
            
        }

        // Liberar el propio tcb
        pthread_mutex_destroy(&tcb->mutex_cola_hilos_bloqueados);
        queue_destroy(tcb->cola_hilos_bloqueados);
        free(tcb);
    }
}

bool strings_iguales(char*c1,char*c2){
    return strcmp(c1,c2)==0;
}

t_cola_prioridad* obtener_cola_con_mayor_prioridad(t_list* colas_hilos_prioridad_ready) {
    if (list_is_empty(colas_hilos_prioridad_ready)) {
        sem_wait(&semaforo_cola_ready);

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



//Busca un tcb en los estados ready y block
t_tcb* buscar_tcb(int tid_buscado, t_tcb* hilo_exec) {
    char* algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    pthread_mutex_lock(&mutex_cola_ready);
    // Si es FIFO
    if (strings_iguales(algoritmo, "FIFO")) {
        for (int i = 0; i < queue_size(cola_ready_fifo); i++) {
            t_tcb* hilo = (t_tcb*) list_get(cola_ready_fifo->elements, i);  // Acceso a elementos de la cola FIFO
            if (hilo->tid == tid_buscado && hilo->pid == hilo_exec->pid) {
                pthread_mutex_unlock(&mutex_cola_ready);
                return hilo;
            }
        }
    }
    // Si es PRIORIDADES
    if (strings_iguales(algoritmo, "PRIORIDADES")) {
        for (int i = 0; i < list_size(lista_ready_prioridad); i++) {
            t_tcb* hilo = (t_tcb*) list_get(lista_ready_prioridad, i);
            if (hilo->tid == tid_buscado && hilo->pid == hilo_exec->pid) {
                pthread_mutex_unlock(&mutex_cola_ready);
                return hilo;
            }
        }
    }

    // Si es MULTINIVEL
    if (strings_iguales(algoritmo, "CMN")) {
        

        // Iterar en las colas_ready_procesos
        for (int i = 0; i < list_size(colas_ready_prioridad); i++) {
            t_cola_prioridad* cola_prioridad = (t_cola_prioridad*) list_get(colas_ready_prioridad, i);

            // Iterar en la cola de esa prioridad
            for (int j = 0; j < queue_size(cola_prioridad->cola); j++) {
                t_tcb* hilo = (t_tcb*) list_get(cola_prioridad->cola->elements, j);
                if (hilo->tid == tid_buscado && hilo->pid == hilo_exec->pid) {
                    pthread_mutex_unlock(&mutex_cola_ready);
                    return hilo;
                }
            }
        }

    }
    // Buscar en la lista de bloqueados
  
    for (int i = 0; i < list_size(lista_bloqueados); i++) {
        t_tcb* hilo_bloqueado = (t_tcb*) list_get(lista_bloqueados, i);
        if (hilo_bloqueado->tid == tid_buscado && hilo_bloqueado->pid == hilo_exec->pid) {
            pthread_mutex_unlock(&mutex_cola_ready);
            return hilo_bloqueado;
        }
    }
 
    pthread_mutex_unlock(&mutex_cola_ready);
    // Si no se encontrÃ³ en ninguna parte
    return NULL;
}

bool tcb_metido_en_estructura(t_tcb*tcb){
    char* algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    // Si es FIFO
    if (strings_iguales(algoritmo, "FIFO")) {
        pthread_mutex_lock(&mutex_cola_ready);
        for (int i = 0; i < queue_size(cola_ready_fifo); i++) {
            t_tcb* hilo = (t_tcb*) list_get(cola_ready_fifo->elements, i);  // Acceso a elementos de la cola FIFO
            if (hilo->tid == tcb->tid && hilo->pid == tcb->pid) {
                pthread_mutex_unlock(&mutex_cola_ready);
                return true;
            }
        }
        pthread_mutex_unlock(&mutex_cola_ready);
    }
    // Si es PRIORIDADES
    if (strings_iguales(algoritmo, "PRIORIDADES")) {
        pthread_mutex_lock(&mutex_cola_ready);
        for (int i = 0; i < list_size(lista_ready_prioridad); i++) {
            t_tcb* hilo = (t_tcb*) list_get(lista_ready_prioridad, i);
            if (hilo->tid == tcb->tid && hilo->pid == tcb->pid) {
                pthread_mutex_unlock(&mutex_cola_ready);
                return true;
            }
        }
        pthread_mutex_unlock(&mutex_cola_ready);
    }

    // Si es MULTINIVEL
    if (strings_iguales(algoritmo, "CMN")) {
        

        // Iterar en las colas_ready_procesos
        pthread_mutex_lock(&mutex_cola_ready);
        for (int i = 0; i < list_size(colas_ready_prioridad); i++) {
            t_cola_prioridad* cola_prioridad = (t_cola_prioridad*) list_get(colas_ready_prioridad, i);

            // Iterar en la cola de esa prioridad
            for (int j = 0; j < queue_size(cola_prioridad->cola); j++) {
                t_tcb* hilo = (t_tcb*) list_get(cola_prioridad->cola->elements, j);
                if (hilo->tid == tcb->tid && hilo->pid == tcb->pid) {
                    pthread_mutex_unlock(&mutex_cola_ready);
                    return true;
                }
            }
        }
        pthread_mutex_unlock(&mutex_cola_ready);

    }
    // Buscar en la lista de bloqueados
    pthread_mutex_lock(&mutex_lista_blocked);
    for (int i = 0; i < list_size(lista_bloqueados); i++) {
        t_tcb* hilo_bloqueado = (t_tcb*) list_get(lista_bloqueados, i);
        if (hilo_bloqueado->tid == tcb->tid && hilo_bloqueado->pid == tcb->pid) {
            pthread_mutex_unlock(&mutex_lista_blocked);
            return true;
        }
    }
    pthread_mutex_unlock(&mutex_lista_blocked);
 
    // Si no se encontrÃ³ en ninguna parte
    return false;
}


t_tcb* buscar_tcb_por_tid(t_list* lista_tcbs, int tid_buscado, t_tcb* hilo_exec) {
    for (int i = 0; i < list_size(lista_tcbs); i++) {
        t_tcb* tcb_actual = list_get(lista_tcbs, i);  // Obtener el TCB en la posición 'i'
        if (tcb_actual->tid == tid_buscado && tcb_actual->pid == hilo_exec->pid) {
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

        if (tcb_actual->tid == tcb_a_sacar->tid && tcb_actual->pid == tcb_a_sacar->pid) {
            tcb_encontrado = tcb_actual;  // Encontramos el TCB que queremos sacar
            break;
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

        if (tcb_actual->tid == tcb_a_sacar->tid && tcb_actual->pid == tcb_a_sacar->pid) {
            tcb_encontrado = list_remove(lista, i);  // Elimina el TCB de la lista y lo guarda
            break;  // Salimos del bucle una vez encontrado
        }
    }

    return tcb_encontrado;  // Retorna el TCB extraído, o NULL si no se encontró
}

bool esta_en_lista_blocked(t_tcb*tcb){
    for (int i = 0; i < list_size(lista_bloqueados); i++)
    {
        t_tcb *tcb_actual = list_get(lista_bloqueados, i);

        if (tcb_actual->tid == tcb->tid && tcb_actual->pid == tcb->pid)
        {
            return true;
        }
    }
    return false;
}

t_tcb* buscar_tcb_por_tid_pid(int tid, int pid,t_list* lista_tcbs){
    for (int i = 0; i < list_size(lista_tcbs); i++){
        t_tcb*tcb_actual=list_get(lista_tcbs,i);
        if(tcb_actual->tid == tid && tcb_actual->pid == pid){
            return tcb_actual;
        }
    }
    return NULL;
}

void print_queue(t_queue* queue) {
    if (queue == NULL || queue_size(queue) == 0) {
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"La cola está vacía.");
        pthread_mutex_unlock(&mutex_log);
        return;
    }

    // Iteramos sobre los elementos de la cola sin modificarlos
    for (int i = 0; i < list_size(queue->elements); i++) {
        t_tcb* tcb = list_get(queue->elements, i);  // obtener el elemento
        // Aquí asumo que el tipo de dato es un entero, puedes modificarlo según tu necesidad
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"Elemento %d: PID: %d, TID: %d", i, tcb->pid, tcb->tid);
        pthread_mutex_unlock(&mutex_log);
    }
}

// Función para imprimir los elementos de una lista de t_cola_prioridad
void print_lista_prioridades(t_list* lista_prioridades) {
    if (lista_prioridades == NULL || list_size(lista_prioridades) == 0) {
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"La lista de prioridades está vacía.");
        pthread_mutex_unlock(&mutex_log);
        return;
    }

    // Iteramos sobre la lista de t_cola_prioridad
    for (int i = 0; i < list_size(lista_prioridades); i++) {
        t_cola_prioridad* item = list_get(lista_prioridades, i);
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"Prioridad: %d", item->prioridad);
        log_info(logger,"Cola asociada:");
        pthread_mutex_unlock(&mutex_log);
        print_queue(item->cola);  // Llamamos a la función para imprimir la cola
    }
}

void print_lista(t_list* lista) {
    if (lista == NULL || list_size(lista) == 0) {
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"La lista está vacía.\n");
        pthread_mutex_unlock(&mutex_log);
        return;
    }

    // Iteramos sobre los elementos de la lista
    for (int i = 0; i < list_size(lista); i++) {
        t_tcb* tcb = list_get(lista, i);  // Obtenemos el elemento
        pthread_mutex_lock(&mutex_log);
        log_info(logger,"Elemento %d: PID: %d, TID:%d\n", i, tcb->pid, tcb->tid);  // Imprimimos el valor del elemento
        pthread_mutex_unlock(&mutex_log);
    }
}

bool hilo_esta_en_lista(t_list* lista, int tid, int pid) {
    if (lista == NULL || list_size(lista) == 0) {
        return false;  // La lista está vacía o no inicializada
    }

    // Iteramos sobre la lista de hilos (t_tcb)
    for (int i = 0; i < list_size(lista); i++) {
        t_tcb* hilo = list_get(lista, i);  // Obtenemos el elemento de la lista

        // Verificamos si coinciden el tid y el pid
        if (hilo->tid == tid && hilo->pid == pid) {
            return true;  // Encontramos un hilo con el tid y pid coincidentes
        }
    }

    return false;  // No se encontró ningún hilo con el tid y pid especificados
}

bool hilo_esta_en_cola(t_queue* cola, int tid, int pid) {
    if (cola == NULL || queue_size(cola) == 0) {
        return false;  // La cola está vacía o no inicializada
    }

    // Iteramos sobre los elementos de la cola (t_queue usa internamente una lista)
    for (int i = 0; i < queue_size(cola); i++) {
        t_tcb* hilo = list_get(cola->elements, i);  // Obtenemos el elemento de la cola

        // Verificamos si coinciden el tid y el pid
        if (hilo->tid == tid && hilo->pid == pid) {
            return true;  // Encontramos un hilo con el tid y pid coincidentes
        }
    }

    return false;  // No se encontró ningún hilo con el tid y pid especificados
}

bool hilo_esta_en_colas_multinivel(t_list*colas_ready_prioridad,int tid, int pid, int prioridad){
    for (int i = 0; i< list_size(colas_ready_prioridad); i++){
        t_cola_prioridad*cola_actual=list_get(colas_ready_prioridad,i);
        if (cola_actual->prioridad == prioridad){
            return hilo_esta_en_cola(cola_actual->cola,tid,pid);
        }
    }
    return false;
}

bool hilo_esta_en_ready(t_tcb* hilo){
    char*algoritmo=config_get_string_value(config,"ALGORITMO_PLANIFICACION");

    if (strings_iguales(algoritmo,"FIFO")){
        return hilo_esta_en_cola(cola_ready_fifo,hilo->tid,hilo->pid);
    }
    else if (strings_iguales(algoritmo,"PRIORIDADES")){
        return hilo_esta_en_lista(lista_ready_prioridad,hilo->tid,hilo->pid);
    }
    else if (strings_iguales(algoritmo,"CMN")){
        return hilo_esta_en_colas_multinivel(colas_ready_prioridad,hilo->tid,hilo->pid,hilo->prioridad);
    }
    return false;
}