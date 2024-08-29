#include "cliente.h"

int main(void){

    t_log* logger;
	t_config* config;

	// Crear logger
	logger = iniciar_logger();

    if (logger==NULL){
		printf("No se pudo crear el logger");
		abort();
	}


    // Creo una estructura t_config para que levante el archivo cliente.config. Así, lo que se loguee se guarde en un archivo config y no solo sea info en el codigo
	config = config_create("kernel.config");

	if (config == NULL) {
		printf("No se pudo crear la estructura config");
		abort();
	}



	char * ip_cpu = config_get_string_value(config,"IP_CPU");
	char*puerto_cpu_dispatch=config_get_string_value(config,"PUERTO_CPU_DISPATCH");
	char*puerto_cpu_interrupt=config_get_string_value(config,"PUERTO_CPU_INTERRUPT");


	// Creo un nuevo paquete
	t_paquete*paquete = crear_paquete();

	

	// Leer lo que se ingresa x consola
	leer_consola(logger,paquete);


	// Serializo paquete

	//serializar_paquete(paquete,sizeof(paquete));

    
    int socket_conexion_dispatch = crear_conexion(ip_cpu,puerto_cpu_dispatch);
	int socket_conexion_interrupt = crear_conexion(ip_cpu,puerto_cpu_interrupt);

	enviar_mensaje("hola dispatch",socket_conexion_dispatch);
	enviar_mensaje("mensaje para interrupt",socket_conexion_interrupt);

	enviar_mensaje("DISPATCH SOY YO DE NUEVO",socket_conexion_dispatch);
	

	enviar_mensaje("OH OU INTERRUPCION OU UUUDNIEHJIED",socket_conexion_interrupt);
	enviar_paquete(paquete,socket_conexion_interrupt);

	terminar_programa(socket_conexion_dispatch,socket_conexion_interrupt,logger,config);
}