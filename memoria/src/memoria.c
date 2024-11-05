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
            break;

        case DUMP_MEMORIA:
            t_tid_pid *info_1 = recepcionar_tid_pid_code_op(paquete);
            log_info(logger,"## Memory Dump solicitado - (PID:TID) - (%d:%d)",info_1->pid,info_1->tid);
            int socket_filesystem = cliente_memoria_filesystem(logger,config);
            t_contexto_pid *contexto_pid_1 = obtener_contexto_pid(info_1->pid);
            int tamanio_proceso = (contexto_pid_1->limite - contexto_pid_1->base);
            send_dump_memory_filesystem(info_1->pid,info_1->tid,tamanio_proceso,socket_filesystem);
            recv(socket_filesystem,&respuesta,sizeof(int),0);
            close(socket_filesystem);
            if(respuesta != OK){
            send(socket_cliente, &respuesta, sizeof(int), 0);
            }
            escritura_datos_archivo(info_1->pid,info_1->tid);
            respuesta = OK;
            send(socket_cliente, &respuesta, sizeof(int), 0);
            break;
        case PROCESS_EXIT_AVISO:
            int pid_1 = recepcionar_int_code_op(paquete);
            liberar_espacio_proceso(pid_1);
            t_contexto_pid* contexto_pid = obtener_contexto_pid(pid_1);
            log_info(logger,"## Proceso Destruido -  PID: %d - Tamaño: %d",pid_1,contexto_pid->tamanio_proceso);
            respuesta = OK;
            send(socket_cliente, &respuesta, sizeof(int), 0);
            break;
        case THREAD_CREATE_AVISO:
            t_args_thread_create_aviso *info_4 = recepcionar_inicializacion_hilo(paquete);
            
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
            break;
        case THREAD_ELIMINATE_AVISO:
            t_tid_pid * info_thread_eliminate = recepcionar_tid_pid_code_op(paquete);
            log_info(logger,"PID:%d,TID:%d",info_thread_eliminate->pid,info_thread_eliminate->tid);
            finalizar_hilo(info_thread_eliminate->tid,info_thread_eliminate->pid);
            log_info(logger,"## Hilo Destruido - (PID:TID) - (%d:%d)",info_thread_eliminate->pid,info_thread_eliminate->tid);
            respuesta = OK;
            send(socket_cliente, &respuesta, sizeof(int), 0);
            break;
        default:
            log_info(logger, "Pedido no disponible");
            break;
        }
    }
    close(socket_cliente);
}

uint32_t obtener_base(){
    uint32_t baseActual=0;
    for (int i = 0; i <list_size(lista_contextos_pids); i++){
        t_contexto_pid*contActual=list_get(lista_contextos_pids,i);
        baseActual+=contActual->limite;
    }
    return baseActual;
}