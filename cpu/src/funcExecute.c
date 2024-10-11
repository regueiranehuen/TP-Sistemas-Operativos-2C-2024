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
        printf("Registro desconocido: %s\n", registroOrig);
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
        printf("Registro desconocido: %s\n", registroDest);
    }
}

void funcJNZ(char* registro, char* num_instruccion) {
    uint32_t reg_value;
    if (strcmp(registro, "AX") == 0) {
        reg_value = contexto->registros->AX;
    } else if (strcmp(registro, "BX") == 0) {
        reg_value = contexto->registros->BX;
    } else if (strcmp(registro, "CX") == 0) {
        reg_value = contexto->registros->CX;
    } else if (strcmp(registro, "DX") == 0) {
        reg_value = contexto->registros->DX;
    } else if (strcmp(registro, "EX") == 0) {
        reg_value = contexto->registros->EX;
    } else if (strcmp(registro, "FX") == 0) {
        reg_value = contexto->registros->FX;
    } else if (strcmp(registro, "GX") == 0) {
        reg_value = contexto->registros->GX;
    } else if (strcmp(registro, "HX") == 0) {
        reg_value = contexto->registros->HX;
    } else {
        printf("Registro desconocido: %s\n", registro);
        return;
    }

    if (reg_value != 0) {
        contexto->pc = atoi(num_instruccion);
    }
}

void funcLOG(char* registro) {
    uint32_t valorRegistro = obtener_valor_registro(registro);
    log_info(log_cpu, "LOG: Registro %s = %d", registro, valorRegistro);
}


void funcREAD_MEM(char* registro_datos, char* registro_direccion) {
    log_info(log_cpu, "Instrucción READ_MEM ejecutada");
    
    // Obtener la dirección lógica desde el registro de dirección
    uint32_t direccionLogica = obtener_valor_registro(registro_direccion);
    
    // Traducir la dirección lógica a física
    uint32_t direccionFisica = traducirDireccion(direccionLogica);
    
    // Verificar si la dirección es válida
    if (direccionFisica >= 0) {
        // Leer el valor desde la memoria en esa dirección física
        int tamanio_bytes = tamanio_registro(registro_datos);
        uint32_t valorMemoria = leer_valor_de_memoria(direccionFisica, tamanio_bytes);
        
        // Almacenar el valor leído en el registro de datos
        char buffer[20];
        sprintf(buffer, "%d", valorMemoria);
        valor_registro_cpu(registro_datos, buffer);
        
        log_info(log_cpu, "TID: %d - Acción: LEER - Dirección Física: %d - Valor leído: %d", contexto->tid, direccionFisica, valorMemoria);
    } else {
        log_error(log_cpu, "Dirección física inválida: %d", direccionFisica);
    }
}

void funcWRITE_MEM(char* registro_direccion, char* registro_datos) {
    log_info(log_cpu, "Instrucción WRITE_MEM ejecutada");
    
    // Obtener la dirección lógica desde el registro de dirección
    uint32_t direccionLogica = obtener_valor_registro(registro_direccion);
    
    // Traducir la dirección lógica a física
    uint32_t direccionFisica = traducirDireccion(direccionLogica);
    
    // Verificar si la dirección es válida
    if (direccionFisica >= 0) {
        // Obtener el valor del registro de datos
        uint32_t tamanio_bytes = tamanio_registro(registro_datos);
        char* valor = encontrarValorDeRegistro(registro_datos);
        
        // Escribir el valor en la memoria en esa dirección física
        escribir_valor_en_memoria(direccionFisica, valor, tamanio_bytes);
        
        log_info(log_cpu, "TID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor escrito: %s", contexto->tid, direccionFisica, valor);
    } else {
        log_error(log_cpu, "Dirección física inválida: %d", direccionFisica);
    }
}

//--FUNIONES EXTRAS

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

void valor_registro_cpu(char* registro, char* valor) {
    if (strcmp(registro, "PC") == 0) //no se si este hace falta
        contexto->pc = atoi(valor);
    if (strcmp(registro, "AX") == 0)
        contexto->registros->AX = atoi(valor);
    if (strcmp(registro, "BX") == 0)
        contexto->registros->BX = atoi(valor);
    if (strcmp(registro, "CX") == 0)
        contexto->registros->CX = atoi(valor);
    if (strcmp(registro, "DX") == 0)
        contexto->registros->DX = atoi(valor);
    if (strcmp(registro, "EX") == 0)
        contexto->registros->EX = atoi(valor);
    if (strcmp(registro, "FX") == 0)
        contexto->registros->FX = atoi(valor);
    if (strcmp(registro, "GX") == 0)
        contexto->registros->GX = atoi(valor);
    if (strcmp(registro, "HX") == 0)
        contexto->registros->HX = atoi(valor);
}

char* encontrarValorDeRegistro(char* register_to_find_value) {
    char* retorno = malloc(12); // Asigna suficiente espacio para el string
    if (retorno == NULL) {
        // Manejo de error en caso de que la asignación de memoria falle
        return NULL;
    }

    if (strcmp(register_to_find_value, "AX") == 0) {
        sprintf(retorno, "%u", contexto->registros->AX);
        return retorno;
    } else if (strcmp(register_to_find_value, "BX") == 0) {
        sprintf(retorno, "%u", contexto->registros->BX);
        return retorno;
    } else if (strcmp(register_to_find_value, "CX") == 0) {
        sprintf(retorno, "%u", contexto->registros->CX);
        return retorno;
    } else if (strcmp(register_to_find_value, "DX") == 0) {
        sprintf(retorno, "%u", contexto->registros->DX);
        return retorno;
    } else if (strcmp(register_to_find_value, "EX") == 0) {
        sprintf(retorno, "%u", contexto->registros->EX);
        return retorno;
    } else if (strcmp(register_to_find_value, "FX") == 0) {
        sprintf(retorno, "%u", contexto->registros->FX);
        return retorno;
    } else if (strcmp(register_to_find_value, "GX") == 0) {
        sprintf(retorno, "%u", contexto->registros->GX);
        return retorno;
    } else if (strcmp(register_to_find_value, "HX") == 0) {
        sprintf(retorno, "%u", contexto->registros->HX);
        return retorno;

    // Manejo del caso en que no se encuentre el registro
    free(retorno);
    return NULL;
}