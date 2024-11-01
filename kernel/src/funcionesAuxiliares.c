#include "includes/funcionesAuxiliares.h"

pthread_mutex_t mutex_lista_blocked;

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

    // Destrucción de listas
    list_destroy(lista_ready_prioridad);
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
}

void inicializar_mutex() {
    pthread_mutex_init(&mutex_lista_pcbs, NULL);
    pthread_mutex_init(&mutex_cola_new_procesos, NULL);
    pthread_mutex_init(&mutex_cola_exit_procesos, NULL);
    pthread_mutex_init(&mutex_cola_exit_hilos, NULL);
    pthread_mutex_init(&mutex_cola_ready, NULL);
    pthread_mutex_init(&mutex_conexion_kernel_a_dispatch,NULL);
    pthread_mutex_init(&mutex_conexion_kernel_a_interrupt,NULL);
    pthread_mutex_init(&mutex_log,NULL);
}

void destruir_mutex() {
    pthread_mutex_destroy(&mutex_lista_pcbs);
    pthread_mutex_destroy(&mutex_cola_new_procesos);
    pthread_mutex_destroy(&mutex_cola_exit_procesos);
    pthread_mutex_destroy(&mutex_cola_exit_hilos);
    pthread_mutex_destroy(&mutex_cola_ready);
    pthread_mutex_destroy(&mutex_conexion_kernel_a_dispatch);
    pthread_mutex_destroy(&mutex_conexion_kernel_a_interrupt);
    pthread_mutex_destroy(&mutex_log);
}

void liberar_proceso (t_pcb * pcb){

enviar_tcbs_a_cola_exit_por_pid(lista_tcbs, cola_exit, pcb->pid);
list_destroy(pcb->tids);//falta liberar lista de mutexes
pthread_mutex_destroy(&pcb->mutex_lista_mutex);
pthread_mutex_destroy(&pcb->mutex_tids);
free(pcb);
}

void enviar_tcbs_a_cola_exit_por_pid(t_list* lista_tcbs, t_queue* cola_exit, int pid_buscado) {
    for (int i = 0; i < list_size(lista_tcbs); i++) {
        t_tcb* tcb_actual = list_get(lista_tcbs, i);
        printf("tid:%d\n",tcb_actual->tid);  // Obtener el TCB de la lista
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
            log_info(logger,"pseudocodigo:%s",tcb->pseudocodigo);
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
        sem_wait(&semaforo_cola_ready);  // Espera hasta que haya elementos en alguna cola
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

//Busca un tcb en los estados ready y block
t_tcb* buscar_tcb(int tid_buscado, t_tcb* hilo_exec) {
    char* algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    pthread_mutex_lock(&mutex_cola_ready);
    // Si es FIFO
    if (strings_iguales(algoritmo, "FIFO")) {
        for (int i = 0; i < queue_size(cola_ready_fifo); i++) {
            t_tcb* hilo = (t_tcb*) list_get(cola_ready_fifo->elements, i);  // Acceso a elementos de la cola FIFO
            if (hilo->tid == tid_buscado && hilo->pid == hilo_exec->pid) {
                return hilo;
            }
        }
    }
    // Si es PRIORIDADES
    if (strings_iguales(algoritmo, "PRIORIDADES")) {
        for (int i = 0; i < list_size(lista_ready_prioridad); i++) {
            t_tcb* hilo = (t_tcb*) list_get(lista_ready_prioridad, i);
            if (hilo->tid == tid_buscado && hilo->pid == hilo_exec->pid) {
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
                    return hilo;
                }
            }
        }

    }
    // Buscar en la lista de bloqueados
  
    for (int i = 0; i < list_size(lista_bloqueados); i++) {
        t_tcb* hilo_bloqueado = (t_tcb*) list_get(lista_bloqueados, i);
        if (hilo_bloqueado->tid == tid_buscado && hilo_bloqueado->pid == hilo_exec->pid) {
            return hilo_bloqueado;
        }
    }
 
    pthread_mutex_unlock(&mutex_cola_ready);
    // Si no se encontrÃ³ en ninguna parte
    return NULL;
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