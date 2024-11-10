#include "includes/peticiones.h"

void atender_conexiones(int socket_cliente){

code_operacion respuesta;
bool conexion = true;
while(conexion){

t_paquete_code_operacion* paquete = recibir_paquete_code_operacion(socket_cliente);

        if (paquete == NULL)
        {   log_info(log_filesystem,"Cierre de conexion");
            conexion = false;
            continue;
        }

        log_info(log_filesystem,"recibi esto de Memoria: %d",paquete->code);

        switch (paquete->code)
        { // hay que enviar el pid/tid correspondiente que vamos a crear o eliminar. Por ejemplo: Para thread_exit o thread_cancel hay que mandarle a memoria el tid que vamos a eliminar

        case DUMP_MEMORIA:
        //char* nombre, int tamanio, contenido
        char* path = config_get_string_value(config, "MOUNT_DIR");
        int block_count = config_get_int_value(config, "BLOCK_COUNT");
        int block_size = config_get_int_value(config, "BLOCK_SIZE");
        t_bitarray* bitmap = cargar_bitmap(path, block_count);
        t_args_dump_memory* info = recepcionar_dump_memory_filesystem(paquete);
        
        respuesta = crear_archivo_dump(info, bitmap, path, block_size);
        
        if(respuesta != -1) { 
            respuesta = OK;
        } else {
            respuesta = ERROR;
        }

        send(socket_cliente,&respuesta,sizeof(int),0);
        break;

        default:
            log_info(log_filesystem,"Pedido no disponible");
            break;

        }
            

    }

close(socket_cliente);

}