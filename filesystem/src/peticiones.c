#include "includes/peticiones.h"


void atender_conexiones(int socket_cliente){
    pthread_mutex_lock(&mutex_logs);
    log_info(log_filesystem,"Atendiendo conexiones");
    pthread_mutex_unlock(&mutex_logs);
    code_operacion respuesta;
    bool conexion = true;
    while(conexion){
        pthread_mutex_lock(&mutex_logs);
        log_info(log_filesystem,"Esperando paquete de Memoria");
        pthread_mutex_unlock(&mutex_logs);
        t_paquete_code_operacion* paquete = recibir_paquete_code_operacion(socket_cliente);

            if (paquete == NULL)
            {   
                pthread_mutex_lock(&mutex_logs);
                log_info(log_filesystem,"Cierre de conexion");
                pthread_mutex_unlock(&mutex_logs);
                conexion = false;
                continue;
            }
            switch (paquete->code){ // hay que enviar el pid/tid correspondiente que vamos a crear o eliminar. Por ejemplo: Para thread_exit o thread_cancel hay que mandarle a memoria el tid que vamos a eliminar
            
                case DUMP_MEMORIA:

                    t_args_dump_memory* info = recepcionar_dump_memory_filesystem(paquete);

                    char* aura = crear_archivo_dump(info, bitmap, mount_dir, block_size);
                    if(aura != NULL) { 
                        respuesta = OK;
                        free(aura); // Solo liberamos aura cuando != NULL
                    } else {
                        respuesta = ERROR;
                    }

                    send(socket_cliente,&respuesta,sizeof(int),0);

                    free(info->contenido);
                    free(info);
                    break;
                case TERMINAR_EJECUCION_MODULO:
                    pthread_mutex_lock(&mutex_logs);
                    log_info(log_filesystem,"LLEGA TERMINAR_EJECUCION_MODULO");
                    pthread_mutex_unlock(&mutex_logs);
                    free(paquete->buffer);
                    free(paquete);
                    send_code_operacion(OK_TERMINAR,socket_cliente);

                    sem_post(&sem_fin_filesystem);

                    conexion = false;
                    break;
                default:
                    pthread_mutex_lock(&mutex_logs);
                    log_info(log_filesystem,"Pedido no disponible");
                    pthread_mutex_unlock(&mutex_logs);
                    free(paquete->buffer->stream);
                    free(paquete->buffer);
                    free(paquete);
                    break;

            }
                

        
        }

close(socket_cliente);

}

//bloque de punteros
//funcion escribir_bloques
