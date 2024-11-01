#include "includes/commCpu.h"

void* recibir_cpu(void*args) {

    log_info(logger,"2_socket de cpu: %d", sockets_iniciales->socket_cpu);
    int retardo_respuesta = config_get_int_value(config,"RETARDO_RESPUESTA");
    int codigoOperacion = 0;
    while (codigoOperacion != -1) {
        
        t_paquete*paquete_operacion = recibir_paquete_op_code(sockets_iniciales->socket_cpu);

        if(paquete_operacion==NULL){
            printf("Memoria recibio un paquete == NULL de cpu");
            break;
        }
            printf("Memoria recibio el siguiente codigo operacion de CPU: %d",paquete_operacion->codigo_operacion);

        usleep(retardo_respuesta * 1000);  // Aplicar retardo configurado

        switch (paquete_operacion->codigo_operacion) {
            case OBTENER_CONTEXTO_TID: {
                t_tid_pid *info = recepcionar_solicitud_contexto_tid(paquete_operacion);  // Recibe PID y TID

                log_info(logger, "## Contexto solicitado - (PID:TID) - (%d:%d)",info->pid,info->tid);
                
                t_contexto_tid*contexto_tid=obtener_contexto_tid(info->pid,info->tid);
                
                if (contexto_tid == NULL){
                    log_error(logger, "No se encontro el contexto del tid %d asociado al pid %d", contexto_tid->tid,contexto_tid->pid);
                    send_paquete_op_code(sockets_iniciales->socket_cpu,NULL,CONTEXTO_TID_INEXISTENTE);
                    break;
                }

                send_contexto_tid(sockets_iniciales->socket_cpu,contexto_tid);
                log_info(logger, "Enviado contexto para PID: %d, TID: %d", contexto_tid->pid, contexto_tid->tid);
                

                break;
            }

            case OBTENER_CONTEXTO_PID:{ 
                int pid_obtencion = recepcionar_solicitud_contexto_pid(paquete_operacion);

                
                t_contexto_pid* contextoPid = obtener_contexto_pid(pid_obtencion);
                
                if (contextoPid == NULL){
                    log_error(logger, "No se encontro el contexto del pid %d", pid_obtencion);
                    send_paquete_op_code(sockets_iniciales->socket_cpu,NULL,CONTEXTO_PID_INEXISTENTE);
                    break;
                }
                t_contexto_pid_send* contexto_a_enviar = malloc(sizeof(contexto_a_enviar));
                contexto_a_enviar->pid = contextoPid->pid;
                contexto_a_enviar->base = contextoPid->base;
                contexto_a_enviar->limite=contextoPid->limite;

                send_contexto_pid(sockets_iniciales->socket_cpu,contexto_a_enviar);
                break;
            }


            case ACTUALIZAR_CONTEXTO_TID:{

                t_contexto_tid*contexto_tid=recepcionar_contexto_tid(paquete_operacion);

                log_info(logger, "PROGRAM COUNTER ACTUAL: %u", contexto_tid->registros->PC);
                log_info(logger, "AX A ENVIAR: %u", contexto_tid->registros->AX);
                log_info(logger, "BX A ENVIAR: %u", contexto_tid->registros->BX);
                log_info(logger, "CX A ENVIAR: %u", contexto_tid->registros->CX);
                log_info(logger, "DX A ENVIAR: %u", contexto_tid->registros->DX);
                log_info(logger, "EX A ENVIAR: %u", contexto_tid->registros->EX);
                log_info(logger, "FX A ENVIAR: %u", contexto_tid->registros->FX);
                log_info(logger, "GX A ENVIAR: %u", contexto_tid->registros->GX);
                log_info(logger, "HX A ENVIAR: %u", contexto_tid->registros->HX);

                actualizar_contexto(contexto_tid->pid,contexto_tid->tid,contexto_tid->registros);
                send_code_operacion(OK,sockets_iniciales->socket_cpu);
                log_info(logger, "## Contexto actualizado - (PID:TID) - (%d:%d)", contexto_tid->pid,contexto_tid->tid);
                free(contexto_tid);
                break;
            }

            case OBTENER_INSTRUCCION: {

                t_instruccion_memoria* solicitud_instruccion = recepcionar_solicitud_instruccion_memoria(paquete_operacion);
                log_info(logger,"tid %d, pid %d, pc %d",solicitud_instruccion->tid,solicitud_instruccion->pid,solicitud_instruccion->pc);
                t_instruccion *instruccion = obtener_instruccion(solicitud_instruccion->tid, solicitud_instruccion->pid,solicitud_instruccion->pc);
                if (instruccion==NULL){
                    log_info(logger,"putoooo");
                }

                enviar_instruccion(sockets_iniciales->socket_cpu, instruccion, INSTRUCCION_OBTENIDA);
                if(strcmp(instruccion->parametros2,"") == 0){
                log_info(logger,"## Obtener instruccion - (PID:TID) - (%d:%d) - Instruccion: <%s>",solicitud_instruccion->pid,solicitud_instruccion->tid,instruccion->parametros1);
                }
                else if(strcmp(instruccion->parametros3,"")== 0){
                log_info(logger,"## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: <%s> <%s>",solicitud_instruccion->pid,solicitud_instruccion->tid,instruccion->parametros1,instruccion->parametros2);
                }
                else if(strcmp(instruccion->parametros4,"")== 0){
                log_info(logger,"## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: <%s> <%s> <%s>",solicitud_instruccion->pid,solicitud_instruccion->tid,instruccion->parametros1,instruccion->parametros2,instruccion->parametros3);    
                } 
                else{
                log_info(logger,"## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: <%s> <%s> <%s> <%s>",solicitud_instruccion->pid,solicitud_instruccion->tid,instruccion->parametros1,instruccion->parametros2,instruccion->parametros3,instruccion->parametros4);
                }
                
                break;
            }

            case READ_MEM: {
                
            }

            case WRITE_MEM: {
                
            }

            case 1:
                codigoOperacion = paquete_operacion->codigo_operacion;
                break;

            default:
                log_warning(logger, "Operación desconocida recibida: %d", paquete_operacion->codigo_operacion);
                break;
        }
    }
    sem_post(&sem_fin_memoria);
    log_warning(logger, "Se desconectó la CPU");
    return NULL;
}

void actualizar_contexto(int pid, int tid, t_registros_cpu* reg){
    pthread_mutex_lock(&mutex_lista_contextos_pids);
    t_contexto_tid *contexto = obtener_contexto_tid(pid, tid);
    if (contexto != NULL)
    {
        contexto->registros->PC = reg->PC;
        contexto->registros->AX = reg->AX;
        contexto->registros->BX = reg->BX;
        contexto->registros->CX = reg->CX;
        contexto->registros->DX = reg->DX;
        contexto->registros->EX = reg->EX;
        contexto->registros->HX = reg->HX;
    }
    pthread_mutex_unlock(&mutex_lista_contextos_pids);
}



// Se utiliza al inicializar el contexto del tid 0 tras crear un nuevo proceso y cuando viene un hilo con tid N a ejecutar
t_contexto_tid* inicializar_contexto_tid(t_contexto_pid* cont,int tid){
    t_contexto_tid* contexto=malloc(sizeof(t_contexto_tid));
    contexto->registros= malloc(sizeof(t_registros_cpu));
    contexto->pid=cont->pid;
    contexto->tid = tid;
    contexto->registros->AX = 0;
    contexto->registros->BX = 0;
    contexto->registros->CX = 0;
    contexto->registros->DX = 0;
    contexto->registros->EX = 0;
    contexto->registros->FX = 0;
    contexto->registros->GX = 0;
    contexto->registros->HX = 0;
    contexto->registros->PC = 0;

    list_add(cont->contextos_tids,contexto);
    return contexto;
}

// Se debe usar despues de un PROCESS_CREATE y para el proceso inicial de la CPU
t_contexto_pid*inicializar_contexto_pid(int pid,uint32_t base, uint32_t limite){ 
    t_contexto_pid*nuevo_contexto=malloc(sizeof(t_contexto_pid));
    nuevo_contexto->pid=pid;
    nuevo_contexto->base=base;
    nuevo_contexto->limite=limite;

    nuevo_contexto->contextos_tids=list_create();
// Paso como segundo parámetro el 0 ya que el proceso está siendo inicializado, y al iniciarse si o si tiene que tener un hilo
    t_contexto_tid* contexto_tid_0=inicializar_contexto_tid(nuevo_contexto,0); 
    if (contexto_tid_0!=NULL){

        pthread_mutex_lock(&mutex_lista_contextos_pids);
        list_add(lista_contextos_pids,nuevo_contexto);
        pthread_mutex_unlock(&mutex_lista_contextos_pids);

        return nuevo_contexto;
    }
    else{
        printf("NO SE PUDO INICIALIZAR EL CONTEXTO DEL PID %d\n",pid);
    }
    return NULL;
    
}

t_contexto_tid*obtener_contexto_tid(int pid, int tid){ // hay que usar mutex cada vez que se usa esta funcion
    //pthread_mutex_lock(&mutex_lista_contextos_pids);
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        
        t_contexto_pid*cont_actual=(t_contexto_pid*)list_get(lista_contextos_pids,i);
        t_list*contextos_tids=cont_actual->contextos_tids;
        if (pid == cont_actual->pid && esta_tid_en_lista(tid,contextos_tids)){
            //pthread_mutex_unlock(&mutex_lista_contextos_pids);
            return obtener_tid_en_lista(tid,contextos_tids);
        }
 
    }
    //pthread_mutex_unlock(&mutex_lista_contextos_pids);
    return NULL;
}

void remover_contexto_pid_lista(t_contexto_pid* contexto){
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=(t_contexto_pid*)list_get(lista_contextos_pids,i);
        if (contexto->pid == cont_actual->pid){
            list_remove(lista_contextos_pids,i);
            break;
        }
            

    }
}

t_contexto_pid* obtener_contexto_pid(int pid){ // al usarla hay q meterle mutex x la lista de contextos
    pthread_mutex_lock(&mutex_lista_contextos_pids);
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=(t_contexto_pid*)list_get(lista_contextos_pids,i);
        if (pid == cont_actual->pid){
            pthread_mutex_unlock(&mutex_lista_contextos_pids);
            return cont_actual;
        }
            
    }
    pthread_mutex_unlock(&mutex_lista_contextos_pids);
    return NULL;
}


void remover_contexto_tid_lista(t_contexto_tid*contexto,t_list*lista){
    for (int i = 0; i < list_size(lista); i++){
        t_contexto_tid*cont_actual=(t_contexto_tid*)list_get(lista,i);
        if (contexto->tid == cont_actual->tid)
            list_remove(lista,i);

    }
}


bool esta_tid_en_lista(int tid,t_list*contextos_tids){
    for (int i = 0; i < list_size(contextos_tids); i++){
        t_contexto_tid*cont_actual=(t_contexto_tid*)list_get(contextos_tids,i);
        if (cont_actual->tid==tid)
            return true;

    }
    return false;
}

t_contexto_tid* obtener_tid_en_lista(int tid,t_list*contextos_tids){
    for (int i = 0; i < list_size(contextos_tids); i++){
        t_contexto_tid*cont_actual=(t_contexto_tid*)list_get(contextos_tids,i);
        if (cont_actual->tid==tid)
            return cont_actual;

    }
    return NULL;
}


void liberar_contexto_tid(t_contexto_pid *contexto_pid,t_contexto_tid*contexto_tid){
    if(contexto_tid){
        remover_contexto_tid_lista(contexto_tid,contexto_pid->contextos_tids);
        free(contexto_tid->registros);
        free(contexto_tid);
    }
}

void liberar_contexto_pid(t_contexto_pid *contexto_pid){
    if(contexto_pid){
        
        list_destroy_and_destroy_elements(contexto_pid->contextos_tids,(void*)liberar_contexto_tid);
        free(contexto_pid);
    }
}

void liberar_lista_contextos(){
    list_destroy_and_destroy_elements(lista_contextos_pids,(void*)liberar_contexto_pid);
}


