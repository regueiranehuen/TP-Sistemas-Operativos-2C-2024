#include "includes/commCpu.h"

void recibir_cpu(int SOCKET_CLIENTE_CPU) {
    int retardo_respuesta = config_get_int_value(config,"RETARDO_RESPUESTA");
    int codigoOperacion = 0;
    while (codigoOperacion != -1) {
        
        t_paquete*paquete_operacion = recibir_paquete_op_code(SOCKET_CLIENTE_CPU);
        usleep(retardo_respuesta * 1000);  // Aplicar retardo configurado

        switch (paquete_operacion->codigo_operacion) {
            case OBTENER_CONTEXTO_TID: {
                t_tid_pid *info = recepcionar_tid_pid_op_code(paquete_operacion);  // Recibe PID y TID
                int pid = info->pid;
                int tid = info->tid;

                log_info(logger, "## Contexto solicitado - (PID:TID) - (%d:%d)",pid,tid);

                t_contexto_tid*contexto_tid=obtener_contexto_tid(pid,tid);

                // Si el contexto no existe, lo creamos y lo metemos en la lista de contextos de tid del contexto del pid
                if (contexto_tid==NULL){
                    t_contexto_pid*contexto_pid = obtener_contexto_pid(pid);


                    inicializar_contexto_tid(contexto_pid,tid);
                }

                enviar_contexto_tid(SOCKET_CLIENTE_CPU,contexto_tid);
                log_info(logger, "Enviado contexto para PID: %d, TID: %d", pid, tid);
                

                break;
            }

            case OBTENER_CONTEXTO_PID:{ 
                int pid_obtencion = recepcionar_entero_paquete(paquete_operacion);
                t_contexto_pid*contextoPid = obtener_contexto_pid(pid_obtencion);

                if (contextoPid == NULL){
                    log_error(logger, "No se encontro el contexto del pid %d", pid_obtencion);
                    enviar_paquete_op_code(SOCKET_CLIENTE_CPU,CONTEXTO_PID_INEXISTENTE);
                    break;
                }

                enviar_contexto_pid(SOCKET_CLIENTE_CPU,contextoPid);
                break;
            }


            case ACTUALIZAR_CONTEXTO_TID:{
                t_tid_pid *info = recepcionar_tid_pid_op_code(paquete_operacion);  // Recibe PID y TID
                int pid = info->pid;
                int tid = info->tid;
                t_registros_cpu* registros_a_actualizar = recepcionar_registros(paquete_operacion);
                actualizar_contexto(pid,tid,registros_a_actualizar);
                enviar_codop(SOCKET_CLIENTE_CPU, ACTUALIZACION_OK);
                free(info);
                log_info(logger, "## Contexto actualizado - (PID:TID) - (%d:%d)", pid,tid);
                break;
            }

            case OBTENER_INSTRUCCION: {
                t_2_enteros *solicitud = recibir_2_enteros(SOCKET_CLIENTE_CPU);  // PID y PC
                uint32_t tid = solicitud->entero1;
                uint32_t pc = solicitud->entero2;
                free(solicitud);

                char *instruccion = obtener_instruccion(tid, pc);
                enviar_instruccion(SOCKET_CLIENTE_CPU, instruccion, OBTENER_INSTRUCCION); // REVISAR
                log_info(logger, "Instrucción enviada para PID: %d, PC: %d", tid, pc);
                free(instruccion);
                break;
            }

            case READ_MEM: {
                t_2_enteros *solicitud = recibir_2_enteros(SOCKET_CLIENTE_CPU);  // Dirección física y tamaño
                uint32_t direccion_fisica = solicitud->entero1;
                uint32_t tam_a_leer = solicitud->entero2;
                free(solicitud);

                void *datos = malloc(tam_a_leer);
                memcpy(datos, ESPACIO_USUARIO + direccion_fisica, tam_a_leer); // REVISAR
                enviar_datos(SOCKET_CLIENTE_CPU, datos, tam_a_leer); // REVISAR
                log_info(logger, "Memoria leída desde %d, tamaño %d", direccion_fisica, tam_a_leer);
                free(datos);
                break;
            }

            case WRITE_MEM: {
                t_string_2enteros *escritura = recibir_string_2enteros(SOCKET_CLIENTE_CPU); // estaba mal nombrada la función acá
                uint32_t direccion_fisica = escritura->entero1;
                uint32_t tam_a_escribir = escritura->entero2;
                char *contenido = escritura->string;

                memcpy(ESPACIO_USUARIO + direccion_fisica, contenido, tam_a_escribir);
                enviar_codop(SOCKET_CLIENTE_CPU, WRITE_OK);
                log_info(logger, "Escritos %d bytes en dirección %d", tam_a_escribir, direccion_fisica);
                free(escritura);
                break;
            }

            case 1:
                codigoOperacion = paquete_operacion->codigo_operacion;
                break;

            default:
                log_warning(logger, "Operación desconocida recibida: %d", paquete_operacion->codigo_operacion);
                break;
        }
    }

    log_warning(logger, "Se desconectó la CPU");
}

void actualizar_contexto(int pid, int tid, t_registros_cpu* reg){
    t_contexto_tid*contexto=obtener_contexto_tid(pid,tid);
    contexto->registros->PC=reg->PC;
    contexto->registros->AX=reg->AX;
    contexto->registros->BX=reg->BX;
    contexto->registros->CX=reg->CX;
    contexto->registros->DX=reg->DX;
    contexto->registros->EX=reg->EX;
    contexto->registros->HX=reg->HX;
}

void actualizar_contexto_program_counter(int pid, int tid, uint32_t pc){
    t_contexto_tid*contexto=obtener_contexto_tid(pid,tid);
    contexto->registros->PC=pc;
}
void actualizar_contexto_reg(int pid, int tid, t_registros_cpu* reg){
    t_contexto_tid*contexto=obtener_contexto_tid(pid,tid);
    contexto->registros->PC=reg->PC;
    contexto->registros->AX=reg->AX;
    contexto->registros->BX=reg->BX;
    contexto->registros->CX=reg->CX;
    contexto->registros->DX=reg->DX;
    contexto->registros->EX=reg->EX;
    contexto->registros->HX=reg->HX;
}



bool existe_contexto_pid(int pid){
    for (int i = 0; i< list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=list_get(lista_contextos_pids,i);
        if (cont_actual->pid==pid)
            return true;
    }
    return false;
}

t_contexto_pid*inicializar_contexto_pid(int pid,uint32_t base, uint32_t limite){
    t_contexto_pid*nuevo_contexto=malloc(sizeof(t_contexto_pid));
    nuevo_contexto->pid=pid;
    nuevo_contexto->base=base;
    nuevo_contexto->limite=limite;

    nuevo_contexto->contextos_tids=list_create();
// Paso como segundo parámetro el 0 ya que el proceso está siendo inicializado, y al iniciarse si o si tiene que tener un hilo
    t_contexto_tid* contexto_tid_0=inicializar_contexto_tid(nuevo_contexto,0); 
    if (contexto_tid_0!=NULL){
        list_add(lista_contextos_pids,nuevo_contexto);
        return nuevo_contexto;
    }
    else{
        printf("NO SE PUDO INICIALIZAR EL CONTEXTO DEL PID %d\n",pid);
    }
    return NULL;
    
}

t_contexto_tid*obtener_contexto_tid(int pid, int tid){ // hay que usar mutex
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=(t_contexto_pid*)list_get(lista_contextos_pids,i);
        t_list*contextos_tids=cont_actual->contextos_tids;
        if (pid == cont_actual->pid && esta_tid_en_lista(tid,contextos_tids)){
            return obtener_tid_en_lista(tid,contextos_tids);
        }
        
    }
    return NULL;
}

void remover_contexto_pid_lista(t_contexto_pid* contexto){
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=(t_contexto_pid*)list_get(lista_contextos_pids,i);
        if (contexto->pid == cont_actual->pid)
            list_remove(lista_contextos_pids,i);

    }
}

t_contexto_pid* obtener_contexto_pid(int pid){
    //wait
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=(t_contexto_pid*)list_get(lista_contextos_pids,i);
        if (pid == cont_actual->pid){
            //signal
            return cont_actual;
        }
            
    }

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

void liberar_lisa_contextos(){
    list_destroy_and_destroy_elements(lista_contextos_pids,(void*)liberar_contexto_pid);
}


