#include "includes/estructuras.h"
#include "../../memoria/src/includes/server.h"


t_config*iniciar_config(char*path){
    return config_create(path);
}


int recibir_operacion(int socket_cliente){
    int cod_op;
    if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
    {
        return cod_op;
    }
    else
    {
        close(socket_cliente);
        return -1;
    }
}

op_code recibir_op_code(int socket_cliente){
    op_code cod_op;
    if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
    {
        return cod_op;
    }
    else
    {
        close(socket_cliente);
        return -1;
    }
}



void *recibir_buffer(int *size, int socket_cliente)
{
    void *buffer;

    recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    recv(socket_cliente, buffer, *size, MSG_WAITALL);

    return buffer;
}

void recibir_mensaje(int socket_cliente, t_log *loggs)
{
    int size;
    char *buffer = recibir_buffer(&size, socket_cliente);
    log_trace(loggs, "Me llego el mensaje %s", buffer);
    free(buffer);
}

t_list *recibir_paquete(int socket_cliente)
{
    int size;
    int desplazamiento = 0;
    void *buffer;
    t_list *valores = list_create();
    int tamanio;

    buffer = recibir_buffer(&size, socket_cliente);
    while (desplazamiento < size)
    {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        char *valor = malloc(tamanio);
        memcpy(valor, buffer + desplazamiento, tamanio);
        desplazamiento += tamanio;
        list_add(valores, valor);
    }
    free(buffer);
    return valores;
}

t_paquete* recibir_paquete_op_code(int socket_cliente){
    

    op_code codigo_operacion;
    // Primero recibimos el codigo de operacion
    int bytes = recv(socket_cliente, &codigo_operacion, sizeof(int), 0);

    

    if (bytes <= 0){
        return NULL;
    }
    else{
        // Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
        t_paquete* paquete = malloc(sizeof(t_paquete));
        paquete->buffer = malloc(sizeof(t_buffer));
        paquete->codigo_operacion=codigo_operacion; 
        recv(socket_cliente, &(paquete->buffer->size), sizeof(uint32_t), 0);
        paquete->buffer->stream = malloc(paquete->buffer->size);
        if (paquete->buffer->size > 0)
            recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);
        return paquete;
    }
}



void *serializar_paquete(t_paquete *paquete, int bytes)
{
    void *magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento += paquete->buffer->size;

    return magic;
}

void enviar_mensaje(char *mensaje, int socket_cliente)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = MENSAJE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2 * sizeof(int);

    void *a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}

void crear_buffer(t_paquete *paquete)
{
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

t_paquete *crear_paquete(void)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PAQUETE;
    crear_buffer(paquete);
    return paquete;
}

void agregar_a_paquete(t_paquete *paquete, void *valor, uint32_t tamanio)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(uint32_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(uint32_t), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(uint32_t);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
    int bytes = paquete->buffer->size + 2 * sizeof(int);
    void *a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
}

void eliminar_paquete(t_paquete *paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}


/*
typedef struct{
    int pid;
    t_list*contextos_tids;
    uint32_t base; 
    uint32_t limite; 
}t_contexto_pid;
*/




void enviar_registros_a_actualizar(int socket_cliente,t_registros_cpu*registros,int pid, int tid){
    
    t_buffer* buffer = malloc(sizeof(t_buffer));
    
    buffer->size = 2*sizeof(int) + 9*sizeof(uint32_t);
    buffer->stream = malloc(buffer->size);

    void* stream = buffer->stream;

    memcpy(stream,&(pid),sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&(tid),sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&(registros->AX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(registros->BX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(registros->CX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(registros->DX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(registros->EX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(registros->FX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(registros->GX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(registros->HX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(registros->PC),sizeof(uint32_t));
    
    op_code code = ACTUALIZAR_CONTEXTO_TID;

    send_paquete_op_code(socket_cliente,buffer,code);
}


void send_paquete_op_code(int socket, t_buffer* buffer, op_code code){
    t_paquete*paquete= malloc(sizeof(t_paquete));

    if(buffer==NULL){
    void* a_enviar = malloc(sizeof(int));
    memcpy(a_enviar, &(paquete->codigo_operacion), sizeof(int));
    send(socket, a_enviar,sizeof(int),0);
    free(a_enviar);
    } else{

    paquete->codigo_operacion=code;
    paquete->buffer = buffer;

    int total_size = sizeof(int) + sizeof(uint32_t) + buffer->size;
    void* a_enviar = malloc(total_size);

    int offset=0;

    memcpy(a_enviar+offset, &(paquete->codigo_operacion), sizeof(int));
    offset += sizeof(int);
    memcpy(a_enviar+offset, &(paquete->buffer->size), sizeof(uint32_t));  // Agregar el tamaño del buffer
    offset += sizeof(uint32_t);
    if (paquete->buffer->size>0)
        memcpy(a_enviar+offset, paquete->buffer->stream, paquete->buffer->size);

    send(socket, a_enviar, total_size, 0);
    free(a_enviar);
    }

    eliminar_paquete(paquete);
}




void enviar_codop(int conexion, op_code cod_op)
{
    t_paquete *codigo = crear_paquete_op(cod_op);

    enviar_paquete(codigo, conexion);

    eliminar_paquete(codigo);
}

void send_valor_read_mem(uint32_t valor, int socket_cliente, op_code code){
if(code == OK_OP_CODE){
t_buffer* buffer = malloc(sizeof(t_buffer));

 buffer->size = sizeof(uint32_t);
 buffer->stream = malloc(buffer->size);
 
 void* stream = buffer->stream;

 memcpy(stream,&valor,sizeof(uint32_t));
 stream += sizeof(uint32_t);

    send_paquete_op_code(socket_cliente,buffer,code);
}
else{
    send_paquete_op_code(socket_cliente,NULL,code);
}

}

void send_read_mem(uint32_t direccionFisica, int socket_memoria){
 t_buffer* buffer = malloc(sizeof(t_buffer));

 buffer->size = sizeof(uint32_t);
 buffer->stream = malloc(buffer->size);
 
 void* stream = buffer->stream;

 memcpy(stream,&direccionFisica,sizeof(uint32_t));

 op_code code = READ_MEM;

 send_paquete_op_code(socket_memoria,buffer,code);    
}

void send_write_mem(uint32_t direccionFisica, uint32_t valor, int socket_memoria){
t_buffer* buffer = malloc(sizeof(t_buffer));

 buffer->size = 2*sizeof(uint32_t);
 buffer->stream = malloc(buffer->size);
 
 void* stream = buffer->stream;

 memcpy(stream,&direccionFisica,sizeof(uint32_t));
 stream += sizeof(uint32_t);
 memcpy(stream,&valor,sizeof(uint32_t));

 op_code code = WRITE_MEM;

 send_paquete_op_code(socket_memoria,buffer,code); 
}

t_write_mem* recepcionar_write_mem(t_paquete* paquete){
void* stream = paquete->buffer->stream;

t_write_mem* info = malloc(sizeof(t_write_mem));

uint32_t direccionFisica;
uint32_t valor;

memcpy(&(direccionFisica),stream,sizeof(uint32_t));
stream += sizeof(uint32_t);
memcpy(&(valor),stream,sizeof(uint32_t));

info->direccionFisica = direccionFisica;
info->valor = valor;

eliminar_paquete(paquete);
return info;  
}

uint32_t recepcionar_read_mem(t_paquete* paquete){
void* stream = paquete->buffer->stream;

uint32_t direccionFisica;

memcpy(&(direccionFisica),stream,sizeof(uint32_t));

eliminar_paquete(paquete);
return direccionFisica;
}

void solicitar_contexto_ejecucion(int pid, int tid,int conexion){
    t_buffer* buffer = malloc(sizeof(t_buffer));

    buffer->size = 2*sizeof(int);
    buffer->stream = malloc(buffer->size);

    void* stream = buffer->stream;

    
    memcpy(stream,&tid,sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&pid,sizeof(int));

    op_code code = OBTENER_CONTEXTO_EJECUCION;

    send_paquete_op_code(conexion,buffer,code);
}

void enviar_contexto_ejecucion(t_contextos*contextos,int socket_cliente){
    t_buffer* buffer = malloc(sizeof(t_buffer));

    buffer->size = 3*sizeof(int) + 11*sizeof(uint32_t);
    buffer->stream = malloc(buffer->size);

    void* stream = buffer->stream;

    memcpy(stream,&contextos->contexto_pid->pid,sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&contextos->contexto_tid->tid,sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&contextos->contexto_pid->base,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&contextos->contexto_pid->limite,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&contextos->contexto_pid->tamanio_proceso,sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&(contextos->contexto_tid->registros->AX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contextos->contexto_tid->registros->BX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contextos->contexto_tid->registros->CX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contextos->contexto_tid->registros->DX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contextos->contexto_tid->registros->EX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contextos->contexto_tid->registros->FX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contextos->contexto_tid->registros->GX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contextos->contexto_tid->registros->HX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contextos->contexto_tid->registros->PC),sizeof(uint32_t));
    
    op_code code = OBTENCION_CONTEXTO_EJECUCION_OK;

    send_paquete_op_code(socket_cliente,buffer,code);
}

t_contextos* recepcionar_contextos(t_paquete* paquete) {
    t_contextos* contextos = malloc(sizeof(t_contextos));
    contextos->contexto_pid = malloc(sizeof(t_contexto_pid_send));
    contextos->contexto_tid = malloc(sizeof(t_contexto_tid));
    contextos->contexto_tid->registros = malloc(sizeof(t_registros_cpu));

    void* stream = paquete->buffer->stream;

    // Corregir el orden en memcpy (destino <- fuente)
    memcpy(&contextos->contexto_pid->pid, stream, sizeof(int));
    stream += sizeof(int);

    memcpy(&contextos->contexto_tid->tid, stream, sizeof(int));
    stream += sizeof(int);

    memcpy(&contextos->contexto_pid->base, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&contextos->contexto_pid->limite, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&contextos->contexto_pid->tamanio_proceso, stream, sizeof(int));
    stream += sizeof(int);

    memcpy(&(contextos->contexto_tid->registros->AX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(contextos->contexto_tid->registros->BX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(contextos->contexto_tid->registros->CX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(contextos->contexto_tid->registros->DX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(contextos->contexto_tid->registros->EX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(contextos->contexto_tid->registros->FX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(contextos->contexto_tid->registros->GX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(contextos->contexto_tid->registros->HX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    memcpy(&(contextos->contexto_tid->registros->PC), stream, sizeof(uint32_t));
    
    contextos->contexto_tid->pid=contextos->contexto_pid->pid;

    // Liberar el paquete
    eliminar_paquete(paquete);

    return contextos;
}






t_paquete *crear_paquete_op(op_code codop)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codop;
    crear_buffer(paquete);
    return paquete;
}






// Una vez serializado -> recibimos y leemos estas variables




int leer_entero(char *buffer, int *desplazamiento)
{
    int entero;
    memcpy(&entero, buffer + (*desplazamiento), sizeof(int));
    (*desplazamiento) += sizeof(int);
    return entero;
}




char *leer_string(char *buffer, int *desplazamiento)
{
    int tamanio = leer_entero(buffer, desplazamiento);

    char *palabra = malloc(tamanio + 1);

    memcpy(palabra, buffer + *desplazamiento, tamanio);

    palabra[tamanio] = '\0';

    *desplazamiento += tamanio;

    return palabra;
}

int recibir_entero(int socket){
    int size = 0;
    char *buffer;
    int desp = 0;

    buffer = recibir_buffer(&size, socket);
    int entero_nuevo= leer_entero(buffer, &desp);
    // log_trace(loggs, "Me llego en numero %i", entero_nuevo32);
    free(buffer);
    return entero_nuevo;
}



char *recibir_string(int socket, t_log *loggs)
{

    int size = 0;
    char *buffer;
    int desp = 0;

    buffer = recibir_buffer(&size, socket);
    char *nuevo_string = leer_string(buffer, &desp);
    log_trace(loggs, "Recibi el mensaje %s", nuevo_string);

    free(buffer);
    return nuevo_string;
}






t_contexto_tid* recepcionar_contexto_tid(t_paquete*paquete){ 

    t_contexto_tid*nuevo_contexto=malloc(sizeof(t_contexto_tid));
    nuevo_contexto->registros=malloc(sizeof(t_registros_cpu));
    void* stream = paquete->buffer->stream;
    memcpy(&(nuevo_contexto->pid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(nuevo_contexto->tid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(nuevo_contexto->registros->AX),stream,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(nuevo_contexto->registros->BX),stream,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(nuevo_contexto->registros->CX),stream,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(nuevo_contexto->registros->DX),stream,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(nuevo_contexto->registros->EX),stream,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(nuevo_contexto->registros->FX),stream,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(nuevo_contexto->registros->GX),stream,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(nuevo_contexto->registros->HX),stream,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(nuevo_contexto->registros->PC),stream,sizeof(uint32_t));
    
    eliminar_paquete(paquete);
    
    return nuevo_contexto;
}





t_tid_pid* recepcionar_solicitud_contexto(t_paquete* paquete){
    void* stream = paquete->buffer->stream;
    t_tid_pid* info = malloc(sizeof(t_tid_pid));
    memcpy(&(info->tid),stream,sizeof(int));
    stream+= sizeof(int);
    memcpy(&(info->pid),stream,sizeof(int));
    
    eliminar_paquete(paquete);
    return info;
}


t_instruccion_memoria* recepcionar_solicitud_instruccion_memoria(t_paquete* paquete){
    void* stream = paquete->buffer->stream;
    t_instruccion_memoria* info = malloc(sizeof(t_instruccion_memoria));
    memcpy(&(info->pid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->tid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->pc),stream,sizeof(uint32_t));
    
    eliminar_paquete(paquete);
    return info;
}

void send_terminar_ejecucion_op_code(int socket){
    t_buffer*buffer=malloc(sizeof(t_buffer));
    buffer->size = 0;
    send_paquete_op_code(socket,buffer,TERMINAR_EJECUCION_MODULO_OP_CODE);
}
