#include "auxiliares.h"

void inicializar_semaforos(){
    sem_init(&sem_ok_o_interrupcion,0,0);
    sem_init(&sem_finalizacion_cpu,0,0);
    sem_init(&sem_ciclo_instruccion,0,0);
}

void inicializar_estructuras() {
    log_cpu = log_create("CPU.log", "tp", true, LOG_LEVEL_TRACE);
    leer_config("CPU.config");
    
    
    sockets_cpu = malloc(sizeof(t_sockets_cpu));


    log_info(log_cpu, "Estructuras inicializadas");
}


void inicializar_mutex(){
    pthread_mutex_init(&mutex_contextos_exec,NULL);
    pthread_mutex_init(&mutex_interrupt,NULL);
}



void destruir_mutex(){
    pthread_mutex_destroy(&mutex_contextos_exec);
    pthread_mutex_destroy(&mutex_interrupt);
}

void destruir_semaforos(){
    sem_destroy(&sem_finalizacion_cpu);
    sem_destroy(&sem_finalizacion_cpu);
    sem_destroy(&sem_ciclo_instruccion);
}

void liberarMemoria(t_sockets_cpu * sockets,t_log* log, t_config* config){

    if (sockets == NULL || sockets->socket_memoria == -1 ||
        sockets->socket_servidor == NULL || 
        sockets->socket_servidor->socket_cliente_Dispatch == -1 || 
        sockets->socket_servidor->socket_cliente_Interrupt == -1) {

        log_info(log, "Error en los sockets de cpu");
        }
        else{
    close(sockets->socket_memoria);
    close(sockets->socket_servidor->socket_servidor_Dispatch);
    close(sockets->socket_servidor->socket_servidor_Interrupt);
    close(sockets->socket_servidor->socket_cliente_Dispatch);
    close(sockets->socket_servidor->socket_cliente_Interrupt);

    }
    if (sockets != NULL) {
            if (sockets->socket_servidor != NULL) {
                free(sockets->socket_servidor);
            }
            free(sockets);
        }
    config_destroy(config);
    log_destroy(log);
}

void terminar_programa() {
    liberarMemoria(sockets_cpu, log_cpu, config); 
    destruir_mutex();
    destruir_semaforos();
    log_info(log_cpu, "Estructuras liberadas");
}

t_instruccion *recepcionar_instruccion(t_paquete *paquete)
{
    t_instruccion *instruccion_nueva = malloc(sizeof(t_instruccion));

    void *stream = paquete->buffer->stream;
    // int cantParam = 0;
    // memcpy(&(cantParam), stream, sizeof(int));
    //stream += sizeof(int);

    //log_info(log_cpu, "Cantidad parametros:%d", cantParam);
    int l1 = 0;
    int l2 = 0;
    int l3 = 0;
    int l4 = 0;

    memcpy(&l1, stream, sizeof(int));
    stream += sizeof(int);
    instruccion_nueva->parametros1 = malloc(l1);
    memcpy(instruccion_nueva->parametros1, stream, l1);
    log_info(log_cpu, "Instruccion: %s", instruccion_nueva->parametros1);
    stream += l1;
    memcpy(&l2, stream, sizeof(int));
    stream += sizeof(int);
    instruccion_nueva->parametros2 = malloc(l2);
    memcpy(instruccion_nueva->parametros2, stream, l2);
    log_info(log_cpu, "Parametro: %s", instruccion_nueva->parametros2);
    stream += l2;
    memcpy(&l3, stream, sizeof(int));
    stream += sizeof(int);
    instruccion_nueva->parametros3= malloc(l3);
    memcpy(instruccion_nueva->parametros3, stream, l3);
    log_info(log_cpu, "Parametro: %s", instruccion_nueva->parametros3);
    stream += l3;
    memcpy(&l4, stream, sizeof(int));
    stream += sizeof(int);
    instruccion_nueva->parametros4 = malloc(l4);
    memcpy(instruccion_nueva->parametros4, stream, l4);
    log_info(log_cpu, "Parametro: %s", instruccion_nueva->parametros4);
    stream += l4;

    // DUMP_MEMORY, THREAD_EXIT Y PROCESS_EXIT NO LLEVAN PARÁMETROS

    /*t_instruccion *instruccion_nueva = malloc(l1 + l2 + l3 + l4);
    instruccion_nueva->parametros1=p1;
    instruccion_nueva->parametros2=p2;
    instruccion_nueva->parametros3=p3;
    instruccion_nueva->parametros4=p4;
    */
    eliminar_paquete(paquete);
    return instruccion_nueva;
}

/*t_instruccion* recepcionar_instruccion(t_paquete* paquete) {
    t_instruccion *instruccion_nueva = malloc(sizeof(t_instruccion));
    if (!instruccion_nueva) {
        log_error(log_cpu, "Error al asignar memoria para instruccion_nueva");
        eliminar_paquete(paquete);
        return NULL;  // Manejar error de asignación de memoria
    }

    void *stream = paquete->buffer->stream;
    int cantParam = 0;
    memcpy(&cantParam, stream, sizeof(int));
    stream += sizeof(int);

    log_info(log_cpu, "Cantidad parametros: %d", cantParam);

    // Asignación de memoria y copia para el primer parámetro
    int l1 = 0;
    memcpy(&l1, stream, sizeof(int));
    stream += sizeof(int);
    instruccion_nueva->parametros1 = malloc(l1);
    if (!instruccion_nueva->parametros1) {
        log_error(log_cpu, "Error al asignar memoria para parametros1");
        free(instruccion_nueva);
        eliminar_paquete(paquete);
        return NULL;  // Manejar error de asignación de memoria
    }
    memcpy(instruccion_nueva->parametros1, stream, l1);
    log_info(log_cpu, "Instruccion: %s", instruccion_nueva->parametros1);
    stream += l1;

    // Manejo de parámetros adicionales
    for (int i = 1; i <= cantParam; i++) {
        int longitud = 0;
        memcpy(&longitud, stream, sizeof(int));
        stream += sizeof(int);

        char **parametros = NULL;
        switch (i) {
            case 1: parametros = &instruccion_nueva->parametros2; break;
            case 2: parametros = &instruccion_nueva->parametros3; break;
            case 3: parametros = &instruccion_nueva->parametros4; break;
            default: break;  // No debería llegar aquí
        }

        if (longitud > 0) {
            *parametros = malloc(longitud);
            if (!*parametros) {
                log_error(log_cpu, "Error al asignar memoria para parametros%d", i);
                free(instruccion_nueva->parametros1);
                free(instruccion_nueva);
                eliminar_paquete(paquete);
                return NULL;  // Manejar error de asignación de memoria
            }
            memcpy(*parametros, stream, longitud);
            log_info(log_cpu, "Parametro %d: %s", i, *parametros);
            stream += longitud;
        }
    }

    eliminar_paquete(paquete);
    return instruccion_nueva;
}*/

/*void enviar_instruccion(int conexion, t_instruccion *instruccion_nueva, op_code codop) {
    t_buffer *buffer = malloc(sizeof(t_buffer));
    if (!buffer) {
        log_error(logger, "Error al asignar memoria para el buffer");
        return;  // Manejar error de asignación de memoria
    }
    
    buffer->size = 0;
    int cant_param = 0;
    int longitudes[4] = {0};  // Array para almacenar las longitudes de los parámetros

    // Calcular longitudes y cantidad de parámetros
    for (int i = 0; i < 4; i++) {
        char *param = NULL;
        switch (i) {
            case 0: param = instruccion_nueva->parametros1; break;
            case 1: param = instruccion_nueva->parametros2; break;
            case 2: param = instruccion_nueva->parametros3; break;
            case 3: param = instruccion_nueva->parametros4; break;
        }
        if (param && strlen(param) > 0) {
            longitudes[i] = strlen(param) + 1;  // +1 para el terminador nulo
            buffer->size += longitudes[i];
            cant_param++;
        }
    }

    buffer->stream = malloc(buffer->size);
    if (!buffer->stream) {
        log_error(logger, "Error al asignar memoria para el stream del buffer");
        free(buffer);
        return;  // Manejar error de asignación de memoria
    }

    void *stream = buffer->stream;
    memcpy(stream, &cant_param, sizeof(int));
    stream += sizeof(int);

    // Copiar parámetros al stream
    for (int i = 0; i < cant_param; i++) {
        memcpy(stream, &longitudes[i], sizeof(int));
        stream += sizeof(int);
        char *param = NULL;
        switch (i) {
            case 0: param = instruccion_nueva->parametros1; break;
            case 1: param = instruccion_nueva->parametros2; break;
            case 2: param = instruccion_nueva->parametros3; break;
            case 3: param = instruccion_nueva->parametros4; break;
        }
        if (param) {
            memcpy(stream, param, longitudes[i]);
            stream += longitudes[i];
        }
    }

    // Enviar el paquete con el código de operación
    send_paquete_op_code(conexion, buffer, codop);

    // Limpiar memoria
    free(buffer->stream);
    free(buffer);
}*/

