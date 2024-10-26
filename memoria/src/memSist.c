#include "includes/memSist.h"
#include "includes/server.h"


int longitud_maxima=200;
int parametros_maximos=6;
int instrucciones_maximas=200;


void cargar_instrucciones_desde_archivo(char* nombre_archivo, int pid, int tid){
    FILE* archivo = fopen(nombre_archivo, "r");
    
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
        //t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        char* token = strtok(linea, " \t\n");
        int param_count = 0;

        while (token != NULL && param_count < parametros_maximos) {
            switch (param_count) {
                case 0:
                    instruccion_tid_pid->instrucciones->parametros1 = "";
                    instruccion_tid_pid->instrucciones->parametros1 = strdup(token);
                    break;
                case 1:
                    instruccion_tid_pid->instrucciones->parametros2 = "";
                    instruccion_tid_pid->instrucciones->parametros2 = strdup(token);
                    break;
                case 2:
                    instruccion_tid_pid->instrucciones->parametros3 = "";
                    instruccion_tid_pid->instrucciones->parametros3 = strdup(token);
                    break;
                case 3:
                    instruccion_tid_pid->instrucciones->parametros4 = "";
                    instruccion_tid_pid->instrucciones->parametros4 = strdup(token);
                    break;
                case 4:
                    instruccion_tid_pid->instrucciones->parametros5 = "";
                    instruccion_tid_pid->instrucciones->parametros5 = strdup(token);
                    break;
                case 5:
                    instruccion_tid_pid->instrucciones->parametros6 = "";
                    instruccion_tid_pid->instrucciones->parametros6 = strdup(token);
                    break;
                default:
                    break;
            }
            token = strtok(NULL, " \t\n");
            param_count++;
        }
        
        //list_add(lista_instrucciones_tid_pid,instruccion_tid_pid);



        list_add(lista_instrucciones_tid_pid,instruccion_tid_pid);
        indice_instruccion++;
        instruccion_tid_pid->pc+=1;
        
    }
    fclose(archivo);
}


t_instruccion* obtener_instruccion(int tid, int pid,uint32_t pc){
    for (int i = 0; i < list_size(lista_instrucciones_tid_pid); i++){
        t_instruccion_tid_pid*actual = (t_instruccion_tid_pid*)list_get(lista_instrucciones_tid_pid,i);
        if (actual->pid == pid && actual->tid == tid && actual->pc == pc){
            return actual->instrucciones;
        }
    }
    return NULL;
}


void copiarBytes(uint32_t tamanio, t_contexto_pid *contexto) {

    uint32_t valorBase = contexto->base;
    uint32_t valorLimite= contexto->limite;

    char *origen = (char*)(uintptr_t)valorBase;
    char *destino = (char*)(uintptr_t)valorLimite;

    memcpy(destino, origen, tamanio);
}


