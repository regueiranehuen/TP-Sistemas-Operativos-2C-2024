#include "includes/commCpu.h"

void recibir_cpu(int SOCKET_CLIENTE_CPU) {
    int retardo_respuesta = config_get_int_value(config,"RETARDO_RESPUESTA");
    int codigoOperacion = 0;
    while (codigoOperacion != -1) {
        op_code codOperacion = recibir_operacion(SOCKET_CLIENTE_CPU);
        usleep(retardo_respuesta * 1000);  // Aplicar retardo configurado

        switch (codOperacion) {
            case OBTENER_CONTEXTO_TID: {
                t_2_enteros *solicitud = recibir_2_enteros(SOCKET_CLIENTE_CPU);  // Recibe PID y TID
                uint32_t pid = solicitud->entero1;
                uint32_t tid = solicitud->entero2;
                free(solicitud);

                t_contexto_tid*contexto=obtener_contexto_tid(pid,tid);
                enviar_contexto_tid(SOCKET_CLIENTE_CPU,contexto);
                log_info(logger, "Enviado contexto para PID: %d, TID: %d", pid, tid);
                
                

                //liberar_contexto_pid(contexto);
                break;
            }

            case ACTUALIZAR_CONTEXTO: { // AUXILIO
                t_contexto *contexto = recibir_contexto(SOCKET_CLIENTE_CPU);  // Recibe nuevo contexto
                actualizar_contexto(contexto); // Falta crear esta funcion? O se usa enviar_contexto_a_memoria???
                //enviar_contexto_a_memoria(contexto);
                enviar_codop(SOCKET_CLIENTE_CPU, ACTUALIZACION_OK);
                log_info(logger, "Contexto actualizado para TID: %d", contexto->tid);
                free(contexto);
                break;
            }

            case OBTENER_INSTRUCCION: {
                t_2_enteros *solicitud = recibir_2_enteros(SOCKET_CLIENTE_CPU);  // PID y PC
                uint32_t tid = solicitud->entero1;
                uint32_t pc = solicitud->entero2;
                free(solicitud);

                char *instruccion = obtener_instruccion(tid, pc);
                enviar_instruccion(SOCKET_CLIENTE_CPU, instruccion, OBTENER_INSTRUCCION);
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
                memcpy(datos, ESPACIO_USUARIO + direccion_fisica, tam_a_leer);
                enviar_datos(SOCKET_CLIENTE_CPU, datos, tam_a_leer);
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
                codigoOperacion = codOperacion;
                break;

            default:
                log_warning(logger, "Operación desconocida recibida: %d", codOperacion);
                break;
        }
    }

    log_warning(logger, "Se desconectó la CPU");
}


bool existe_contexto_pid(int pid){
    for (int i = 0; i< list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=list_get(lista_contextos_pids,i);
        if (cont_actual->pid==pid)
            return true;
    }
    return false;
}

t_contexto_pid*inicializar_contexto_pid(int pid){
    t_contexto_pid*nuevo_contexto=malloc(sizeof(t_contexto_pid));
    nuevo_contexto->pid=pid;
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

t_contexto_tid*obtener_contexto_tid(int pid, int tid){
    //wait
    for (int i = 0; i < list_size(lista_contextos_pids); i++){
        t_contexto_pid*cont_actual=(t_contexto_pid*)list_get(lista_contextos_pids,i);
        t_list*contextos_tids=cont_actual->contextos_tids;
        if (pid == cont_actual->pid && esta_tid_en_lista(tid,contextos_tids)){
            //signal
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


