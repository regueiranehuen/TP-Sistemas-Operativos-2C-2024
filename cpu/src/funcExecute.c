#include "funcExecute.h"

void funcSET(t_contexto_tid*contexto,char* registro, uint32_t valor) {
    valor_registro_cpu(contexto,registro, valor);
    log_info(log_cpu, "SET: Registro %s = %d", registro, valor);
}

void funcSUM(t_contexto_tid*contexto,char* registroOrig, char* registroDest) {
    uint32_t valor_orig = obtener_valor_registro(contexto,registroOrig);
    uint32_t valor_dest = obtener_valor_registro(contexto,registroDest);
    valor_registro_cpu(contexto,registroDest, valor_orig + valor_dest);
    log_info(log_cpu, "SUM: %s + %s", registroOrig, registroDest);
}

void funcSUB(t_contexto_tid*contexto,char* registroDest, char* registroOrig) {
    uint32_t valor_orig = obtener_valor_registro(contexto,registroOrig);
    uint32_t valor_dest = obtener_valor_registro(contexto,registroDest);
    valor_registro_cpu(contexto,registroDest, valor_dest - valor_orig);
    log_info(log_cpu, "SUB: %s - %s", registroDest, registroOrig);
}

void funcJNZ(t_contexto_tid*contexto,char* registro, uint32_t num_instruccion) {
    uint32_t reg_value = obtener_valor_registro(contexto,registro);
    if (reg_value != 0) contexto->registros->PC = num_instruccion;
}

void funcLOG(t_contexto_tid*contexto,char* registro) {
    uint32_t valor = obtener_valor_registro(contexto,registro);
    log_info(log_cpu, "LOG: Registro %s = %d", registro, valor);
}



void funcREAD_MEM(t_contexto_pid*contextoPid,t_contexto_tid*contextoTid,char* registro_datos, char* registro_direccion) {
    uint32_t direccionLogica = obtener_valor_registro(contextoTid,registro_direccion);
    uint32_t direccionFisica = traducir_direccion_logica(contextoTid,contextoPid,direccionLogica);

    if (direccionFisica >= 0) {
        uint32_t valor = leer_valor_de_memoria(direccionFisica);
        valor_registro_cpu(contextoTid,registro_datos, valor);
        log_info(log_cpu, "READ_MEM: Dirección %d, Valor %d", 
                 direccionFisica, valor);
    } else {
        log_error(log_cpu, "Dirección física inválida: %d", direccionFisica);
    }
}

void funcWRITE_MEM(t_contexto_pid*contextoPid,t_contexto_tid*contextoTid,char* registro_direccion, char* registro_datos) {
    uint32_t direccionLogica = obtener_valor_registro(contextoTid,registro_direccion);
    uint32_t direccionFisica = traducir_direccion_logica(contextoTid,contextoPid,direccionLogica);

    if (direccionFisica >= 0) {
        uint32_t valor = obtener_valor_registro(contextoTid,registro_datos);
        escribir_valor_en_memoria(direccionFisica, valor);
        log_info(log_cpu, "WRITE_MEM: Dirección %d, Valor %d", 
                 direccionFisica, valor);
    } else {
        log_error(log_cpu, "Dirección física inválida: %d", direccionFisica);
    }
}

uint32_t leer_valor_de_memoria(uint32_t direccionFisica) {
    t_paquete* paquete = crear_paquete_op(READ_MEM);
    agregar_entero_a_paquete(paquete, direccionFisica); // Solo dirección
    enviar_paquete(paquete, sockets_cpu->socket_memoria);
    eliminar_paquete(paquete);

    int cod_op = recibir_operacion(sockets_cpu->socket_memoria);
    if (cod_op == READ_MEM) {
        uint32_t valor = recibir_entero_uint32(sockets_cpu->socket_memoria);
        return valor;
    } else {
        log_error(log_cpu, "Error al leer memoria");
        return 0; // Valor por defecto en caso de error
    }
}


void escribir_valor_en_memoria(uint32_t direccionFisica, uint32_t valor) {
    t_paquete* paquete = crear_paquete_op(WRITE_MEM);
    agregar_entero_a_paquete(paquete, direccionFisica); // Solo dirección
    agregar_entero_a_paquete(paquete, valor); // Valor a escribir
    enviar_paquete(paquete, sockets_cpu->socket_memoria);
    eliminar_paquete(paquete);
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


//--FUNIONES EXTRAS

uint32_t obtener_valor_registro(t_contexto_tid*contexto,char* registro) {
    if (strcmp(registro, "AX") == 0) return contexto->registros->AX;
    if (strcmp(registro, "BX") == 0) return contexto->registros->BX;
    if (strcmp(registro, "CX") == 0) return contexto->registros->CX;
    if (strcmp(registro, "DX") == 0) return contexto->registros->DX;
    if (strcmp(registro, "EX") == 0) return contexto->registros->EX;
    if (strcmp(registro, "FX") == 0) return contexto->registros->FX;
    if (strcmp(registro, "GX") == 0) return contexto->registros->GX;
    if (strcmp(registro, "HX") == 0) return contexto->registros->HX;
    printf("Registro desconocido: %s\n", registro);
    return 0;
}

void valor_registro_cpu(t_contexto_tid*contexto,char* registro, uint32_t valor) {
    if (strcmp(registro, "AX") == 0) contexto->registros->AX = valor;
    else if (strcmp(registro, "BX") == 0) contexto->registros->BX = valor;
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
