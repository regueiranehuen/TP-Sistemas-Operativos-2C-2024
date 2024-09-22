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
    uint32_t valorOrig = obtenerValorRegistro(registroOrig);
    uint32_t valorDest = obtenerValorRegistro(registroDest);
    
    uint32_t resultado = valorDest + valorOrig;
    asignarValorRegistro(registroDest, resultado);
    
    log_info(log_cpu, "SUM: %s = %d + %d", registroDest, valorDest, valorOrig);
}

void funcSUB(char* registroDest, char* registroOrig) {
    uint32_t valorOrig = obtenerValorRegistro(registroOrig);
    uint32_t valorDest = obtenerValorRegistro(registroDest);
    
    uint32_t resultado = valorDest - valorOrig;
    asignarValorRegistro(registroDest, resultado);
    
    log_info(log_cpu, "SUB: %s = %d - %d", registroDest, valorDest, valorOrig);
}

void funcJNZ(char* registro, char* numInstruccion) {
    uint32_t valorRegistro = obtenerValorRegistro(registro);
    
    if (valorRegistro != 0) {
        uint32_t nuevaInstruccion = atoi(numInstruccion);
        contexto->pc = nuevaInstruccion;  // Actualizar el Program Counter
        log_info(log_cpu, "JNZ: Salta a la instrucción %d", nuevaInstruccion);
    } else {
        log_info(log_cpu, "JNZ: No se salta porque %s = 0", registro);
    }
}

void funcLOG(char* registro) {
    uint32_t valorRegistro = obtenerValorRegistro(registro);
    log_info(log_cpu, "LOG: Registro %s = %d", registro, valorRegistro);
}

