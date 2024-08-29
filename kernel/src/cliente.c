#include "cliente.h"



t_log* iniciar_logger(void)
{
    t_log* logger;


	char * file = "logger_cliente.log";
	char * process_name = "Logs cliente";
	bool is_active_console = true;
	t_log_level level = LOG_LEVEL_INFO;
	logger = log_create(file, process_name, is_active_console,level);

	return logger;
}

t_config* iniciar_config(void)
{
    t_config* config;

    // Creo una estructura t_config para que levante el archivo cliente.config. Así, lo que se loguee se guarde en un archivo config y no solo sea info en el codigo
	config = config_create("kernel.config");

	return config;
}

void leer_consola(t_log* logger, t_paquete * paquete)
{


	char* linea = readline(">");

	if (strcmp(linea,"")!=0){
		while(strcmp(linea,"")!=0){
			log_info(logger,linea);
			agregar_a_paquete(paquete,linea,strlen(linea) + 1);
			free(linea);
			linea=readline(">");
		}
	}



	free(linea);


}



void terminar_programa(int socket_conexion_dispatch,int socket_conexion_interrupt ,t_log* logger, t_config* config)
{

	config_destroy(config);
    liberar_conexion(socket_conexion_dispatch);
	liberar_conexion(socket_conexion_interrupt);
	log_destroy(logger);

	remove("logger_cliente.log");
}
