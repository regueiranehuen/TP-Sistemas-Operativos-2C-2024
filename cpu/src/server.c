#include "server.h"

#define PUERTO_CPU_DISPATCH "4443"
#define PUERTO_CPU_INTERRUPT "4444"

void modo_server(t_log*logger){

	struct addrinfo * addrinfo_dispatch;
	struct addrinfo * addrinfo_interrupt;

	struct addrinfo hints_dispatch, hints_interrupt;


	// Crear los sockets de servidor
    int server_fd_dispatch = iniciar_servidor(PUERTO_CPU_DISPATCH, &hints_dispatch);
    int server_fd_interrupt = iniciar_servidor(PUERTO_CPU_INTERRUPT, &hints_interrupt);

    int nuevo_socket=0;

    fd_set readfds;
    int max_sd;

    // Poner los sockets en modo escucha
    if (listen(server_fd_dispatch, 3) < 0) {
        perror("Error en listen del socket dispatch");
		close(server_fd_dispatch);
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd_interrupt, 3) < 0) {
        perror("Error en listen del socket interrupt");
		close(server_fd_interrupt);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en los puertos %s y %s\n", PUERTO_CPU_DISPATCH, PUERTO_CPU_INTERRUPT);

    // Vaciar la bolsa de sockets
    FD_ZERO(&readfds);

    // Agregar los sockets al conjunto de sockets
    FD_SET(server_fd_dispatch, &readfds);
    FD_SET(server_fd_interrupt, &readfds);

    max_sd = server_fd_dispatch > server_fd_interrupt ? server_fd_dispatch : server_fd_interrupt;

    // Esperar por una actividad en los sockets usando select
    int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if (activity < 0) {
        perror("Error en select");
        close(server_fd_dispatch);
		close(server_fd_interrupt);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Comprobar actividad en el socket de dispatch
    if (FD_ISSET(server_fd_dispatch, &readfds)) {
        nuevo_socket = accept(server_fd_dispatch, (struct sockaddr*)&client_addr, &addr_len);
			
        if (nuevo_socket < 0) {
            perror("Error en accept del socket dispatch");
            close(server_fd_dispatch);
        }
        printf("Nueva conexión en puerto %s\n", PUERTO_CPU_DISPATCH);
        procesar_operaciones(nuevo_socket);

    }

    //nuevo_socket=0;

    // Comprobar actividad en el socket de interrupt
    if (FD_ISSET(server_fd_interrupt, &readfds)) {
        nuevo_socket = accept(server_fd_interrupt, (struct sockaddr*)&client_addr, &addr_len);
        if (nuevo_socket < 0) {
            perror("Error en accept del socket interrupt");
            close(server_fd_interrupt);
        }
        printf("Nueva conexión en puerto %s\n", PUERTO_CPU_INTERRUPT);

        procesar_operaciones(nuevo_socket);
    }


}

void procesar_operaciones(int nuevo_socket){
    // Procesar la operación recibida
    while(1){
        int cod_op = recibir_operacion(nuevo_socket);

        switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(nuevo_socket);
                break;
            case PAQUETE:
                t_list* lista = recibir_paquete(nuevo_socket);
                log_info(logger, "Me llegaron los siguientes valores:\n");
                list_iterate(lista, (void*)iterator);
                break;
            case -1:
                goto final;
                //break;
            default:
                log_warning(logger, "Operación desconocida. No intentes meter la pata.");
                acabar_log(logger);
                exit(EXIT_FAILURE);
        }
    }
    final:
}


void iterator(char* value) {
    log_info(logger, "%s", value);
}

void acabar_log(t_log* logger) {
    log_destroy(logger);
    remove("cpu_servidor.log");
}