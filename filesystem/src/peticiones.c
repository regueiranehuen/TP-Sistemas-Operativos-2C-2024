#include "includes/peticiones.h"


void atender_conexiones(int socket_cliente){
    code_operacion respuesta;
    bool conexion = true;

    while(conexion){
        t_paquete_code_operacion* paquete = recibir_paquete_code_operacion(socket_cliente);

        if (paquete == NULL){   
            log_info(log_filesystem,"Cierre de conexion");
            conexion = false;
            continue;
        }
        log_info(log_filesystem,"recibi esto de Memoria: %d",paquete->code);
        switch (paquete->code){ // hay que enviar el pid/tid correspondiente que vamos a crear o eliminar. Por ejemplo: Para thread_exit o thread_cancel hay que mandarle a memoria el tid que vamos a eliminar
            case DUMP_MEMORIA: 
                t_args_dump_memory* info = recepcionar_dump_memory_filesystem(paquete);
                respuesta = OK;
                send(socket_cliente,&respuesta,sizeof(int),0);
            break;
            default:
                log_info(log_filesystem,"Pedido no disponible");
                break;
            }      
        }
        close(socket_cliente);
}