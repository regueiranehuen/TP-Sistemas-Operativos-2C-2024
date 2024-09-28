#include "funcExecute.h"

void funcSET(char *registro, char* valor) {
    // Asignar el valor al registro correspondiente
    uint32_t val = atoi(valor);
    if (strcmp(registro, "AX") == 0) {
        contexto->registros->AX = val;
    } else if (strcmp(registro, "BX") == 0) {
        contexto->registros->BX = val;
    }
    else if (strcmp(registro, "CX") == 0) {
        contexto->registros->CX = val;
    }
    else if (strcmp(registro, "DX") == 0) {
        contexto->registros->DX = val;
    }
    else if (strcmp(registro, "EX") == 0) {
        contexto->registros->EX = val;
    }
    else if (strcmp(registro, "FX") == 0) {
        contexto->registros->FX = val;
    }
    else if (strcmp(registro, "GX") == 0) {
        contexto->registros->GX = val;
    }
    else if (strcmp(registro, "HX") == 0) {
        contexto->registros->HX = val;
    }
    log_info(log_cpu, "SET: Registro %s = %d", registro, val);
}

void funcREAD_MEM(char* registroDatos, char* registroDireccion) {
    // Traducir la dirección lógica a física usando la MMU
    uint32_t dirLogica = obtenerValorRegistro(registroDireccion);
    uint32_t dirFisica = traducirDireccionLogica(dirLogica);
    
    if (dirFisica == SEGMENTATION_FAULT) {
        manejarSegmentationFault();
        return;
    }

    uint32_t valorLeido = leerMemoria(dirFisica);  // Leer desde la memoria
    asignarValorRegistro(registroDatos, valorLeido);
    
    log_info(log_cpu, "READ_MEM: Leído %d de la Dirección Física %d", valorLeido, dirFisica);
}

void funcWRITE_MEM(char* registroDireccion, char* registroDatos) {
    // Traducir dirección lógica a física usando la MMU
    uint32_t dirLogica = obtenerValorRegistro(registroDireccion);
    uint32_t dirFisica = traducirDireccionLogica(dirLogica);

    if (dirFisica == SEGMENTATION_FAULT) {
        manejarSegmentationFault();
        return;
    }

    uint32_t valorDatos = obtenerValorRegistro(registroDatos);
    escribirMemoria(dirFisica, valorDatos);  // Escribir en memoria
    
    log_info(log_cpu, "WRITE_MEM: Escrito %d en la Dirección Física %d", valorDatos, dirFisica);
}

void funcSUM(char* registroOrig, char* registroDest) {
    
     if (strcmp(registroOrig, "AX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroOrig);
        uint32_t valor_registro_destino = obtener_valor_registro(registroDest);
        contexto->registros->AX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(registroOrig, "BX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroOrig);
        uint32_t valor_registro_destino = obtener_valor_registro(registroDest);
        contexto->registros->BX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(registroOrig, "CX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroOrig);
        uint32_t valor_registro_destino = obtener_valor_registro(registroDest);
        contexto->registros->CX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(registroOrig, "DX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroOrig);
        uint32_t valor_registro_destino = obtener_valor_registro(registroDest);
        contexto->registros->DX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(registroOrig, "EX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroOrig);
        uint32_t valor_registro_destino = obtener_valor_registro(registroDest);
        contexto->registros->EX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(registroOrig, "FX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroOrig);
        uint32_t valor_registro_destino = obtener_valor_registro(registroDest);
        contexto->registros->FX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(registroOrig, "GX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroOrig);
        uint32_t valor_registro_destino = obtener_valor_registro(registroDest);
        contexto->registros->GX = valor_registro_origen + valor_registro_destino;

    } else if (strcmp(registroOrig, "HX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroOrig);
        uint32_t valor_registro_destino = obtener_valor_registro(registroDest);
        contexto->registros->HX = valor_registro_origen + valor_registro_destino;

    } else {
        printf("Registro desconocido: %s\n", instruccion->parametros1);
    }
}

void funcSUB(char* registroDest, char* registroOrig) {

    if (strcmp(registroDest, "AX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro_(registroDest);
        uint32_t valor_registro_destino = obtener_valor_registro(registroOrig);
        contexto->registros->AX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(registroDest, "BX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroDest);
        uint32_t valor_registro_destino = obtener_valor_registro(registroOrig);
        contexto->registros->BX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(registroDest, "CX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroDest);
        uint32_t valor_registro_destino = obtener_valor_registro(registroOrig);
        contexto->registros->CX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(registroDest, "DX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroDest);
        uint32_t valor_registro_destino = obtener_valor_registro(registroOrig);
        contexto->registros->DX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(registroDest, "EX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroDest);
        uint32_t valor_registro_destino = obtener_valor_registro(registroOrig);
        contexto->registros->EX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(registroDest, "EFX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroDest);
        uint32_t valor_registro_destino = obtener_valor_registro(registroOrig);
        contexto->registros->FX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(registroDest, "GX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroDest);
        uint32_t valor_registro_destino = obtener_valor_registro(registroOrig);
        contexto->registros->GX = valor_registro_origen - valor_registro_destino;

    } else if (strcmp(registroDest, "HX") == 0) {

        uint32_t valor_registro_origen = obtener_valor_registro(registroDest);
        uint32_t valor_registro_destino = obtener_valor_registro(registroOrig);
        contexto->registros->HX = valor_registro_origen - valor_registro_destino;

    } else {
        printf("Registro desconocido: %s\n", instruccion->parametros1);
    }
}

void funcJNZ(char* registro, char* numInstruccion) {
    uint32_t valorRegistro = obtener_valor_registro(registro);
    
    if (valorRegistro != 0) {
        uint32_t nuevaInstruccion = atoi(numInstruccion);
        contexto->pc = nuevaInstruccion;  // Actualizar el Program Counter
        log_info(log_cpu, "JNZ: Salta a la instrucción %d", nuevaInstruccion);
    } else {
        log_info(log_cpu, "JNZ: No se salta porque %s = 0", registro);
    }
}

void funcLOG(char* registro) {
    uint32_t valorRegistro = obtener_valor_registro(registro);
    log_info(log_cpu, "LOG: Registro %s = %d", registro, valorRegistro);
}

uint32_t obtener_valor_registro(char* parametro){
    uint32_t valor_registro;
    if (strcmp(parametro, "AX") == 0) {
        valor_registro = contexto->registros->AX;
    } else if (strcmp(parametro, "BX") == 0) {
        valor_registro = contexto->registros->BX;
    } else if (strcmp(parametro, "CX") == 0) {
        valor_registro = contexto->registros->CX;
    } else if (strcmp(parametro, "DX") == 0) {
        valor_registro = contexto->registros->DX;
    } else if (strcmp(parametro, "EX") == 0) {
        valor_registro = contexto->registros->EX;
    } else if (strcmp(parametro, "FX") == 0) {
        valor_registro = contexto->registros->FX;
    } else if (strcmp(parametro, "GX") == 0) {
        valor_registro = contexto->registros->GX;
    } else if (strcmp(parametro, "HX") == 0) {
        valor_registro = contexto->registros->HX;
    }else {
        printf("Registro desconocido: %s\n", parametro);
        return 0;
    }
    return valor_registro;
}
