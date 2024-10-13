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

        uint32_t valor_registro_origen = obtener_valor_registro(registroDest);
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
    
    uint32_t direccionLogica = obtener_valor_registro(registro_direccion);
    uint32_t direccionFisica = traducir_direccion_logica(direccionLogica);
    
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
    uint32_t direccionFisica = traducir_direccion_logica(direccionLogica);
    
    // Verificar si la dirección es válida
    if (direccionFisica >= 0) {
        // Obtener el valor del registro de datos
        uint32_t tamanio_bytes = tamanio_registro(registro_datos);
        uint32_t valor = obtener_valor_registro(registro_datos);
        escribir_valor_en_memoria(direccionFisica, valor, tamanio_bytes);
        
        log_info(log_cpu, "TID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor escrito: %s", contexto->tid, direccionFisica, valor);
    } else {
        log_error(log_cpu, "Dirección física inválida: %d", direccionFisica);
    }
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

char *leer_valor_de_memoria(uint32_t direccionFisica, uint32_t tamanio) {

    t_paquete *paquete = crear_paquete_op(READ_MEM);
    agregar_entero_a_paquete(paquete, direccionFisica);
    agregar_entero_a_paquete(paquete, contexto->tid);
    agregar_entero_a_paquete(paquete, tamanio);

    enviar_paquete(paquete, sockets_cpu->socket_cliente);
    eliminar_paquete(paquete);
    log_trace(log_cpu, "MOV IV enviado");

    
    int cod_op = recibir_operacion(sockets_cpu->socket_cliente);
    switch (cod_op)
    {
    case 0:
        log_error(log_cpu, "Llego codigo operacion 0");
        break;
    case READ_MEM:
        log_info(log_cpu, "Codigo de operacion recibido en cpu : %d", cod_op);
        
        char *valor_recibido = recibir_string(sockets_cpu->socket_cliente, log_cpu);

        log_info(log_cpu, "PID: %d - Acción: LEER - Dirección física: %d - Valor: %s",
                    contexto->tid, direccionFisica, valor_recibido);
        log_info(log_cpu, "Recibo string :%s", valor_recibido);
        return valor_recibido;
        break;
    default:
        log_warning(log_cpu, "Llego un codigo de operacion desconocido, %d", cod_op);
        break;
    }
}

void escribir_valor_en_memoria(uint32_t direccionFisica, char *valor, uint32_t tamanio) {
    t_paquete *paquete = crear_paquete_op(WRITE_MEM);
    agregar_entero_a_paquete(paquete, direccionFisica);
    agregar_entero_a_paquete(paquete, contexto->tid);
    agregar_entero_a_paquete(paquete, tamanio);
    agregar_a_paquete(paquete, valor, strlen(valor)+1);

    enviar_paquete(paquete, sockets_cpu->socket_cliente);
    eliminar_paquete(paquete);
    log_trace(log_cpu, "MOV OUT enviado");

    op_code operacion = recibir_operacion(sockets_cpu->socket_cliente);
    
    switch (operacion){
        case 0:
            log_error(log_cpu, "Llego código operación 0");
            break;
        case WRITE_MEM:
            log_info(log_cpu, "Código de operación recibido en cpu: %d", operacion);
            log_info(log_cpu, "Valor escrito en memoria correctamente");
            log_info(log_cpu, "PID: %d - Acción: ESCRIBIR - Dirección física: %i - Valor: %s",
                        contexto->tid, tamanio, valor);
            break;
        default:
            log_warning(log_cpu, "Llegó un código de operación desconocido, %i", operacion);
            break;
        }
    
    int codigo_operaciones = recibir_operacion(sockets_cpu->socket_cliente);

}
//--FUNIONES EXTRAS

uint32_t obtener_valor_registro(char* registro){
    uint32_t valor_registro;
    if (strcmp(registro, "AX") == 0) {
        valor_registro = contexto->registros->AX;
    } else if (strcmp(registro, "BX") == 0) {
        valor_registro = contexto->registros->BX;
    } else if (strcmp(registro, "CX") == 0) {
        valor_registro = contexto->registros->CX;
    } else if (strcmp(registro, "DX") == 0) {
        valor_registro = contexto->registros->DX;
    } else if (strcmp(registro, "EX") == 0) {
        valor_registro = contexto->registros->EX;
    } else if (strcmp(registro, "FX") == 0) {
        valor_registro = contexto->registros->FX;
    } else if (strcmp(registro, "GX") == 0) {
        valor_registro = contexto->registros->GX;
    } else if (strcmp(registro, "HX") == 0) {
        valor_registro = contexto->registros->HX;
    }else {
        printf("Registro desconocido: %s\n", registro);
        return 0;
    }
    return valor_registro;
}

void valor_registro_cpu(char* registro, char* valor) {
    if (strcmp(registro, "PC") == 0) //no se si este hace falta, como los de base y limite
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

char* encontrarValorDeRegistro(char* registro){
    char* retorno = malloc(12); // Asigna suficiente espacio para el string
    if (retorno == NULL) {
        // Manejo de error en caso de que la asignación de memoria falle
        return NULL;
    }

    if (strcmp(registro, "AX") == 0) {
        sprintf(retorno, "%u", contexto->registros->AX);
        return retorno;
    } else if (strcmp(registro, "BX") == 0) {
        sprintf(retorno, "%u", contexto->registros->BX);
        return retorno;
    } else if (strcmp(registro, "CX") == 0) {
        sprintf(retorno, "%u", contexto->registros->CX);
        return retorno;
    } else if (strcmp(registro, "DX") == 0) {
        sprintf(retorno, "%u", contexto->registros->DX);
        return retorno;
    } else if (strcmp(registro, "EX") == 0) {
        sprintf(retorno, "%u", contexto->registros->EX);
        return retorno;
    } else if (strcmp(registro, "FX") == 0) {
        sprintf(retorno, "%u", contexto->registros->FX);
        return retorno;
    } else if (strcmp(registro, "GX") == 0) {
        sprintf(retorno, "%u", contexto->registros->GX);
        return retorno;
    } else if (strcmp(registro, "HX") == 0) {
        sprintf(retorno, "%u", contexto->registros->HX);
        return retorno;
    }
    // Manejo del caso en que no se encuentre el registro
    free(retorno);
    return NULL;
}
