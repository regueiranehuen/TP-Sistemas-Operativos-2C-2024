#include "utils/includes/serializacion.h"


void agregar_tid_a_paquete(t_tcb* tcb, t_paquete* paquete) {  
    agregar_a_paquete(paquete, &(tcb->tid), sizeof(int));
}


void send_tid(t_tcb* tcb, int socket_memoria) {
    t_paquete* paquete = crear_paquete();
    agregar_tid_a_paquete(tcb, paquete);
    enviar_paquete(paquete, socket_memoria);
    // eliminar_paquete(paquete);
}


void agregar_pid_a_paquete(t_pcb* pcb, t_paquete* paquete) {
    agregar_a_paquete(paquete, &(pcb->pid), sizeof(int));
}


void send_pid(t_pcb* pcb, int socket_memoria) {
    t_paquete* paquete = crear_paquete();
    agregar_pid_a_paquete(pcb, paquete);
    enviar_paquete(paquete, socket_memoria);
    // eliminar_paquete(paquete);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
    void * magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento+= paquete->buffer->size;

    return magic;
}


void crear_buffer(t_paquete* paquete)
{
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PAQUETE;
    crear_buffer(paquete);
    return paquete;
}
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
    int bytes = paquete->buffer->size + 2*sizeof(int);
    void* a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void liberar_conexion(int socket_cliente)
{
    close(socket_cliente);
}


void send_operacion_tid(int tid, code_operacion code, int socket_cliente){
    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete,&code,sizeof(code_operacion));
    agregar_a_paquete(paquete,&tid,sizeof(tid));

    enviar_paquete(paquete,socket_cliente);

    // ??? eliminar_paquete(paquete);
}

t_paquete* send_operacion_tid_pid(int pid,int tid,code_operacion code, int socket_cliente){
    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete,&code,sizeof(code_operacion));
    agregar_a_paquete(paquete,&pid,sizeof(pid));
    agregar_a_paquete(paquete,&tid,sizeof(tid));

    enviar_paquete(paquete,socket_cliente);

    // ??? eliminar_paquete(paquete);
    return paquete;
}


t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void agregar_tcb_a_paquete(t_tcb*tcb,t_paquete*paquete){  // AGREGAR LOS CAMPOS QUE FALTAN
    
    agregar_a_paquete(paquete,&(tcb->tid),sizeof(int));
    agregar_a_paquete(paquete,&(tcb->prioridad),sizeof(int));
    agregar_a_paquete(paquete,&(tcb->pid),sizeof(int));
    agregar_a_paquete(paquete,&(tcb->estado),sizeof(int));

    tcb->pseudocodigo_length = string_length(tcb->pseudocodigo);
    agregar_a_paquete(paquete,&(tcb->pseudocodigo_length),sizeof(int));
    agregar_a_paquete(paquete,tcb->pseudocodigo,sizeof(tcb->pseudocodigo_length));

}

void send_tcb(t_tcb*tcb,int socket_memoria){
    t_paquete* paquete = crear_paquete();
    agregar_tcb_a_paquete(tcb,paquete);
    enviar_paquete(paquete,socket_memoria);
}

void agregar_pcb_a_paquete(t_pcb*pcb,t_paquete*paquete){
    agregar_a_paquete(paquete,&(pcb->pid),sizeof(int));
    agregar_a_paquete(paquete,pcb->tids,list_size(pcb->tids)*sizeof(int));
    agregar_a_paquete(paquete,pcb->colas_hilos_prioridad_ready,suma_tam_hilos_colas_en_lista(pcb->colas_hilos_prioridad_ready));
    agregar_a_paquete(paquete,pcb->lista_hilos_blocked,list_size(pcb->lista_hilos_blocked)*sizeof(pthread_t));
    agregar_a_paquete(paquete,pcb->cola_hilos_new,queue_size(pcb->cola_hilos_new)*sizeof(pthread_t));
    agregar_a_paquete(paquete,pcb->cola_hilos_exit,queue_size(pcb->cola_hilos_exit)*sizeof(pthread_t));
    agregar_tcb_a_paquete(pcb->hilo_exec,paquete);

    agregar_a_paquete(paquete,pcb->lista_mutex,list_size(pcb->lista_mutex)*sizeof(int));

    agregar_a_paquete(paquete,&(pcb->estado),sizeof(int));
    agregar_a_paquete(paquete,&(pcb->tamanio_proceso),sizeof(int));
    agregar_a_paquete(paquete,&(pcb->prioridad),sizeof(int));
}


void send_pcb(t_pcb*pcb,int socket_memoria){
    t_paquete*paquete=crear_paquete();
    agregar_pcb_a_paquete(pcb,paquete);
    enviar_paquete(paquete,socket_memoria);
}
