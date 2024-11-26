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
int lenght_recurso = strlen(recurso) + 1;

buffer->size = sizeof(int) + lenght_recurso;

buffer->offset = 0;
buffer->stream = malloc(buffer->size);



memcpy(buffer->stream + buffer->offset, &lenght_recurso, sizeof(int));
buffer->offset += sizeof(int);
memcpy(buffer->stream + buffer->offset, recurso, lenght_recurso);

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
int lenght_recurso = strlen(recurso) + 1;
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

void send_ciclo_nuevo(int socket_cliente){
    t_buffer* buffer = malloc(sizeof(t_buffer));

    buffer-> size = 0;

    syscalls syscall = ENUM_CICLO_NUEVO;
    printf("Enviando CICLO NUEVO\n");
    send_paquete_syscall(buffer,socket_cliente,syscall);
}

void send_syscall(syscalls syscall, int socket_cliente){
    send(socket_cliente,&syscall,sizeof(int),0);
}

syscalls recibir_syscall(int socket_cliente){
    syscalls syscall;
    recv(socket_cliente,&syscall,sizeof(int),0);
    return syscall;
}

void send_fin_quantum_rr(int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = 0;

    syscalls syscall = ENUM_FIN_QUANTUM_RR;

    send_paquete_syscall(buffer,socket_cliente,syscall);
}

void send_desalojo(int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = 0;

    syscalls syscall = ENUM_DESALOJAR;

    send_paquete_syscall(buffer,socket_cliente,syscall);
}

void send_segmentation_fault(int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    buffer->size = 0;

    syscalls syscall = ENUM_SEGMENTATION_FAULT;

    send_paquete_syscall(buffer,socket_cliente,syscall);
}


void send_paquete_syscall_sin_parametros(int socket_cliente, syscalls syscall) {
    int n = 0;
    printf("SYSCALL QUE ESTOY POR MANDAR: %d",syscall);
    void *a_enviar = malloc(sizeof(int) + sizeof(int));
    int offset = 0;

    memcpy(a_enviar + offset, &syscall, sizeof(int));
    offset += sizeof(int);
    memcpy(a_enviar + offset, &n, sizeof(int));
    
    send(socket_cliente, a_enviar, sizeof(int) + sizeof(int),0);

    free(a_enviar);

}

void send_paquete_syscall(t_buffer*buffer, int socket_cliente,syscalls syscall){
    

    if(buffer->size == 0){
        send_paquete_syscall_sin_parametros(socket_cliente,syscall);
        free(buffer);
    } else{

    t_paquete_syscall*paquete=malloc(sizeof(t_paquete_syscall));

    paquete->syscall=syscall;
    paquete->buffer=buffer;

    // Armamos el stream a enviar
    void *a_enviar = malloc(buffer->size + sizeof(int) + sizeof(int));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->syscall), sizeof(int));
    offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(int));
    offset += sizeof(int);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    // Por último enviamos
    send(socket_cliente, a_enviar, buffer->size + sizeof(int) + sizeof(int),0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    eliminar_paquete_syscall(paquete);
}

}


t_paquete_syscall* recibir_paquete_syscall(int socket_dispatch) {
    t_paquete_syscall* paquete = malloc(sizeof(t_paquete_syscall));

    paquete->buffer=malloc(sizeof(t_buffer));

    // Primero recibimos el codigo de operacion
    int bytes = recv(socket_dispatch, &(paquete->syscall), sizeof(int), 0);

    //if (paquete->syscall == ENUM_DUMP_MEMORY || paquete->syscall == THREAD_EX)

    if (bytes <= 0){
        return NULL;
    }
    else{
        // Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
        
        recv(socket_dispatch, &(paquete->buffer->size), sizeof(uint32_t), 0);

        if (paquete->buffer->size > 0){
            paquete->buffer->stream = malloc(paquete->buffer->size);
            recv(socket_dispatch, paquete->buffer->stream, paquete->buffer->size, 0);
        }

        
        return paquete;
    }

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

    int sizeNombreArchivo = 0;
    // Deserializamos los campos que tenemos en el buffer
    memcpy(&sizeNombreArchivo, stream, sizeof(int)); // Recibimos el size del nombre del archivo de pseudocodigo
    stream += sizeof(int);
    info->nombreArchivo = malloc(sizeNombreArchivo + 1);
    memcpy(info->nombreArchivo, stream, sizeNombreArchivo); // Primer parámetro para la syscall: nombre del archivo
    stream += sizeNombreArchivo;
    memcpy(&(info->tamProceso), stream, sizeof(int));
    stream += sizeof(int);
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



void send_operacion_pid_tamanio_proceso(code_operacion code, int pid, int tamanio_proceso, int socket_cliente) {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = 2 * sizeof(int); // Asegura espacio para pid y tamanio_proceso
    buffer->stream = malloc(buffer->size);

    int offset = 0;
    memcpy(buffer->stream + offset, &pid, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer->stream + offset, &tamanio_proceso, sizeof(int));
    
    send_paquete_code_operacion(code, buffer, socket_cliente);

    free(buffer->stream);
    free(buffer);
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



void send_paquete_code_operacion(code_operacion code, t_buffer* buffer, int socket_cliente) {
    t_paquete_code_operacion* paquete = malloc(sizeof(t_paquete_code_operacion));
    paquete->code = code;
    paquete->buffer = buffer;

    // Ajustar tamaño del mensaje incluyendo code, buffer->size y el stream completo
    int total_size = sizeof(int) + sizeof(uint32_t) + buffer->size;
    void *a_enviar = malloc(total_size);

    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->code), sizeof(int));
    offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));  // Agregar el tamaño del buffer
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    send(socket_cliente, a_enviar, total_size, 0);

    free(a_enviar);
    eliminar_paquete_code_op(paquete); // Asegúrate de liberar bien todos los recursos en esta función
}




t_paquete_code_operacion* recibir_paquete_code_operacion(int socket_cliente){
    t_paquete_code_operacion*paquete=malloc(sizeof(t_paquete_code_operacion));

    paquete->buffer=malloc(sizeof(t_buffer));

    // Primero recibimos el codigo de operacion
    int bytes = recv(socket_cliente, &(paquete->code), sizeof(int), 0);

    if (bytes <= 0){
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
    stream+=sizeof(int);
    

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
    int bytes = recv(socket_cliente,&code,sizeof(int),0);
    if(bytes <= 0){
    return -1;
    }
    return code;
}

void send_inicializacion_hilo(int tid, int pid, char*arch_pseudocodigo,int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    
    int length_arch_pseudocodigo=strlen(arch_pseudocodigo)+1;

    


    buffer->size = 3*sizeof(int)+length_arch_pseudocodigo;
    
    buffer->stream = malloc(buffer->size);
    void* stream = buffer->stream;
    

    memcpy(stream, &pid,sizeof(int));
    stream += sizeof(int);
    memcpy(stream, &tid, sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&length_arch_pseudocodigo,sizeof(int));
    stream+=sizeof(int);
    memcpy(stream,arch_pseudocodigo,length_arch_pseudocodigo);
    
    send_paquete_code_operacion(THREAD_CREATE_AVISO,buffer,socket_cliente);
    
}



void send_inicializacion_proceso(int pid, char*arch_pseudocodigo,int tamanio_proceso, int socket_cliente){
    t_buffer*buffer=malloc(sizeof(t_buffer));

    
    int length_arch_pseudocodigo=strlen(arch_pseudocodigo)+1;

    buffer->size = 3*sizeof(int)+length_arch_pseudocodigo;
    
    buffer->stream = malloc(buffer->size);
    void* stream = buffer->stream;
    

    memcpy(stream, &pid,sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&tamanio_proceso,sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&length_arch_pseudocodigo,sizeof(int));
    stream+=sizeof(int);
    memcpy(stream,arch_pseudocodigo,length_arch_pseudocodigo);
    
    send_paquete_code_operacion(INICIALIZAR_PROCESO,buffer,socket_cliente);
    
}

void send_dump_memory_filesystem(int pid,int tid,int tamanio_particion_proceso, void*contenido, int socket_cliente){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    
    
    // Calcula el tamaño total del buffer: 3 enteros + todos los datos en la lista
    int buffer_size = 3 * sizeof(int) + tamanio_particion_proceso;

    buffer->size = buffer_size;
    buffer->stream = malloc(buffer->size);

    void* stream = buffer->stream;

    memcpy(stream,&pid,sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&tid,sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&tamanio_particion_proceso,sizeof(int));
    stream+=sizeof(int);
    memcpy(stream,contenido,tamanio_particion_proceso);
    
    send_paquete_code_operacion(DUMP_MEMORIA,buffer,socket_cliente);

}

t_args_dump_memory* recepcionar_dump_memory_filesystem(t_paquete_code_operacion* paquete){
    t_args_dump_memory* info = malloc(sizeof(t_args_dump_memory)); // APLICAR SIEMPRE ASI

    void* stream = paquete->buffer->stream;

    memcpy(&(info->pid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->tid), stream,sizeof(int));
    stream+=sizeof(int);
    memcpy(&(info->tamanio_particion_proceso), stream,sizeof(int));
    stream += sizeof(int);

    info->contenido=malloc(info->tamanio_particion_proceso);
    memcpy(info->contenido,stream,info->tamanio_particion_proceso);

    eliminar_paquete_code_op(paquete);

    return info; 
}


t_args_inicializar_proceso* recepcionar_inicializacion_proceso(t_paquete_code_operacion*paquete){
    t_args_inicializar_proceso* info = malloc(sizeof(t_args_inicializar_proceso)); // APLICAR SIEMPRE ASI

    void* stream = paquete->buffer->stream;
    
    int length_arch_pseudocodigo;

    memcpy(&(info->pid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->tam_proceso), stream,sizeof(int));
    stream+=sizeof(int);
    memcpy(&length_arch_pseudocodigo, stream,sizeof(int));
    stream+=sizeof(int);

    info->arch_pseudocodigo = malloc(length_arch_pseudocodigo + 1); // +1 para el terminador NULL si es string
    memcpy(info->arch_pseudocodigo,stream,length_arch_pseudocodigo);
    
    eliminar_paquete_code_op(paquete);

    return info;
}

t_args_thread_create_aviso* recepcionar_inicializacion_hilo(t_paquete_code_operacion*paquete){
    t_args_thread_create_aviso* info = malloc(sizeof(t_args_thread_create_aviso)); // APLICAR SIEMPRE ASI

    void* stream = paquete->buffer->stream;

    int length_arch_pseudocodigo;

    memcpy(&(info->pid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->tid),stream,sizeof(int));
    stream+=sizeof(int);
    memcpy(&length_arch_pseudocodigo, stream,sizeof(int));
    stream+=sizeof(int);

    info->arch_pseudo = malloc(length_arch_pseudocodigo + 1);
    memcpy(info->arch_pseudo,stream,length_arch_pseudocodigo);
    
    eliminar_paquete_code_op(paquete);

    return info;
}

char* obtener_ruta_absoluta(const char *ruta_relativa) {
    char* ruta_absoluta=malloc(100);

    if (realpath(ruta_relativa, ruta_absoluta) != NULL) {
        printf("Ruta absoluta: %s\n", ruta_absoluta);
    } else {
        perror("Error obteniendo la ruta absoluta");
    }

    return ruta_absoluta;
}

t_log_level log_level(t_config* config){
    char* log_level = config_get_string_value(config,"LOG_LEVEL");
    if(strcmp(log_level,"TRACE")== 0){
    return LOG_LEVEL_TRACE;
    }
    else if(strcmp(log_level,"DEBUG")== 0){
    return LOG_LEVEL_DEBUG;
    }
    else if(strcmp(log_level,"INFO")== 0){
    return LOG_LEVEL_INFO;
    }
    else if(strcmp(log_level,"WARNING")==0){
    return LOG_LEVEL_WARNING;
    }
    else if(strcmp(log_level,"ERROR")==0){
    return LOG_LEVEL_ERROR;
    }
    return -1;
}