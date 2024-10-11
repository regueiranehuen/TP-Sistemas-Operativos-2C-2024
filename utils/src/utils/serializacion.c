#include "includes/serializacion.h"

void send_process_create(char*nombreArchivo,int tamProceso,int prioridad){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = 3*sizeof(int) + strlen(nombreArchivo)+1;

    int offset = 0;
    buffer->stream = malloc(buffer->size);

    int sizeNombreArchivo = strlen(nombreArchivo)+1;
    memcpy(buffer->stream + offset, &sizeNombreArchivo,sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream + offset, &nombreArchivo,sizeNombreArchivo);
    offset += sizeNombreArchivo;
    memcpy(buffer->stream + offset, &tamProceso,sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream + offset, &prioridad,sizeof(int));

    send_paquete_syscall(buffer);

}

void send_thread_create(char*nombreArchivo,int tamProceso,int prioridad){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = 2*sizeof(int) + strlen(nombreArchivo)+1;

    int offset = 0;
    buffer->stream = malloc(buffer->size);

    int sizeNombreArchivo = strlen(nombreArchivo)+1;
    memcpy(buffer->stream + offset, &sizeNombreArchivo,sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream + offset, &nombreArchivo,sizeNombreArchivo);
    offset += sizeNombreArchivo;
    memcpy(buffer->stream + offset, &prioridad,sizeof(int));

    send_paquete_syscall(buffer);
}

void send_paquete_syscall(t_buffer*buffer){
    t_paquete_syscall*paquete=malloc(sizeof(t_paquete_syscall));

    paquete->buffer=buffer;

    // Armamos el stream a enviar
    void *a_enviar = malloc(buffer->size + sizeof(paquete->syscall) + sizeof(int));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->syscall), sizeof(paquete->syscall));
    offset += sizeof(paquete->syscall);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(int));
    offset += sizeof(int);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    // Por último enviamos
    send(sockets->sockets_cliente_cpu->socket_Dispatch, a_enviar, buffer->size + sizeof(paquete->syscall) + sizeof(int),0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    eliminar_paquete_syscall(paquete);
}

t_paquete_syscall* recibir_paquete_syscall(int socket_dispatch){
    t_paquete_syscall*paquete=malloc(sizeof(t_paquete_syscall));
    paquete->buffer=malloc(sizeof(paquete->buffer));

    // Primero recibimos el codigo de operacion
    recv(socket_dispatch, &(paquete->syscall), sizeof(paquete->syscall), 0);

    // Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
    recv(socket_dispatch, &(paquete->buffer->size), sizeof(int), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket_dispatch, paquete->buffer->stream, paquete->buffer->size, 0);

    return paquete;
}

int recibir_entero_buffer(t_paquete_syscall*paquete){
    int valor;
    memcpy(&valor,paquete->buffer->stream,sizeof(int));
    eliminar_paquete_syscall(paquete);
    return valor;
}   

// Esta forma de recibir un paquete solo es para recibir strings
/*t_list* recibir_paquete(int socket_cliente)
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
}*/



t_process_create* parametros_process_create(t_paquete_syscall*paquete){
    t_process_create*info = malloc(sizeof(t_process_create));

    void * stream = paquete->buffer->stream;

    int sizeNombreArchivo;
    // Deserializamos los campos que tenemos en el buffer
    memcpy(&sizeNombreArchivo, stream, sizeof(int)); // Recibimos el size del nombre del archivo de pseudocodigo
    stream += sizeof(int);

    memcpy(&(info->nombreArchivo), stream, sizeNombreArchivo); // Primer parámetro para la syscall: nombre del archivo
    stream += sizeNombreArchivo;
    memcpy(&(info->tamProceso), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->prioridad), stream, sizeof(int));
    stream += sizeof(int);

    eliminar_paquete_syscall(paquete);

    return info;

}

t_thread_create* parametros_thread_create(t_paquete_syscall*paquete){
    t_thread_create*info = malloc(sizeof(t_thread_create));

    void * stream = paquete->buffer->stream;

    int sizeNombreArchivo;
    // Deserializamos los campos que tenemos en el buffer
    memcpy(&sizeNombreArchivo, stream, sizeof(int)); // Recibimos el size del nombre del archivo de pseudocodigo
    stream += sizeof(int);

    memcpy(&(info->nombreArchivo), stream, sizeNombreArchivo); // Primer parámetro para la syscall: nombre del archivo
    stream += sizeNombreArchivo;
    memcpy(&(info->prioridad), stream, sizeof(int));
    stream += sizeof(int);

    eliminar_paquete_syscall(paquete);

    return info;

}



char* recibir_string_paquete_syscall(t_paquete_syscall*paquete){

    void * stream = paquete->buffer->stream;

    int sizeString;

    // Deserializamos los campos que tenemos en el buffer
    memcpy(&sizeString, stream, sizeof(int)); // Recibimos el size del nombre del archivo de pseudocodigo
    stream += sizeof(int);

    char*string = malloc(sizeString);
    memcpy(string, stream, sizeString); // Primer parámetro para la syscall: nombre del archivo
    
    eliminar_paquete_syscall(paquete);

    return string;
}


void send_operacion_tid_pid(code_operacion code, int tid, int pid, int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = 2*sizeof(int);

    int offset = 0;
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream + offset, &tid,sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream + offset, &pid,sizeof(int));
    
    send_paquete_code_operacion(code,buffer,socket_cliente);
    
}

void send_operacion_entero(code_operacion code, int entero, int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = sizeof(int);

    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream, &entero,sizeof(int));

    send_paquete_code_operacion(code,buffer,socket_cliente);
    
}

void send_operacion_pid(code_operacion code, int pid, int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = sizeof(int);

    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream, &pid,sizeof(int));

    send_paquete_code_operacion(code,buffer,socket_cliente);
    
}


void send_paquete_code_operacion(code_operacion code, t_buffer*buffer, int socket_cliente){
    t_paquete_code_operacion*paquete=malloc(sizeof(t_paquete_code_operacion));

    paquete->code = code;
    paquete->buffer=buffer;

    void*a_enviar=malloc(buffer->size + sizeof(code)+sizeof(int));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->code), sizeof(code));
    offset += sizeof(code);
    memcpy(a_enviar + offset, paquete->buffer->stream,paquete->buffer->size); 

    send(socket_cliente,a_enviar,buffer->size + sizeof(code) + sizeof(int),0);

    free(a_enviar);
    eliminar_paquete_code_op(paquete);
}

t_list* recibir_paquete_code_operacion(int socket_cliente){ // Se sabe que se recibe un codigo de operacion, minimo un int (tid/pid) y maximo dos int (pid y tid)
    t_paquete_code_operacion*paquete=malloc(sizeof(t_paquete_code_operacion));
    t_list*valores = list_create();
    paquete->buffer = malloc(sizeof(t_buffer));
    // Primero recibimos el codigo de operacion
    recv(socket_cliente, &(paquete->code), sizeof(code_operacion), 0);

    // Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
    recv(socket_cliente, &(paquete->buffer->size), sizeof(int), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);


    
    int offset = 0;
    while (offset < paquete->buffer->size){
        int* valor=malloc(sizeof(int));
        memcpy(valor,paquete->buffer->stream + offset,sizeof(int));
        offset+=sizeof(int);
        list_add(valores,valor);
    }

    eliminar_paquete_code_op(paquete);

    return valores;
    
}


void liberar_conexion(int socket_cliente)
{
    close(socket_cliente);
}

void eliminar_paquete_syscall(t_paquete_syscall*paquete){
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void eliminar_paquete_code_op(t_paquete_code_operacion*paquete){
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}