#include "includes/memSist.h"
#include "includes/server.h"

void cargar_instrucciones_desde_archivo(char* nombre_archivo,  uint32_t pid) {
    FILE* archivo = fopen(nombre_archivo, "r");
    
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
    int indice_instruccion = 0;
    char linea[longitud_maxima];

    while (fgets(linea, longitud_maxima, archivo) != NULL && indice_instruccion < instrucciones_maximas) {
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        char* token = strtok(linea, " \t\n");
        int param_count = 0;

        while (token != NULL && param_count < parametros_maximos) {
            switch (param_count) {
                case 0:
                    instruccion->parametros1 = "";
                    instruccion->parametros1 = strdup(token);
                    break;
                case 1:
                    instruccion->parametros2 = "";
                    instruccion->parametros2 = strdup(token);
                    break;
                case 2:
                    instruccion->parametros3 = "";
                    instruccion->parametros3 = strdup(token);
                    break;
                case 3:
                    instruccion->parametros4 = "";
                    instruccion->parametros4 = strdup(token);
                    break;
                case 4:
                    instruccion->parametros5 = "";
                    instruccion->parametros5 = strdup(token);
                    break;
                case 5:
                    instruccion->parametros6 = "";
                    instruccion->parametros6 = strdup(token);
                    break;
                default:
                    break;
            }
            token = strtok(NULL, " \t\n");
            param_count++;
        }

        list_add(listas_instrucciones[pid-1],instruccion);
        indice_instruccion++;
        
    }
    fclose(archivo);
}

void copiarBytes(uint32_t tamanio, t_contexto_pid *contexto) {

    uint32_t valorBase = contexto->base;
    uint32_t valorLimite= contexto->limite;

    char *origen = (char*)(uintptr_t)valorBase;
    char *destino = (char*)(uintptr_t)valorLimite;

    memcpy(destino, origen, tamanio);
}


