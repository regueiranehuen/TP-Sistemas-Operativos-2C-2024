#include "memSist.h"

void cargar_instrucciones_desde_archivo(char* nombre_archivo,  uint32_t pid) {
    //log_info(log_memoria, "log 1 %s", nombre_archivo);
    //size_t path_len = strlen(path_instrucciones) + strlen(nombre_archivo) + 1;
    //char* path_compl = malloc(path_len);    
    //strcpy(path_compl, path_instrucciones);
    //strcat(path_compl, nombre_archivo); 
    //log_info(log_memoria, "log 2");
    FILE* archivo = fopen(nombre_archivo, "r");
    
    // Liberar path_compl ya que no se necesita más
    //free(path_compl);
    //log_info(log_memoria, "log 3");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
    //log_info(log_memoria, "log 4");
    int indice_instruccion = 0;
    char linea[longitud_maxima];
    //log_info(log_memoria, "log 5");

    while (fgets(linea, longitud_maxima, archivo) != NULL && indice_instruccion < instrucciones_maximas) {
        //log_info(log_memoria, "log 6");
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        char* token = strtok(linea, " \t\n");
        int param_count = 0;

        while (token != NULL && param_count < parametros_maximos) {
            switch (param_count) {
                case 0:
                    instruccion->parametros1 = "";
                    instruccion->parametros1 = strdup(token);
                    //log_info(log_memoria, "case instruccion %s", instruccion->parametros1);
                    break;
                case 1:
                    instruccion->parametros2 = "";
                    instruccion->parametros2 = strdup(token);
                    //log_info(log_memoria, "parametro 1 %s", instruccion->parametros2);
                    break;
                case 2:
                    instruccion->parametros3 = "";
                    instruccion->parametros3 = strdup(token);
                    //log_info(log_memoria, "parametro 2 %s", instruccion->parametros3);
                    break;
                case 3:
                    instruccion->parametros4 = "";
                    instruccion->parametros4 = strdup(token);
                    //log_info(log_memoria, "parametro 3 %s", instruccion->parametros4);
                    break;
                case 4:
                    instruccion->parametros5 = "";
                    instruccion->parametros5 = strdup(token);
                    //log_info(log_memoria, "parametro 4 %s", instruccion->parametros5);
                    break;
                case 5:
                    instruccion->parametros6 = "";
                    instruccion->parametros6 = strdup(token);
                    //log_info(log_memoria, "parametro 5 %s", instruccion->parametros6);
                    break;
                default:
                    break;
            }
            token = strtok(NULL, " \t\n");
            param_count++;
        }

        //instrucciones[indice_instruccion] = instruccion;
        list_add(listas_instrucciones[pid-1],instruccion);
        indice_instruccion++;
        
    }

    fclose(archivo);
    
}