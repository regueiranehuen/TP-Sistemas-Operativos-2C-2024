#include "funcExecute.h"

void funcSET(t_contexto_tid*contexto,char* registro, uint32_t valor) {
    valor_registro_cpu(contexto,registro, valor);

    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu, "SET: Registro %s = %d", registro, valor);
    pthread_mutex_unlock(&mutex_logs);
}

void funcSUM(t_contexto_tid*contexto,char* registroDest, char* registroOrig) {
    uint32_t valor_orig = obtener_valor_registro(contexto,registroOrig);
    uint32_t valor_dest = obtener_valor_registro(contexto,registroDest);

    uint32_t suma = valor_orig + valor_dest;

    valor_registro_cpu(contexto,registroDest, suma);
    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu, "SUM: %s + %s = %u", registroOrig, registroDest,suma);
    pthread_mutex_unlock(&mutex_logs);
}

void funcSUB(t_contexto_tid*contexto,char* registroDest, char* registroOrig) {
    uint32_t valor_orig = obtener_valor_registro(contexto,registroOrig);
    uint32_t valor_dest = obtener_valor_registro(contexto,registroDest);

    uint32_t resta = valor_dest - valor_orig;

    valor_registro_cpu(contexto,registroDest, resta);

    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu, "SUB: %s - %s = %u", registroDest, registroOrig,resta);
    pthread_mutex_unlock(&mutex_logs);
}

void funcJNZ(t_contexto_tid*contexto,char* registro, uint32_t num_instruccion) {
    uint32_t reg_value = obtener_valor_registro(contexto,registro);
    if (reg_value != 0) 
        contexto->registros->PC = num_instruccion;
    else
        contexto->registros->PC++;
}

void funcLOG(t_contexto_tid*contexto,char* registro) {
    uint32_t valor = obtener_valor_registro(contexto,registro);
    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu, "LOG: Registro %s = %u", registro, valor);
    pthread_mutex_unlock(&mutex_logs);
}

void funcREAD_MEM(t_contexto_pid_send *contextoPid, t_contexto_tid *contextoTid, char *registro_datos, char *registro_direccion)
{

    uint32_t direccionLogica = obtener_valor_registro(contextoTid, registro_direccion);
    uint32_t *punteroDireccionFisica = traducir_direccion_logica(contextoTid, contextoPid, direccionLogica);
    uint32_t direccionFisica;

    if (punteroDireccionFisica != NULL)
    {
        direccionFisica = *punteroDireccionFisica;
        uint32_t *valorPuntero = leer_valor_de_memoria(contextoTid->tid, contextoPid->pid, direccionFisica);
        if (valorPuntero != NULL)
        {
            uint32_t valor = *valorPuntero;
            valor_registro_cpu(contextoTid, registro_datos, valor);
            pthread_mutex_lock(&mutex_logs);
            log_debug(log_cpu, "## TID: %d - Acción: LEER - Dirección Física: %u", contextoTid->tid, direccionFisica);
            log_info(log_cpu, "READ_MEM: Dirección %u, Valor %u", direccionFisica, valor);
            pthread_mutex_unlock(&mutex_logs);

            free(valorPuntero);
        }
        free(punteroDireccionFisica);
    }
    else
    {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_cpu, "Dirección física inválida: Se recibió NULL");
        pthread_mutex_unlock(&mutex_logs);
    }
}

void funcWRITE_MEM(t_contexto_pid_send*contextoPid,t_contexto_tid*contextoTid,char* registro_direccion, char* registro_datos) {
    uint32_t direccionLogica = obtener_valor_registro(contextoTid,registro_direccion);
    uint32_t* punteroDireccionFisica = traducir_direccion_logica(contextoTid,contextoPid,direccionLogica);
    uint32_t direccionFisica;

    if (punteroDireccionFisica != NULL) {
        direccionFisica = *punteroDireccionFisica;
        uint32_t valor = obtener_valor_registro(contextoTid,registro_datos);
        int resultado = escribir_valor_en_memoria(contextoTid->tid,contextoPid->pid,direccionFisica,valor);
        if(resultado == 0){
        pthread_mutex_lock(&mutex_logs);
        log_debug(log_cpu,"## TID: %d - Acción: ESCRIBIR - Dirección Física: %u",contextoTid->tid,direccionFisica);
        pthread_mutex_unlock(&mutex_logs);
        }
        else{
        pthread_mutex_lock(&mutex_logs);
        log_info(log_cpu,"Error en la escritura en memoria");
        pthread_mutex_unlock(&mutex_logs);
        }
        free(punteroDireccionFisica);
    } else {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_cpu, "Dirección física inválida: Se recibió NULL");
        pthread_mutex_unlock(&mutex_logs);
    }
}

uint32_t* leer_valor_de_memoria(int tid, int pid, uint32_t direccionFisica){

    uint32_t*valor=malloc(sizeof(uint32_t));

    send_read_mem(tid,pid,direccionFisica,sockets_cpu->socket_memoria);

    t_paquete* paquete = recibir_paquete_op_code(sockets_cpu->socket_memoria);

    if (paquete->codigo_operacion == OK_OP_CODE) {
        *valor = recepcionar_valor_read_mem(paquete);
        return valor;
    } else {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_cpu, "Error al leer memoria");
        pthread_mutex_unlock(&mutex_logs);
        free(valor);
        return NULL; // Valor por defecto en caso de error
    }
}


int escribir_valor_en_memoria(int tid, int pid,uint32_t direccionFisica, uint32_t valor) {
    
    send_write_mem(tid,pid,direccionFisica,valor,sockets_cpu->socket_memoria);
    op_code code_op;
    recv(sockets_cpu->socket_memoria,&code_op,sizeof(int),0);
    if(code_op == OK_OP_CODE){
    return 0;
    }
    return -1;
}

uint32_t tamanio_registro(char *registro){
    if (strcmp(registro, "AX") == 0) return 1;
    else if (strcmp(registro, "BX") == 0) return 1;
    else if (strcmp(registro, "CX") == 0) return 1;
    else if (strcmp(registro, "DX") == 0) return 1;
    else if (strcmp(registro, "EX") == 0) return 1;
    else if (strcmp(registro, "FX") == 0) return 1;
    else if (strcmp(registro, "GX") == 0) return 1;
    else if (strcmp(registro, "HX") == 0) return 1;
    return 0;
}


//--FUNCIONES EXTRAS

uint32_t obtener_valor_registro(t_contexto_tid*contexto,char* registro) {

    if (strcmp(registro, "AX") == 0) return contexto->registros->AX;
    if (strcmp(registro, "BX") == 0) return contexto->registros->BX;
    if (strcmp(registro, "CX") == 0) return contexto->registros->CX;
    if (strcmp(registro, "DX") == 0) return contexto->registros->DX;
    if (strcmp(registro, "EX") == 0) return contexto->registros->EX;
    if (strcmp(registro, "FX") == 0) return contexto->registros->FX;
    if (strcmp(registro, "GX") == 0) return contexto->registros->GX;
    if (strcmp(registro, "HX") == 0) return contexto->registros->HX;
    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu,"Registro desconocido:%s", registro);
    pthread_mutex_unlock(&mutex_logs);
    return 0;
}

void valor_registro_cpu(t_contexto_tid*contexto,char* registro, uint32_t valor) {
    if (strcmp(registro, "AX") == 0) contexto->registros->AX = valor;
    else if (strcmp(registro, "BX") == 0){
        contexto->registros->BX = valor;
    }
    else if (strcmp(registro, "CX") == 0) contexto->registros->CX = valor;
    else if (strcmp(registro, "DX") == 0) contexto->registros->DX = valor;
    else if (strcmp(registro, "EX") == 0) contexto->registros->EX = valor;
    else if (strcmp(registro, "FX") == 0) contexto->registros->FX = valor;
    else if (strcmp(registro, "GX") == 0) contexto->registros->GX = valor;
    else if (strcmp(registro, "HX") == 0) contexto->registros->HX = valor;
}

//x si hay que mostrar un registro
void logRegistro(t_contexto_tid*contexto,char* registro) {
    uint32_t valor = obtener_valor_registro(contexto,registro);
    pthread_mutex_lock(&mutex_logs);
    log_info(log_cpu, "Registro %s: %u", registro, valor);
    pthread_mutex_unlock(&mutex_logs);
}
