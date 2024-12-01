#include "funcExecute.h"

void funcSET(t_contexto_tid*contexto,char* registro, uint32_t valor) {
    valor_registro_cpu(contexto,registro, valor);
    log_info(log_cpu, "SET: Registro %s = %d", registro, valor);
}

void funcSUM(t_contexto_tid*contexto,char* registroDest, char* registroOrig) {
    uint32_t valor_orig = obtener_valor_registro(contexto,registroOrig);
    log_info(log_cpu,"valor origen FUNCSUM: %d",valor_orig);
    uint32_t valor_dest = obtener_valor_registro(contexto,registroDest);
    log_info(log_cpu,"valor dest FUNCSUM: %d",valor_dest);
    uint32_t suma = valor_orig + valor_dest;
    valor_registro_cpu(contexto,registroDest, suma);
    log_info(log_cpu, "SUM: %s + %s", registroOrig, registroDest);
}

void funcSUB(t_contexto_tid*contexto,char* registroDest, char* registroOrig) {
    uint32_t valor_orig = obtener_valor_registro(contexto,registroOrig);
    log_info(log_cpu,"valor origen FUNCSUB: %d",valor_orig);
    uint32_t valor_dest = obtener_valor_registro(contexto,registroDest);
    log_info(log_cpu,"valor dest FUNCSUB: %d",valor_dest);
    uint32_t resta = valor_dest - valor_orig;
    valor_registro_cpu(contexto,registroDest, resta);
    log_info(log_cpu, "SUB: %s - %s", registroDest, registroOrig);
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
    log_info(log_cpu, "LOG: Registro %s = %d", registro, valor);
}

void funcREAD_MEM(t_contexto_pid_send*contextoPid,t_contexto_tid*contextoTid,char* registro_datos, char* registro_direccion) {
    printf("registro de datos:%s\n",registro_datos);
    printf("registro de direccion:%s\n",registro_direccion);

    uint32_t direccionLogica = obtener_valor_registro(contextoTid,registro_direccion);
    uint32_t* punteroDireccionFisica = traducir_direccion_logica(contextoTid,contextoPid,direccionLogica);
    uint32_t direccionFisica;

    if (punteroDireccionFisica != NULL) {
        direccionFisica = *punteroDireccionFisica;
        uint32_t valor = leer_valor_de_memoria(direccionFisica);
        if(valor != -1){
        valor_registro_cpu(contextoTid,registro_datos, valor);
        log_info(log_cpu,"## Lectura - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %d",contextoTid->pid,contextoTid->tid,direccionFisica,contextoPid->tamanio_proceso);
        log_info(log_cpu, "READ_MEM: Dirección %d, Valor %d", direccionFisica, valor);
        }
        free(punteroDireccionFisica);
    } else {
        log_error(log_cpu, "Dirección física inválida: Se recibió NULL");
    }
}

void funcWRITE_MEM(t_contexto_pid_send*contextoPid,t_contexto_tid*contextoTid,char* registro_direccion, char* registro_datos) {
    uint32_t direccionLogica = obtener_valor_registro(contextoTid,registro_direccion);
    uint32_t* punteroDireccionFisica = traducir_direccion_logica(contextoTid,contextoPid,direccionLogica);
    uint32_t direccionFisica;

    if (punteroDireccionFisica != NULL) {
        direccionFisica = *punteroDireccionFisica;
        uint32_t valor = obtener_valor_registro(contextoTid,registro_datos);
        int resultado = escribir_valor_en_memoria(direccionFisica, valor);
        if(resultado == 0){
        log_info(log_cpu,"## Escritura - (PID:TID) - (%d:%d) - Dir. Física: %u - Tamaño: %d",contextoTid->pid,contextoTid->tid,direccionFisica,contextoPid->tamanio_proceso);
        }
        else{
        log_info(log_cpu,"Error en la escritura en memoria");
        }
        free(punteroDireccionFisica);
    } else {
        log_error(log_cpu, "Dirección física inválida: Se recibió NULL");
    }
}

uint32_t leer_valor_de_memoria(uint32_t direccionFisica) {

    send_read_mem(direccionFisica,sockets_cpu->socket_memoria);

    t_paquete* paquete = recibir_paquete_op_code(sockets_cpu->socket_memoria);

    if (paquete->codigo_operacion == OK_OP_CODE) {
        uint32_t valor = recepcionar_read_mem(paquete);
        return valor;
    } else {
        log_error(log_cpu, "Error al leer memoria");
        return -1; // Valor por defecto en caso de error
    }
}


int escribir_valor_en_memoria(uint32_t direccionFisica, uint32_t valor) {
    
    send_write_mem(direccionFisica,valor,sockets_cpu->socket_memoria);
    op_code code_op;
    recv(sockets_cpu->socket_memoria,&code_op,sizeof(op_code),0);
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
    log_info(log_cpu,"registro:%s",registro);
    if (strcmp(registro, "AX") == 0) return contexto->registros->AX;
    if (strcmp(registro, "BX") == 0) return contexto->registros->BX;
    if (strcmp(registro, "CX") == 0) return contexto->registros->CX;
    if (strcmp(registro, "DX") == 0) return contexto->registros->DX;
    if (strcmp(registro, "EX") == 0) return contexto->registros->EX;
    if (strcmp(registro, "FX") == 0) return contexto->registros->FX;
    if (strcmp(registro, "GX") == 0) return contexto->registros->GX;
    if (strcmp(registro, "HX") == 0) return contexto->registros->HX;
    log_info(log_cpu,"Registro desconocido:%s", registro);
    return 0;
}

void valor_registro_cpu(t_contexto_tid*contexto,char* registro, uint32_t valor) {
    if (strcmp(registro, "AX") == 0) contexto->registros->AX = valor;
    else if (strcmp(registro, "BX") == 0){
        contexto->registros->BX = valor;
        log_info(log_cpu,"SI ESCRIBO ESTO ES QUE ESTOY RE BASADO AAAAAH Y MI VALOR ES:%d",contexto->registros->BX);
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
    log_info(log_cpu, "Registro %s: %u", registro, valor);
}
