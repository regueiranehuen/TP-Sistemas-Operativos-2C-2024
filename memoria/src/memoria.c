#include "includes/memoria.h"


void atender_conexiones(int socket_cliente){

code_operacion cod_op;
int respuesta = 1;
while(estado_cpu != 0){
int bytes = recv(socket_cliente,&cod_op,sizeof(code_operacion),0);

if (bytes == 0) {
        // El cliente ha cerrado la conexión
        break;
} else if (bytes < 0) {
        // Ocurrió un error
        perror("Error al recibir datos");
        break;
}

switch (cod_op)
{// hay que enviar el pid/tid correspondiente que vamos a crear o eliminar. Por ejemplo: Para thread_exit o thread_cancel hay que mandarle a memoria el tid que vamos a eliminar
case DUMP_MEMORIA:
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case PROCESS_EXIT_AVISO:
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case PROCESS_CREATE_AVISO:
int tamanio_proceso;
recv(socket_cliente,&tamanio_proceso,sizeof(int),0);
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case THREAD_CREATE_AVISO:
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case THREAD_ELIMINATE_AVISO:
int tid;
recv(socket_cliente,&tid,sizeof(int),0);
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
default:
log_info(logger,"Pedido no disponible");
    break;
}
}
close(socket_cliente);
}
