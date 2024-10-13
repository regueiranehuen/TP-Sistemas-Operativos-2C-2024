#include "includes/commCpu.h"

void recibir_cpu(int SOCKET_CLIENTE_CPU) {
    int retardo_respuesta = config_get_int_value(config,"RETARDO_RESPUESTA");
    int codigoOperacion = 0;
    while (codigoOperacion != -1) {
        op_code codOperacion = recibir_operacion(SOCKET_CLIENTE_CPU);
        usleep(retardo_respuesta * 1000);  // Aplicar retardo configurado

        switch (codOperacion) {
            case OBTENER_CONTEXTO: {
                t_2_enteros *solicitud = recibir_2_enteros(SOCKET_CLIENTE_CPU);  // Recibe PID y TID
                uint32_t pid = solicitud->entero1;
                uint32_t tid = solicitud->entero2;
                free(solicitud);

                t_contexto *contexto = obtener_contexto(pid, tid);
                enviar_contexto(SOCKET_CLIENTE_CPU, contexto, OBTENER_CONTEXTO);
                log_info(logger, "Enviado contexto para PID: %d, TID: %d", pid, tid);
                free(contexto);
                break;
            }

            case ACTUALIZAR_CONTEXTO: {
                t_contexto *contexto = recibir_contexto(SOCKET_CLIENTE_CPU);  // Recibe nuevo contexto
                actualizar_contexto(contexto);
                enviar_codop(SOCKET_CLIENTE_CPU, ACTUALIZACION_OK);
                log_info(logger, "Contexto actualizado para TID: %d", contexto->tid);
                free(contexto);
                break;
            }

            case OBTENER_INSTRUCCION: {
                t_2_enteros *solicitud = recibir_2_enteros(SOCKET_CLIENTE_CPU);  // PID y PC
                uint32_t tid = solicitud->entero1;
                uint32_t pc = solicitud->entero2;
                free(solicitud);

                char *instruccion = obtener_instruccion(tid, pc);
                enviar_instruccion(SOCKET_CLIENTE_CPU, instruccion, OBTENER_INSTRUCCION);
                log_info(logger, "Instrucción enviada para PID: %d, PC: %d", tid, pc);
                free(instruccion);
                break;
            }

            case READ_MEM: {
                t_2_enteros *solicitud = recibir_2_enteros(SOCKET_CLIENTE_CPU);  // Dirección física y tamaño
                uint32_t direccion_fisica = solicitud->entero1;
                uint32_t tam_a_leer = solicitud->entero2;
                free(solicitud);

                void *datos = malloc(tam_a_leer);
                memcpy(datos, ESPACIO_USUARIO + direccion_fisica, tam_a_leer);
                enviar_datos(SOCKET_CLIENTE_CPU, datos, tam_a_leer);
                log_info(logger, "Memoria leída desde %d, tamaño %d", direccion_fisica, tam_a_leer);
                free(datos);
                break;
            }

            case WRITE_MEM: {
                t_string_2enteros *escritura = recibir_string_2_enteros(SOCKET_CLIENTE_CPU);
                uint32_t direccion_fisica = escritura->entero1;
                uint32_t tam_a_escribir = escritura->entero2;
                char *contenido = escritura->string;

                memcpy(ESPACIO_USUARIO + direccion_fisica, contenido, tam_a_escribir);
                enviar_codop(SOCKET_CLIENTE_CPU, WRITE_OK);
                log_info(logger, "Escritos %d bytes en dirección %d", tam_a_escribir, direccion_fisica);
                free(escritura);
                break;
            }

            case 1:
                codigoOperacion = codOperacion;
                break;

            default:
                log_warning(logger, "Operación desconocida recibida: %d", codOperacion);
                break;
        }
    }

    log_warning(log_memoria, "Se desconectó la CPU");
}
