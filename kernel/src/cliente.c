#include "includes/cliente.h"

void cliente_Memoria_Kernel(t_log* log, t_config* config) {
    char* ip;
    char* puerto;
    int socket_cliente;
    char* mensaje_prueba = "Hola";

    // Asignar valores a las variables ip y puerto usando config_get_string_value
    ip = config_get_string_value(config, "IP_MEMORIA");
    puerto = config_get_string_value(config, "PUERTO_MEMORIA");

    // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto == NULL) {
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        return;
    }

    // Crear conexión
    socket_cliente = crear_conexion(log, ip, puerto);
    if (socket_cliente == -1) {
        log_info(log, "No se pudo crear la conexión");
        return;
    }

    // Enviar mensaje
    enviar_mensaje(mensaje_prueba, socket_cliente);

    // Liberar recursos
    config_destroy(config);
    log_destroy(log);
    close(socket_cliente);
}