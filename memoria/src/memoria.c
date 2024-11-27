#include "includes/memoria.h"

void atender_conexiones(int socket_cliente)
{

    code_operacion respuesta;
    bool conexion = true;
    while (conexion)
    {
        
        t_paquete_code_operacion *paquete = recibir_paquete_code_operacion(socket_cliente);
    
        if (paquete == NULL)
        {   log_info(logger,"Cierre de conexion");
            conexion = false;
            continue;
        }

        log_info(logger,"recibi esto de kernel: %d",paquete->code);

        switch (paquete->code)
        { // hay que enviar el pid/tid correspondiente que vamos a crear o eliminar. Por ejemplo: Para thread_exit o thread_cancel hay que mandarle a memoria el tid que vamos a eliminar

        case INICIALIZAR_PROCESO:
            t_args_inicializar_proceso *info_0 = recepcionar_inicializacion_proceso(paquete);
            pthread_mutex_lock(&mutex_lista_contextos_pids);
            pthread_mutex_unlock(&mutex_lista_contextos_pids);
            printf("tamanio proceso:%d\n",info_0->tam_proceso);
            t_particiones* particion = inicializar_proceso(info_0->pid,info_0->tam_proceso,config);
            if(particion == NULL){
            log_info(logger,"particion == NULL");
            respuesta = -1;
            send(socket_cliente,&respuesta,sizeof(int),0);       
            }else{
            log_info(logger,"Particion: Base: %d, Limite: %d, Tamanio: %d, Ocupada: %d, PID: %d",particion->base,particion->limite,particion->tamanio,particion->ocupada,particion->pid);
            inicializar_contexto_pid(info_0->pid, particion->base,particion->limite,info_0->tam_proceso);
            log_info(logger,"## Proceso Creado -  PID: %d - Tamaño: %d",info_0->pid,info_0->tam_proceso);
            respuesta = OK;
            send(socket_cliente, &respuesta, sizeof(int), 0);
            }
            free(info_0);
            break;

        case DUMP_MEMORIA:
            t_tid_pid *info_1 = recepcionar_tid_pid_code_op(paquete);
            log_info(logger,"## Memory Dump solicitado - (PID:TID) - (%d:%d)",info_1->pid,info_1->tid);
            int socket_filesystem = cliente_memoria_filesystem(logger,config);

            t_list* lista_datos = lectura_datos_proceso(info_1->pid);
            if(lista_datos == NULL){
                respuesta = -1;
                send(socket_cliente,&respuesta,sizeof(int),0);
                break;
            }
            t_particiones* particion_proceso = obtener_particion(info_1->pid);
            if (particion_proceso == NULL){
                log_info(logger,"No se encontró la partición del proceso de pid %d",info_1->pid);
                respuesta=ERROR;
                send(socket_cliente, &respuesta, sizeof(int), 0);
                break;
            }

            void*contenido = memoria + particion_proceso->base;

            send_dump_memory_filesystem(info_1->pid,info_1->tid,particion_proceso->tamanio,contenido,socket_filesystem);
            recv(socket_filesystem,&respuesta,sizeof(int),0);
            close(socket_filesystem);
            if(respuesta != OK){
            send(socket_cliente, &respuesta, sizeof(int), 0);
            break;
            }
            respuesta = OK;
            send(socket_cliente, &respuesta, sizeof(int), 0);
            break;
        case PROCESS_EXIT_AVISO:
            int pid_1 = recepcionar_int_code_op(paquete);
            log_info(logger,"ME LLEGÓ ESTE PID AMIGAZO: %d",pid_1);
            t_contexto_pid* contexto_pid = obtener_contexto_pid(pid_1);
            if(contexto_pid == NULL){
                log_info(logger,"Soy un cornudo");
            }
            int tam_proceso = contexto_pid->tamanio_proceso;
            eliminar_contexto_pid(contexto_pid);
            liberar_espacio_proceso(pid_1);
            log_info(logger,"## Proceso Destruido -  PID: %d - Tamanio: %d",pid_1,tam_proceso);
            respuesta = OK;
            send(socket_cliente, &respuesta, sizeof(int), 0);
            break;
        case THREAD_CREATE_AVISO:
            log_info(logger,"LLEGA THREAD CREATE AVISO A MEMORIA");
            t_args_thread_create_aviso *info_4 = recepcionar_inicializacion_hilo(paquete);
            log_info(logger,"%d",info_4->pid);
            log_info(logger,"%d",info_4->tid);
            log_info(logger,"%s",info_4->arch_pseudo);
            

            t_contexto_pid *contexto_pid_4 = obtener_contexto_pid(info_4->pid);

            if(contexto_pid_4 == NULL){
            log_info(logger,"No se encontro el contexto buscado");
            }
            
            if (info_4->tid != 0){ // Ya que el hilo 0 se inicializa cuando se inicializa el proceso
                inicializar_contexto_tid(contexto_pid_4, info_4->tid);
            }
            
            cargar_instrucciones_desde_archivo(info_4->arch_pseudo,info_4->pid,info_4->tid);
            log_info(logger,"## Hilo Creado - (PID:TID) - (%d:%d)",info_4->pid,info_4->tid);
            respuesta = OK;
            send(socket_cliente, &respuesta, sizeof(int), 0);
            free(info_4);
            break;
        case THREAD_ELIMINATE_AVISO:
            t_tid_pid * info_thread_eliminate = recepcionar_tid_pid_code_op(paquete);
            log_info(logger,"PID:%d,TID:%d",info_thread_eliminate->pid,info_thread_eliminate->tid);
            finalizar_hilo(info_thread_eliminate->tid,info_thread_eliminate->pid);
            log_info(logger,"## Hilo Destruido - (PID:TID) - (%d:%d)",info_thread_eliminate->pid,info_thread_eliminate->tid);
            respuesta = OK;
            send(socket_cliente, &respuesta, sizeof(int), 0);
            free(info_thread_eliminate);
            break;
        default:
            log_info(logger, "Pedido no disponible");
            break;
        }
    }
    close(socket_cliente);
}