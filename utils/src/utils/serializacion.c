#include "includes/serializacion.h"

void send_process_create(char* nombreArchivo, int tamProceso, int prioridad, int socket_cliente) {
    t_buffer* buffer = malloc(sizeof(t_buffer));

    buffer->size = 3 * sizeof(int) + strlen(nombreArchivo) + 1; // +1 para el null terminator
    syscalls syscall = ENUM_PROCESS_CREATE;

    int offset = 0;
    buffer->stream = malloc(buffer->size);

    int sizeNombreArchivo = strlen(nombreArchivo) + 1; // Incluimos el null terminator
    memcpy(buffer->stream + offset, &sizeNombreArchivo, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream + offset, nombreArchivo, sizeNombreArchivo); // Cambiado: nombreArchivo en lugar de &nombreArchivo
    offset += sizeNombreArchivo;
    memcpy(buffer->stream + offset, &tamProceso, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream + offset, &prioridad, sizeof(int));

    send_paquete_syscall(buffer, socket_cliente, syscall);
}

void send_thread_create(char*nombreArchivo,int prioridad,int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    syscalls syscall = ENUM_THREAD_CREATE;

    buffer->size = 2*sizeof(int) + strlen(nombreArchivo)+1;

    int offset = 0;
    buffer->stream = malloc(buffer->size);

    int sizeNombreArchivo = strlen(nombreArchivo)+1;
    memcpy(buffer->stream + offset, &sizeNombreArchivo,sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream + offset, nombreArchivo,sizeNombreArchivo);
    offset += sizeNombreArchivo;
    memcpy(buffer->stream + offset, &prioridad,sizeof(int));

    send_paquete_syscall(buffer,socket_cliente,syscall);
}

void send_process_exit(int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = 0;

syscalls syscall = ENUM_PROCESS_EXIT;

send_paquete_syscall(buffer,socket_cliente,syscall);

}

void send_thread_join(int tid, int socket_cliente){

t_buffer* buffer = malloc(sizeof(t_buffer));

buffer->size = sizeof(int);

buffer->offset = 0;
buffer->stream = malloc(buffer->size);

memcpy(buffer->stream + buffer->offset, &tid, sizeof(int));

syscalls syscall = ENUM_THREAD_JOIN;

send_paquete_syscall(buffer,socket_cliente,syscall);
}

void send_thread_cancel(int tid, int socket_cliente){
   t_buffer* buffer = malloc(sizeof(t_buffer));

buffer->size = sizeof(int);

buffer->offset = 0;
buffer->stream = malloc(buffer->size);

memcpy(buffer->stream + buffer->offset, &tid, sizeof(int));

syscalls syscall = ENUM_THREAD_CANCEL;

send_paquete_syscall(buffer,socket_cliente,syscall); 
}

void send_mutex_create(char* recurso, int socket_cliente){
    t_buffer* buffer = malloc(sizeof(t_buffer));

int length_recurso = strlen(recurso)+1;
buffer->size = sizeof(int) + length_recurso;

buffer->offset = 0;
buffer->stream = malloc(buffer->size);



memcpy(buffer->stream + buffer->offset, &length_recurso, sizeof(int));
buffer->offset += sizeof(int);
memcpy(buffer->stream + buffer->offset, recurso, length_recurso);

syscalls syscall = ENUM_MUTEX_CREATE;

send_paquete_syscall(buffer,socket_cliente,syscall);
}

void send_mutex_lock(char* recurso, int socket_cliente){
 t_buffer* buffer = malloc(sizeof(t_buffer));

int lenght_recurso = strlen(recurso)+1;
buffer->size = sizeof(int)+lenght_recurso;

buffer->offset = 0;
buffer->stream = malloc(buffer->size);



memcpy(buffer->stream + buffer->offset, &lenght_recurso, sizeof(int));
buffer->offset += sizeof(int);
memcpy(buffer->stream + buffer->offset, recurso, lenght_recurso);

syscalls syscall = ENUM_MUTEX_LOCK;

send_paquete_syscall(buffer,socket_cliente,syscall);

}

void send_mutex_unlock(char* recurso, int socket_cliente){
 t_buffer* buffer = malloc(sizeof(t_buffer));
int lenght_recurso = strlen(recurso)+1;
buffer->size = sizeof(int)+lenght_recurso;

buffer->offset = 0;
buffer->stream = malloc(buffer->size);



memcpy(buffer->stream + buffer->offset, &lenght_recurso, sizeof(int));
buffer->offset += sizeof(int);
memcpy(buffer->stream + buffer->offset, recurso, lenght_recurso);

syscalls syscall = ENUM_MUTEX_UNLOCK;

send_paquete_syscall(buffer,socket_cliente,syscall);

}

void send_IO(int milisegundos, int socket_cliente){

t_buffer* buffer = malloc(sizeof(t_buffer));

buffer->size = sizeof(int);

buffer->offset = 0;
buffer->stream = malloc(buffer->size);

memcpy(buffer->stream + buffer->offset, &milisegundos, sizeof(int));

syscalls syscall = ENUM_IO;

send_paquete_syscall(buffer,socket_cliente,syscall);
}

void send_dump_memory(int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = 0;

syscalls syscall = ENUM_DUMP_MEMORY;

send_paquete_syscall(buffer,socket_cliente,syscall);

}

void send_thread_exit(int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = 0;

syscalls syscall = ENUM_THREAD_EXIT;

send_paquete_syscall(buffer,socket_cliente,syscall);

}





void send_paquete_syscall_sin_parametros(int socket_cliente, syscalls syscall, t_paquete_syscall* paquete) {
    paquete->syscall = syscall;
    void *a_enviar = malloc(sizeof(paquete->syscall)); // Solo enviamos la syscall
    memcpy(a_enviar, &(paquete->syscall), sizeof(paquete->syscall));

    send(socket_cliente, a_enviar, sizeof(paquete->syscall), 0);

    // Liberar la memoria que ya no usamos
    free(a_enviar);
    eliminar_paquete_syscall(paquete);
}

void send_paquete_syscall(t_buffer*buffer, int socket_cliente,syscalls syscall){
    t_paquete_syscall*paquete=malloc(sizeof(t_paquete_syscall));

    if(syscall == ENUM_PROCESS_EXIT || syscall == ENUM_DUMP_MEMORY){
        send_paquete_syscall_sin_parametros(socket_cliente,syscall,paquete);
    } else{

    paquete->syscall=syscall;
    paquete->buffer=buffer;

    // Armamos el stream a enviar
    void *a_enviar = malloc(buffer->size + sizeof(paquete->syscall) + sizeof(uint32_t));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->syscall), sizeof(paquete->syscall));
    offset += sizeof(paquete->syscall);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(int));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    // Por último enviamos
    send(socket_cliente, a_enviar, buffer->size + sizeof(paquete->syscall) + sizeof(int),0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    eliminar_paquete_syscall(paquete);
}
}
t_paquete_syscall* recibir_paquete_syscall(int socket_dispatch){
    t_paquete_syscall*paquete=malloc(sizeof(t_paquete_syscall));
    paquete->buffer=malloc(sizeof(paquete->buffer));

    // Primero recibimos el codigo de operacion
    recv(socket_dispatch, &(paquete->syscall), sizeof(paquete->syscall), 0);

    // Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
    recv(socket_dispatch, &(paquete->buffer->size), sizeof(uint32_t), 0);
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



t_process_create* parametros_process_create(t_paquete_syscall *paquete){
    t_process_create *info = malloc(paquete->buffer->size);

    void * stream = paquete->buffer->stream;

    // Deserializamos los campos que tenemos en el buffer
    memcpy(&(info->length_nombreArchivo), stream, sizeof(int)); // Recibimos el size del nombre del archivo de pseudocodigo
    stream += sizeof(int);

    memcpy(&(info->nombreArchivo), stream, info->length_nombreArchivo); // Primer parámetro para la syscall: nombre del archivo
    stream += info->length_nombreArchivo;
    memcpy(&(info->tamProceso), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->prioridad), stream, sizeof(int));
    stream += sizeof(int);

    eliminar_paquete_syscall(paquete);

    return info;

}

t_thread_create* parametros_thread_create(t_paquete_syscall*paquete){
    t_thread_create*info = malloc(paquete->buffer->size);

    void * stream = paquete->buffer->stream;

    // Deserializamos los campos que tenemos en el buffer
    memcpy(&info->length_nombreArchivo, stream, sizeof(int)); // Recibimos el size del nombre del archivo de pseudocodigo
    stream += sizeof(int);
    memcpy(&(info->nombreArchivo), stream, info->length_nombreArchivo); // Primer parámetro para la syscall: nombre del archivo
    stream += info->length_nombreArchivo;
    memcpy(&(info->prioridad), stream, sizeof(int));
    stream += sizeof(int);

    eliminar_paquete_syscall(paquete);

    return info;

}

int recibir_entero_paquete_syscall(t_paquete_syscall* paquete){

void* stream = paquete->buffer->stream;

int valor;

memcpy(&valor,stream,sizeof(int));
eliminar_paquete_syscall(paquete);

return valor;
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

void send_operacion_tid(code_operacion code, int tid, int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = sizeof(int);

    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream, &tid,sizeof(int));

    send_paquete_code_operacion(code,buffer,socket_cliente);
    
}



void send_operacion_pid_tamanio_proceso(code_operacion code, int pid, int tamanio_proceso, int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    int offset = 0;

    buffer->size = sizeof(int);

    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream, &pid,sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream,&tamanio_proceso,sizeof(int));

    send_paquete_code_operacion(code,buffer,socket_cliente);
    
}


void send_paquete_solo_code_operacion(int socket_cliente,code_operacion code,t_paquete_code_operacion*paquete){
    paquete->code=code;
    void *a_enviar = malloc(sizeof(int)); // Solo enviamos la syscall
    memcpy(a_enviar, &(paquete->code), sizeof(int));

    send(socket_cliente, a_enviar, sizeof(int), 0);

    // Liberar la memoria que ya no usamos
    free(a_enviar);
    eliminar_paquete_code_op(paquete);
}



void send_paquete_code_operacion(code_operacion code, t_buffer*buffer, int socket_cliente){
    t_paquete_code_operacion*paquete=malloc(sizeof(t_paquete_code_operacion));

    paquete->code = code;
    paquete->buffer = buffer;

    void *a_enviar = malloc(buffer->size + sizeof(int) + sizeof(int));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->code), sizeof(int));
    offset += sizeof(int);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(int), 0);

    free(a_enviar);
    eliminar_paquete_code_op(paquete);
}    




t_paquete_code_operacion* recibir_paquete_code_operacion(int socket_cliente){
    t_paquete_code_operacion*paquete=malloc(sizeof(t_paquete_code_operacion));

    paquete->buffer=malloc(sizeof(paquete->buffer));

    // Primero recibimos el codigo de operacion
    int bytes = recv(socket_cliente, &(paquete->code), sizeof(int), 0);

    if (bytes == 0){
        return NULL;
    }
    else{
        // Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
        
        recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), 0);
        paquete->buffer->stream = malloc(paquete->buffer->size);
        recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);
        return paquete;
    }
}

t_tid_pid* recepcionar_tid_pid_code_op(t_paquete_code_operacion* paquete){
    t_tid_pid* info = malloc(sizeof(t_tid_pid));

    void* stream = paquete->buffer->stream;

    memcpy(&(info->tid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->pid), stream,sizeof(int)); // Primer parámetro para la syscall: nombre del archivo

    eliminar_paquete_code_op(paquete);

    return info;
}

int recepcionar_int_code_op(t_paquete_code_operacion* paquete){


    void* stream = paquete->buffer->stream;

    int valor;

    memcpy(&valor,stream,sizeof(int));
    eliminar_paquete_code_op(paquete);

    return valor;
}

t_process_create_mem* recepcionar_pid_tamanio(t_paquete_code_operacion* paquete){
    t_process_create_mem* info = malloc(sizeof(t_process_create_mem));

    void* stream = paquete->buffer->stream;

    memcpy(&info->pid,stream,sizeof(int));
    stream+=sizeof(int);
    memcpy(&info->tamanio_proceso,stream,sizeof(int));

    eliminar_paquete_code_op(paquete);
    return info;
}

void send_code_operacion(code_operacion code, int socket_cliente){
    send(socket_cliente,&code,sizeof(int),0);
}


code_operacion recibir_code_operacion(int socket_cliente){
    code_operacion code;
    recv(socket_cliente,&code,sizeof(int),0);
    return code;
}



void send_inicializacion_proceso(int pid, char*arch_pseudocodigo,int tamanio_proceso, int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    int offset = 0;
    int length_arch_pseudocodigo=strlen(arch_pseudocodigo)+1;

    buffer->size = 3*sizeof(int)+length_arch_pseudocodigo;
    
    buffer->stream = malloc(buffer->size);

    

    memcpy(buffer->stream, &pid,sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream,&tamanio_proceso,sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream,&length_arch_pseudocodigo,sizeof(int));
    offset+=sizeof(int);
    memcpy(buffer->stream,&arch_pseudocodigo,length_arch_pseudocodigo);
    
    send_paquete_code_operacion(INICIALIZAR_PROCESO,buffer,socket_cliente);
    
}


t_args_inicializar_proceso* recepcionar_inicializacion_proceso(t_paquete_code_operacion*paquete){
    t_args_inicializar_proceso* info = malloc(paquete->buffer->size);

    void* stream = paquete->buffer->stream;


    memcpy(&(info->pid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->tam_proceso), stream,sizeof(int));
    stream+=sizeof(int);
    memcpy(&(info->length_nombreArchivo), stream,sizeof(int));
    stream+=sizeof(int);
    memcpy(&(info->arch_pseudocodigo),stream,info->length_nombreArchivo);

    eliminar_paquete_code_op(paquete);

    return info;
}