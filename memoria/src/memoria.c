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
{
case ENUM_DUMP_MEMORY:
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case ENUM_PROCESS_EXIT:
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case ENUM_PROCESS_CREATE:
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case ENUM_THREAD_CREATE:
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case ENUM_THREAD_EXIT:
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
default:
log_info(logger,"Pedido no disponible");
    break;
}
}
close(socket_cliente);
}
