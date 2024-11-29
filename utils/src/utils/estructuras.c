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

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->codigo_operacion=codigo_operacion;

    if (bytes <= 0){
        return NULL;
    }
    else{
        // Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
        
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


void agregar_entero_a_paquete(t_paquete *paquete, uint32_t numero)
{

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint32_t));
    paquete->buffer->size += sizeof(uint32_t);
}

void agregar_entero_uint8_a_paquete(t_paquete *paquete, uint8_t numero)
{

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint8_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint8_t));
    paquete->buffer->size += sizeof(uint8_t);
}

void agregar_entero_uint32_a_paquete(t_paquete *paquete, uint32_t numero)
{

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint32_t));
    paquete->buffer->size += sizeof(uint32_t);
}

void agregar_string_a_paquete(t_paquete *paquete, char *palabra)
{

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(char *));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &palabra, sizeof(char *));
    paquete->buffer->size += sizeof(char *);
}

void enviar_entero(int conexion, uint32_t numero, int codop)
{
    t_paquete *paquete = crear_paquete_op(codop);

    agregar_entero_a_paquete(paquete, numero);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void enviar_string(int conexion, char *palabra, int codop)
{
    t_paquete *paquete = crear_paquete_op(codop);

    agregar_a_paquete(paquete, palabra, strlen(palabra) + 1);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

/*
typedef struct{
    int pid;
    t_list*contextos_tids;
    uint32_t base; 
    uint32_t limite; 
}t_contexto_pid;
*/

void send_contexto_pid(int socket_cliente,t_contexto_pid_send*contexto){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    
    buffer->size =2*sizeof(uint32_t) + 2*sizeof(int);
    buffer->stream = malloc(buffer->size);

    void* stream = buffer->stream;

    memcpy(stream,&(contexto->pid),sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&(contexto->base),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contexto->limite),sizeof(uint32_t));
    stream+=sizeof(uint32_t);
    memcpy(stream,&(contexto->tamanio_proceso),sizeof(int));

    op_code code = OBTENCION_CONTEXTO_PID_OK;

    free(contexto);

    send_paquete_op_code(socket_cliente,buffer,code);
}

void send_contexto_tid(int socket_cliente,t_contexto_tid*contexto){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    
    buffer->size = 2*sizeof(int) + 9*sizeof(uint32_t);
    buffer->stream = malloc(buffer->size);

    void* stream = buffer->stream;

    memcpy(stream,&(contexto->pid),sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&(contexto->tid),sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&(contexto->registros->AX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contexto->registros->BX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contexto->registros->CX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contexto->registros->DX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contexto->registros->EX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contexto->registros->FX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contexto->registros->GX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contexto->registros->HX),sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(stream,&(contexto->registros->PC),sizeof(uint32_t));
    
    op_code code = OBTENCION_CONTEXTO_TID_OK;


    send_paquete_op_code(socket_cliente,buffer,code);
}


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


void enviar_3_enteros(int conexion, t_3_enteros *enteros, int codop)
{
    t_paquete *paquete = crear_paquete_op(codop);

    agregar_3_enteros_a_paquete(paquete, enteros);
    enviar_paquete(paquete, conexion);
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

void solicitar_contexto_tid(int pid, int tid,int conexion){
    t_buffer* buffer = malloc(sizeof(t_buffer));

    buffer->size = 2*sizeof(int);
    buffer->stream = malloc(buffer->size);

    void* stream = buffer->stream;

    
    memcpy(stream,&pid,sizeof(int));
    stream += sizeof(int);
    memcpy(stream,&tid,sizeof(int));

    op_code code = OBTENER_CONTEXTO_TID;

    send_paquete_op_code(conexion,buffer,code);
}

void solicitar_contexto_pid(int pid,int conexion){
    t_buffer* buffer = malloc(sizeof(t_buffer));

    buffer->size = sizeof(int);
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream,&pid,sizeof(int));

    op_code code = OBTENER_CONTEXTO_PID;

    send_paquete_op_code(conexion,buffer,code);
}


void enviar_paquete_string(int conexion, char *string, op_code codOP, int tamanio)
{
    t_paquete *paquete = crear_paquete_op(codOP);
    agregar_a_paquete(paquete, string, tamanio);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void enviar_codigo(t_paquete *codop, int socket_cliente)
{

    void *magic = malloc(sizeof(int));

    memcpy(magic, &(codop->codigo_operacion), sizeof(int));

    send(socket_cliente, magic, sizeof(int), 0);

    free(magic);
}

void eliminar_codigo(t_paquete *codop)
{
    free(codop);
}

void enviar_2_enteros_1_string(int conexion, t_string_2enteros *enteros_string, int codop)
{
    t_paquete *paquete = crear_paquete_op(codop);

    agregar_2_enteros_1_string_a_paquete(paquete, enteros_string);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void enviar_3_enteros_1_string(int conexion, t_string_3enteros *enteros_string, int codop)
{
    t_paquete *paquete = crear_paquete_op(codop);

    agregar_3_enteros_1_string_a_paquete(paquete, enteros_string);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

t_paquete *crear_paquete_op(op_code codop)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codop;
    crear_buffer(paquete);
    return paquete;
}

void agregar_registros_a_paquete(t_paquete *paquete, t_registros_cpu *registros)
{

    agregar_entero_uint32_a_paquete(paquete, registros->AX);
    agregar_entero_uint32_a_paquete(paquete, registros->BX);
    agregar_entero_uint32_a_paquete(paquete, registros->CX);
    agregar_entero_uint32_a_paquete(paquete, registros->DX);
    agregar_entero_uint32_a_paquete(paquete, registros->EX);
    agregar_entero_uint32_a_paquete(paquete, registros->FX);
    agregar_entero_uint32_a_paquete(paquete, registros->GX);
    agregar_entero_uint32_a_paquete(paquete, registros->HX);
    agregar_entero_uint32_a_paquete(paquete, registros->PC);

}

void agregar_entero_int_a_paquete(t_paquete *paquete, int numero)
{

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(int));
    paquete->buffer->size += sizeof(int);
}


void agregar_contexto_pid_a_paquete(t_paquete*paquete,t_contexto_pid*contexto){
    agregar_entero_a_paquete(paquete,contexto->pid);
    agregar_entero_a_paquete(paquete,contexto->base);
    agregar_entero_a_paquete(paquete,contexto->limite);
    
    for (int i = 0; i< list_size(contexto->contextos_tids); i++){
        agregar_contexto_tid_a_paquete(paquete,(t_contexto_tid*)list_get(contexto->contextos_tids,i));
    }
}

void agregar_contexto_tid_a_paquete(t_paquete*paquete,t_contexto_tid*contexto){
    agregar_entero_a_paquete(paquete,contexto->pid);

    agregar_entero_a_paquete(paquete,contexto->tid);
    agregar_registros_a_paquete(paquete,contexto->registros);
}


void agregar_2_enteros_a_paquete(t_paquete *paquete, t_2_enteros *enteros)
{
    agregar_entero_a_paquete(paquete, enteros->entero1);
    agregar_entero_a_paquete(paquete, enteros->entero2);
}

void agregar_3_enteros_a_paquete(t_paquete *paquete, t_3_enteros *enteros)
{
    agregar_entero_a_paquete(paquete, enteros->entero1);
    agregar_entero_a_paquete(paquete, enteros->entero2);
    agregar_entero_a_paquete(paquete, enteros->entero3);
}

void agregar_2_enteros_1_string_a_paquete(t_paquete *paquete, t_string_2enteros *enteros_string)
{
    agregar_entero_a_paquete(paquete, enteros_string->entero1);
    agregar_entero_a_paquete(paquete, enteros_string->entero2);
    agregar_a_paquete(paquete, enteros_string->string, strlen(enteros_string->string) + 1);
}

void agregar_3_enteros_1_string_a_paquete(t_paquete *paquete, t_string_3enteros *enteros_string)
{
    agregar_entero_a_paquete(paquete, enteros_string->entero1);
    agregar_entero_a_paquete(paquete, enteros_string->entero2);
    agregar_entero_a_paquete(paquete, enteros_string->entero3);
    agregar_a_paquete(paquete, enteros_string->string, strlen(enteros_string->string) + 1);
}

void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion *instruccion_nueva)
{
    agregar_a_paquete(paquete, instruccion_nueva->parametros1, strlen(instruccion_nueva->parametros1) + 1);

    if (strcmp(instruccion_nueva->parametros1, "SET") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
        agregar_a_paquete(paquete, instruccion_nueva->parametros3, strlen(instruccion_nueva->parametros3) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "SUM") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
        agregar_a_paquete(paquete, instruccion_nueva->parametros3, strlen(instruccion_nueva->parametros3) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "SUB") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
        agregar_a_paquete(paquete, instruccion_nueva->parametros3, strlen(instruccion_nueva->parametros3) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "JNZ") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
        agregar_a_paquete(paquete, instruccion_nueva->parametros3, strlen(instruccion_nueva->parametros3) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "LOG") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "IO") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "PROCESS_CREATE") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
        agregar_a_paquete(paquete, instruccion_nueva->parametros3, strlen(instruccion_nueva->parametros3) + 1);
        agregar_a_paquete(paquete, instruccion_nueva->parametros4, strlen(instruccion_nueva->parametros4) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "THREAD_CREATE") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
        agregar_a_paquete(paquete, instruccion_nueva->parametros3, strlen(instruccion_nueva->parametros3) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "THREAD_JOIN") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "THREAD_CANCEL") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "MUTEX_CREATE") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "MUTEX_LOCK") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
    }
    if (strcmp(instruccion_nueva->parametros1, "MUTEX_UNLOCK") == 0)
    {
        agregar_a_paquete(paquete, instruccion_nueva->parametros2, strlen(instruccion_nueva->parametros2) + 1);
    }
}

// Una vez serializado -> recibimos y leemos estas variables



uint32_t leer_entero_uint32(void*buffer, int *desplazamiento)
{
    uint32_t entero;
    memcpy(&entero, buffer+(*desplazamiento), sizeof(uint32_t));
    *desplazamiento += sizeof(uint32_t);
    return entero;
}

int leer_entero(char *buffer, int *desplazamiento)
{
    int entero;
    memcpy(&entero, buffer + (*desplazamiento), sizeof(int));
    (*desplazamiento) += sizeof(int);
    return entero;
}

int recepcionar_entero_paquete(t_paquete*paquete){
    int entero;
    memcpy(&entero, paquete->buffer->stream, sizeof(int));
    paquete->buffer->stream+=sizeof(int);
    return entero;
}

uint32_t recepcionar_uint32_paquete(t_paquete*paquete){
    uint32_t entero;
    memcpy(&entero, paquete->buffer->stream, sizeof(uint32_t));
    paquete->buffer->stream+=sizeof(uint32_t);
    return entero;
}

uint8_t leer_entero_uint8(char *buffer, int *desplazamiento)
{
    uint8_t entero;
    memcpy(&entero, buffer + (*desplazamiento), sizeof(uint8_t));
    (*desplazamiento) += sizeof(uint8_t);
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

uint32_t recibir_entero_uint32(int socket)
{

    int size = 0;
    char *buffer;
    int desp = 0;

    buffer = recibir_buffer(&size, socket);
    u_int32_t entero_nuevo32 = leer_entero_uint32(buffer, &desp);
    // log_trace(loggs, "Me llego en numero %i", entero_nuevo32);
    free(buffer);
    return entero_nuevo32;
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




t_list *recibir_doble_entero(int socket)
{
    int size = 0;
    char *buffer;
    int desp = 0;
    t_list *devolver = list_create();
    buffer = recibir_buffer(&size, socket);
    u_int32_t entero_nuevo1 = leer_entero_uint32(buffer, &desp);
    u_int32_t entero_nuevo2 = leer_entero_uint32(buffer, &desp);
    list_add(devolver, (void *)(uintptr_t)entero_nuevo1);
    list_add(devolver, (void *)(uintptr_t)entero_nuevo2);
    free(buffer);

    return devolver;
}




void recibir_3_string(int conexion_kernel_cpu_dispatch, char **palabra1, char **palabra2, char **palabra3)
{
    int size = 0;
    char *buffer;
    int desp = 0;
    buffer = recibir_buffer(&size, conexion_kernel_cpu_dispatch);
    *palabra1 = leer_string(buffer, &desp);
    *palabra2 = leer_string(buffer, &desp);
    *palabra3 = leer_string(buffer, &desp);
}


t_2_enteros *recibir_2_enteros(int socket)
{

    t_2_enteros *nuevos_enteros = malloc(sizeof(t_2_enteros));

    int size = 0;
    char *buffer;
    int desp = 0;

    buffer = recibir_buffer(&size, socket);

    nuevos_enteros->entero1 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero2 = leer_entero_uint32(buffer, &desp);

    free(buffer);
    return nuevos_enteros;
}

t_3_enteros *recibir_3_enteros(int socket)
{

    t_3_enteros *nuevos_enteros = malloc(sizeof(t_3_enteros));

    int size = 0;
    char *buffer;
    int desp = 0;

    buffer = recibir_buffer(&size, socket);

    nuevos_enteros->entero1 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero2 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero3 = leer_entero_uint32(buffer, &desp);

    free(buffer);
    return nuevos_enteros;
}

t_4_enteros *recibir_4_enteros(int socket)
{

    t_4_enteros *nuevos_enteros = malloc(sizeof(t_4_enteros));

    int size = 0;
    char *buffer;
    int desp = 0;

    buffer = recibir_buffer(&size, socket);

    nuevos_enteros->entero1 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero2 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero3 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero4 = leer_entero_uint32(buffer, &desp);

    free(buffer);
    return nuevos_enteros;
}

t_string_3enteros *recibir_string_3_enteros(int socket)
{

    t_string_3enteros *nuevos_enteros = malloc(sizeof(t_string_3enteros));

    int size = 0;
    char *buffer;
    int desp = 0;

    buffer = recibir_buffer(&size, socket);

    nuevos_enteros->entero1 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero2 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->entero3 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros->string = leer_string(buffer, &desp);

    free(buffer);
    return nuevos_enteros;
}

t_string_2enteros *recibir_string_2enteros(int socket)
{

    t_string_2enteros *nuevos_enteros_string = malloc(sizeof(t_string_2enteros));

    int size = 0;
    char *buffer;
    int desp = 0;

    buffer = recibir_buffer(&size, socket);

    nuevos_enteros_string->entero1 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros_string->entero2 = leer_entero_uint32(buffer, &desp);
    nuevos_enteros_string->string = leer_string(buffer, &desp);

    free(buffer);
    return nuevos_enteros_string;
}

t_string_mas_entero *recibir_string_mas_entero(int socket, t_log *loggs)
{

    t_string_mas_entero *nuevos_entero_string = malloc(sizeof(t_string_mas_entero));

    int size = 0;
    char *buffer;
    int desp = 0;

    buffer = recibir_buffer(&size, socket);

    nuevos_entero_string->entero1 = leer_entero_uint32(buffer, &desp);
    nuevos_entero_string->string = leer_string(buffer, &desp);

    free(buffer);
    return nuevos_entero_string;
}




t_tid_pid* recepcionar_tid_pid_op_code(void*stream){
    t_tid_pid* info = malloc(sizeof(t_tid_pid));

    

    memcpy(&(info->pid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(info->tid), stream,sizeof(int));
    stream += sizeof(int);

    //eliminar_paquete(paquete);

    return info;
}


t_tid_pid_pc*recepcionar_tid_pid_pc(t_paquete*paquete){
    int desp = 0;
    t_tid_pid_pc*info=malloc(sizeof(t_tid_pid_pc));
    info->pid=leer_entero(paquete->buffer->stream,&desp);
    info->tid=leer_entero(paquete->buffer->stream,&desp);
    info->pc=leer_entero_uint32(paquete->buffer->stream,&desp);
    
    eliminar_paquete(paquete);

    return info;
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

uint32_t leer_registros(void*stream){
    uint32_t reg=0;

    memcpy(&reg,stream,sizeof(uint32_t));

    stream+=sizeof(uint32_t);

    return reg;
}

/*t_contexto_tid *recepcionar_registros(t_paquete *paquete, void *stream) {
    t_registros_cpu *reg = malloc(sizeof(t_registros_cpu));

    memcpy(&(reg->AX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(reg->BX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(reg->CX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(reg->DX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(reg->EX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(reg->FX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(reg->GX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(reg->HX), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(reg->PC), stream, sizeof(uint32_t)); // Último valor leído correctamente

    eliminar_paquete(paquete);
    return reg;
}*/


t_contexto_pid_send* recepcionar_contexto_pid(t_paquete*paquete){
    t_contexto_pid_send* contexto = malloc(sizeof(t_contexto_pid_send));
    
    void* stream = paquete->buffer->stream;

    memcpy(&(contexto->pid),stream,sizeof(int));
    stream += sizeof(int);
    memcpy(&(contexto->base),stream,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(contexto->limite),stream,sizeof(uint32_t));
    stream +=sizeof(uint32_t);
    memcpy(&(contexto->tamanio_proceso),stream,sizeof(uint32_t));
    eliminar_paquete(paquete);
    return contexto;
}

int recepcionar_solicitud_contexto_pid(t_paquete* paquete_operacion){
    
    void* stream = paquete_operacion->buffer->stream;

    int pid;

    memcpy(&(pid),stream,sizeof(int));
    
    eliminar_paquete(paquete_operacion);

    return pid;
}

t_tid_pid* recepcionar_solicitud_contexto_tid(t_paquete* paquete){
    void* stream = paquete->buffer->stream;
    t_tid_pid* info = malloc(sizeof(t_tid_pid));
    memcpy(&(info->pid),stream,sizeof(int));
    stream+= sizeof(int);
    memcpy(&(info->tid),stream,sizeof(int));
    
    eliminar_paquete(paquete);
    return info;
}

void pedir_creacion_contexto_tid(int pid, int tid,int conexion){
    t_paquete *paquete = crear_paquete_op(CREAR_CONTEXTO_TID);
    agregar_entero_int_a_paquete(paquete,pid);
    agregar_entero_int_a_paquete(paquete,tid);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
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
