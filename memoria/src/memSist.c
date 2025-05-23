#include "includes/memSist.h"
#include "includes/server.h"

pthread_mutex_t mutex_lista_instruccion;

int longitud_maxima=200;
int parametros_maximos=6;
int instrucciones_maximas=200;

char* limpiar_token(char* token) {
    size_t len = strlen(token);
    if (len > 0 && (token[len - 1] == '\n' || token[len - 1] == '\r')) {
        token[len - 1] = '\0';
    }
    return token;
}

void cargar_instrucciones_desde_archivo(char* nombre_archivo, int pid, int tid){
    

    char* path_instrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
    char* path_instrucciones_aux = malloc(strlen(path_instrucciones)+strlen(nombre_archivo) + 1);
    

    snprintf(path_instrucciones_aux,strlen(path_instrucciones)+strlen(nombre_archivo) + 1,"%s%s",path_instrucciones,nombre_archivo);
    
    FILE* archivo = fopen(path_instrucciones_aux, "r");
    
    free(path_instrucciones_aux);

    if (archivo == NULL) {
        pthread_mutex_lock(&mutex_logs);
        perror("Error al abrir el archivo");
        pthread_mutex_unlock(&mutex_logs);
        exit(EXIT_FAILURE);
    }
    
    int indice_instruccion = 0;
    char linea[longitud_maxima];

    while (fgets(linea, longitud_maxima, archivo) != NULL && indice_instruccion < instrucciones_maximas) {
        t_instruccion_tid_pid* instruccion_tid_pid = malloc(sizeof(t_instruccion_tid_pid));
        instruccion_tid_pid->pid=pid;
        instruccion_tid_pid->tid=tid;
        instruccion_tid_pid->pc = indice_instruccion;

        instruccion_tid_pid->instrucciones = malloc(sizeof(t_instruccion));
        //t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        char* token = strtok(linea, " \t\n");
        int param_count = 0;

        while (token != NULL && param_count < parametros_maximos) {
            switch (param_count) {
                case 0:
                    instruccion_tid_pid->instrucciones->parametros1 = "";
                    instruccion_tid_pid->instrucciones->parametros1 = strdup(limpiar_token(token));
                    
                    break;
                case 1:
                    instruccion_tid_pid->instrucciones->parametros2 = "";
                    instruccion_tid_pid->instrucciones->parametros2 = strdup(limpiar_token(token));
                    
                    break;
                case 2:
                    instruccion_tid_pid->instrucciones->parametros3 = "";
                    instruccion_tid_pid->instrucciones->parametros3 = strdup(limpiar_token(token));

                    break;
                case 3:
                    instruccion_tid_pid->instrucciones->parametros4 = "";
                    instruccion_tid_pid->instrucciones->parametros4 = strdup(limpiar_token(token));
                    
                    break;
                default:
                    break;
            }
            token = strtok(NULL, " \t\n");
            param_count++;
        }
        //list_add(lista_instrucciones_tid_pid,instruccion_tid_pid);
        inicializar_resto_parametros(param_count,instruccion_tid_pid);

        pthread_mutex_lock(&mutex_lista_instruccion);
        list_add(lista_instrucciones_tid_pid,instruccion_tid_pid);
        indice_instruccion++;
        pthread_mutex_unlock(&mutex_lista_instruccion);        
        
    }
    fclose(archivo);
    
}

void inicializar_resto_parametros(int cant_param, t_instruccion_tid_pid *instruccion)
{
    switch (cant_param)
    {
    case 1:
        instruccion->instrucciones->parametros2 = "";
        instruccion->instrucciones->parametros3 = "";
        instruccion->instrucciones->parametros4 = "";
        break;
    case 2:
        instruccion->instrucciones->parametros3 = "";
        instruccion->instrucciones->parametros4 = "";
        break;
    case 3:
        instruccion->instrucciones->parametros4 = "";
        break;
    default:
        break;
    }
}


void enviar_instruccion(int conexion, t_instruccion *instruccion_nueva, op_code codop)
{
    // no olvidar el codigo de operacion

    t_buffer *buffer = malloc(sizeof(t_buffer));
    // buffer->size=0;

    int l1 = 0;
    int l2 = 0;
    int l3 = 0;
    int l4 = 0;

    // log_info(logger,"parametros 4:%s",instruccion_nueva->parametros4);

    l1 = strlen(instruccion_nueva->parametros1) + 1;
    l2 = strlen(instruccion_nueva->parametros2) + 1;
    l3 = strlen(instruccion_nueva->parametros3) + 1;
    l4 = strlen(instruccion_nueva->parametros4) + 1;
    buffer->size = sizeof(int) * 4 + l1 + l2 + l3 + l4;


    buffer->stream = malloc(buffer->size);
    void *stream = buffer->stream;

    memcpy(stream, &l1, sizeof(int));
    stream += sizeof(int);
    memcpy(stream, instruccion_nueva->parametros1, l1);
    stream += l1;
    memcpy(stream, &l2, sizeof(int));
    stream += sizeof(int);
    memcpy(stream, instruccion_nueva->parametros2, l2);
    stream += l2;
    memcpy(stream, &l3, sizeof(int));
    stream += sizeof(int);
    memcpy(stream, instruccion_nueva->parametros3, l3);
    stream += l3;
    memcpy(stream, &l4, sizeof(int));
    stream += sizeof(int);
    memcpy(stream, instruccion_nueva->parametros4, l4);
    stream += l4;

    send_paquete_op_code(conexion, buffer, INSTRUCCION_OBTENIDA);
}

void finalizar_hilo(int tid, int pid) {
    pthread_mutex_lock(&mutex_lista_instruccion);
    for (int i = 0; i < list_size(lista_instrucciones_tid_pid); i++) {
        t_instruccion_tid_pid* actual = list_get(lista_instrucciones_tid_pid, i);
        
        if (actual->pid == pid && actual->tid == tid) {

            list_remove(lista_instrucciones_tid_pid, i);
            liberar_instruccion(actual);
      
            i--; // Decrementa i para no saltar el siguiente elemento
        }
    }
    pthread_mutex_unlock(&mutex_lista_instruccion);
    pthread_mutex_lock(&mutex_lista_contextos_pids);
    t_contexto_pid* contexto_pid = obtener_contexto_pid(pid);


    t_contexto_tid* contexto_tid = obtener_contexto_tid(pid,tid);

    
    if (contexto_tid == NULL){
        pthread_mutex_lock(&mutex_logs);
        log_info(logger,"Contexto tid: NULL");
        pthread_mutex_unlock(&mutex_logs);
    }


    list_remove_element(contexto_pid->contextos_tids,contexto_tid);
    free(contexto_tid->registros);
    free(contexto_tid);
    

    //eliminar_elemento_por_tid(contexto_tid->tid, contexto_pid->contextos_tids);
    pthread_mutex_unlock(&mutex_lista_contextos_pids);
}

void eliminar_elemento_por_tid(int tid, t_list* contextos_tids) {

    for (int i = 0; i < list_size(contextos_tids); i++) {
        t_contexto_tid* actual = list_get(contextos_tids, i);
        if (actual->tid == tid) {
            // Eliminar el elemento de la lista y liberar su memoria
            list_remove(contextos_tids, i); // Libera la memoria del elemento eliminado
            free(actual->registros);
            free(actual);

            break; // Salir del bucle después de eliminar el primer elemento encontrado
        }
    }   
}



t_instruccion* obtener_instruccion(int tid, int pid,uint32_t pc){
    pthread_mutex_lock(&mutex_lista_instruccion);
    for (int i = 0; i < list_size(lista_instrucciones_tid_pid); i++){
        t_instruccion_tid_pid*actual = (t_instruccion_tid_pid*)list_get(lista_instrucciones_tid_pid,i);
        if (actual->pid == pid && actual->tid == tid && actual->pc == pc){
            pthread_mutex_unlock(&mutex_lista_instruccion);
            return actual->instrucciones;
        }
    }
    pthread_mutex_unlock(&mutex_lista_instruccion);
    return NULL;
}


void copiarBytes(uint32_t tamanio, t_contexto_pid *contexto) {

    uint32_t valorBase = contexto->base;
    uint32_t valorLimite= contexto->limite;

    char *origen = (char*)(uintptr_t)valorBase;
    char *destino = (char*)(uintptr_t)valorLimite;

    memcpy(destino, origen, tamanio);
}