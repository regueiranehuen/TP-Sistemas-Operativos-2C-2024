#include "includes/memoria.h"

void atender_conexiones(int socket_cliente){

code_operacion respuesta;
while(estado_cpu != 0){
t_paquete_code_operacion* paquete = recibir_paquete_code_operacion(socket_cliente);
if(paquete == NULL){//cierre de conexión
break;
}
switch (paquete->code)
{// hay que enviar el pid/tid correspondiente que vamos a crear o eliminar. Por ejemplo: Para thread_exit o thread_cancel hay que mandarle a memoria el tid que vamos a eliminar

case INICIALIZAR_PROCESO:
t_args_inicializar_proceso* info_0 = recepcionar_inicializacion_proceso(paquete);
respuesta=OK;
send(socket_cliente,&respuesta,sizeof(int),0);
inicializar_contexto_pid(info_0->pid,0,info_0->tam_proceso);


break; 

case DUMP_MEMORIA:
t_tid_pid* info_1 = recepcionar_tid_pid_code_op(paquete);
respuesta = OK;
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case PROCESS_EXIT_AVISO:
int pid_1 = recepcionar_int_code_op(paquete);
respuesta = OK;
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case PROCESS_CREATE_AVISO:
t_process_create_mem* info_2 = recepcionar_pid_tamanio(paquete);
respuesta = OK;
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case THREAD_CREATE_AVISO:
int tid_1 = recepcionar_int_code_op(paquete);
respuesta = OK;
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
case THREAD_ELIMINATE_AVISO:
int tid_2 = recepcionar_int_code_op(paquete);
respuesta = OK;
send(socket_cliente,&respuesta,sizeof(int),0);
    break;
default:
log_info(logger,"Pedido no disponible");
    break;
}
}
close(socket_cliente);
}
