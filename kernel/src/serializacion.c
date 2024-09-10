#include "includes/funcionesAuxiliares.h"


// typedef struct
// {
//     int tid;
//     int prioridad;
//     int pid; // proceso asociado al hilo



//     char *estado;
//     char *pseudocodigo;

//     int estado_length;
//     int pseudocodigo_length;

//     // No se si me conviene hacer el void * stream acá o trabajarlo x afuera, por ahora lo hago x afuera

// } t_tcb;


// void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
// {
// 	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

// 	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
// 	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

// 	paquete->buffer->size += tamanio + sizeof(int);
// }



t_paquete* agregar_tcb_a_paquete(t_tcb*tcb){

    t_paquete*paquete=crear_paquete();

    agregar_a_paquete(paquete,&(tcb->tid),sizeof(int));
    agregar_a_paquete(paquete,&(tcb->prioridad),sizeof(int));
    agregar_a_paquete(paquete,&(tcb->pid),sizeof(int));
    tcb->estado_length=string_length(tcb->estado);
    agregar_a_paquete(paquete,&(tcb->estado_length),sizeof(int));
    agregar_a_paquete(paquete,tcb->estado,tcb->estado_length);
    tcb->pseudocodigo_length = string_length(tcb->pseudocodigo);
    agregar_a_paquete(paquete,&(tcb->pseudocodigo_length),sizeof(int));
    agregar_a_paquete(paquete,tcb->pseudocodigo,sizeof(tcb->pseudocodigo_length));

    return paquete;

}



// typedef struct
// {
//     int pid;
//     t_list *tids;
//     t_list *colas_hilos_prioridad_ready;
//     t_list *lista_hilos_blocked;
//     t_queue *cola_hilos_new;
//     t_queue *cola_hilos_exit;
//     t_tcb *hilo_exec;
//     char *mutex;
//     char *estado;
//     char *pseudocodigo;
//     int tamanio_proceso;
//     int prioridad;
// } t_pcb;


t_paquete* agregar_pcb_a_paquete(t_pcb*pcb){
    t_paquete*paquete=crear_paquete();

    agregar_a_paquete(paquete,&(pcb->pid),sizeof(int));

	
	agregar_a_paquete(paquete,&(pcb->tamanio_proceso),sizeof(int));

	agregar_a_paquete(paquete,&(pcb->prioridad),sizeof(int));
	

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



void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
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
