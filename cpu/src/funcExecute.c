#include "funcExecute.h"

void funcSET(char *registro, char* valor) {
    // Asignar el valor al registro correspondiente
    uint32_t val = atoi(valor);
    if (strcmp(registro, "AX") == 0) {
        contextoActual->registros->AX = val;
    } else if (strcmp(registro, "BX") == 0) {
        contextoActual->registros->BX = val;
    }
    // Añadir el resto de los registros (CX, DX, etc.)
    log_info(logger, "SET: Registro %s = %d", registro, val);
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
    
    log_info(logger, "READ_MEM: Leído %d de la Dirección Física %d", valorLeido, dirFisica);
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
    
    log_info(logger, "WRITE_MEM: Escrito %d en la Dirección Física %d", valorDatos, dirFisica);
}

void funcSUM(char* registroOrig, char* registroDest) {
    uint32_t valorOrig = obtenerValorRegistro(registroOrig);
    uint32_t valorDest = obtenerValorRegistro(registroDest);
    
    uint32_t resultado = valorDest + valorOrig;
    asignarValorRegistro(registroDest, resultado);
    
    log_info(logger, "SUM: %s = %d + %d", registroDest, valorDest, valorOrig);
}

void funcSUB(char* registroDest, char* registroOrig) {
    uint32_t valorOrig = obtenerValorRegistro(registroOrig);
    uint32_t valorDest = obtenerValorRegistro(registroDest);
    
    uint32_t resultado = valorDest - valorOrig;
    asignarValorRegistro(registroDest, resultado);
    
    log_info(logger, "SUB: %s = %d - %d", registroDest, valorDest, valorOrig);
}

void funcJNZ(char* registro, char* numInstruccion) {
    uint32_t valorRegistro = obtenerValorRegistro(registro);
    
    if (valorRegistro != 0) {
        uint32_t nuevaInstruccion = atoi(numInstruccion);
        contextoActual->pc = nuevaInstruccion;  // Actualizar el Program Counter
        log_info(logger, "JNZ: Salta a la instrucción %d", nuevaInstruccion);
    } else {
        log_info(logger, "JNZ: No se salta porque %s = 0", registro);
    }
}

void funcLOG(char* registro) {
    uint32_t valorRegistro = obtenerValorRegistro(registro);
    log_info(logger, "LOG: Registro %s = %d", registro, valorRegistro);
}

