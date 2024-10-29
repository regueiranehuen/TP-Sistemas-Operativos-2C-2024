#include "includes/memSist.h"
#include "includes/server.h"

pthread_mutex_t mutex_lista_instruccion;

int longitud_maxima=200;
int parametros_maximos=6;
int instrucciones_maximas=200;

void cargar_instrucciones_desde_archivo(char* nombre_archivo, int pid, int tid){
    printf("nombre del archivo: %s\n",nombre_archivo);

    const char *ruta_relativa = nombre_archivo;
    char*ruta_absoluta = obtener_ruta_absoluta(ruta_relativa);

    FILE* archivo = fopen(ruta_absoluta, "r");
    
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
    
    int indice_instruccion = 0;
    char linea[longitud_maxima];

    while (fgets(linea, longitud_maxima, archivo) != NULL && indice_instruccion < instrucciones_maximas) {
        t_instruccion_tid_pid* instruccion_tid_pid = malloc(sizeof(t_instruccion_tid_pid));
        instruccion_tid_pid->pid=pid;
        instruccion_tid_pid->tid=tid;
        instruccion_tid_pid->pc = 0;
        //instruccion_tid_pid->pc;
        instruccion_tid_pid->instrucciones = malloc(sizeof(t_instruccion));
        log_info(logger,"reservé espacio para instrucciones!");
        //t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        char* token = strtok(linea, " \t\n");
        int param_count = 0;

        while (token != NULL && param_count < parametros_maximos) {
            switch (param_count) {
                case 0:
                    instruccion_tid_pid->instrucciones->parametros1 = "";
                    instruccion_tid_pid->instrucciones->parametros1 = strdup(token);
                    log_info(logger,"%s",instruccion_tid_pid->instrucciones->parametros1);
                    break;
                case 1:
                    instruccion_tid_pid->instrucciones->parametros2 = "";
                    instruccion_tid_pid->instrucciones->parametros2 = strdup(token);
                    log_info(logger,"%s",instruccion_tid_pid->instrucciones->parametros2);
                    break;
                case 2:
                    instruccion_tid_pid->instrucciones->parametros3 = "";
                    instruccion_tid_pid->instrucciones->parametros3 = strdup(token);
                    log_info(logger,"%s",instruccion_tid_pid->instrucciones->parametros3);
                    break;
                case 3:
                    instruccion_tid_pid->instrucciones->parametros4 = "";
                    instruccion_tid_pid->instrucciones->parametros4 = strdup(token);
                    log_info(logger,"%s",instruccion_tid_pid->instrucciones->parametros4);
                    break;
                case 4:
                    instruccion_tid_pid->instrucciones->parametros5 = "";
                    instruccion_tid_pid->instrucciones->parametros5 = strdup(token);
                    log_info(logger,"%s",instruccion_tid_pid->instrucciones->parametros5);
                    break;
                case 5:
                    instruccion_tid_pid->instrucciones->parametros6 = "";
                    instruccion_tid_pid->instrucciones->parametros6 = strdup(token);
                    log_info(logger,"%s",instruccion_tid_pid->instrucciones->parametros6);
                    break;
                default:
                    break;
            }
            token = strtok(NULL, " \t\n");
            param_count++;
        }
        
        //list_add(lista_instrucciones_tid_pid,instruccion_tid_pid);


        pthread_mutex_lock(&mutex_lista_instruccion);
        list_add(lista_instrucciones_tid_pid,instruccion_tid_pid);
        indice_instruccion++;
        instruccion_tid_pid->pc+=1;
        pthread_mutex_unlock(&mutex_lista_instruccion);        
        
    }
    fclose(archivo);
}



void finalizar_hilo(int tid, int pid) {
    for (int i = 0; i < list_size(lista_instrucciones_tid_pid); i++) {
        pthread_mutex_lock(&mutex_lista_instruccion);
        t_instruccion_tid_pid* actual = list_get(lista_instrucciones_tid_pid, i);
        pthread_mutex_unlock(&mutex_lista_instruccion);
        if (actual->pid == pid && actual->tid == tid) {
            pthread_mutex_lock(&mutex_lista_instruccion);
            list_remove(lista_instrucciones_tid_pid, i);
            liberar_instruccion(actual);
            pthread_mutex_unlock(&mutex_lista_instruccion);
            
            
            i--; // Decrementa i para no saltar el siguiente elemento
        }
    }
    
    t_contexto_pid* contexto_pid = obtener_contexto_pid(pid);
    t_contexto_tid* contexto_tid = obtener_contexto_tid(pid,tid);
    eliminar_elemento_por_tid(contexto_tid->tid, contexto_pid->contextos_tids);
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


