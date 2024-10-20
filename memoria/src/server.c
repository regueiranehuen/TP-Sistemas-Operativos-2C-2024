#include "includes/server.h"

static pthread_mutex_t cliente_count_mutex = PTHREAD_MUTEX_INITIALIZER;
static int client_count = 0; //numero incremental del numero del cliente
sem_t sem_conexion_hecha;

t_list*lista_contextos_pids;
pthread_mutex_t mutex_lista_contextos_pids;


void* hilo_por_cliente (void* void_args){

hilo_clientes *args = (hilo_clientes*)void_args;

int socket_cliente = esperar_cliente(args->log,args->socket_servidor);
if (socket_cliente == -1) {
    log_error(args->log, "Error al esperar cliente");
    close(args->socket_servidor);
    free(args);
    return NULL;
}

int cliente_n;
pthread_mutex_lock(&cliente_count_mutex);
cliente_n = ++client_count;
pthread_mutex_unlock(&cliente_count_mutex);
if(cliente_n <= 2){//conexiones iniciales de cpu y kernel
servidor_handshake(socket_cliente,args->log); 
log_info(args->log, "Handshake memoria -> cliente_%d realizado correctamente", cliente_n);
}
sem_post(&sem_conexion_hecha);
atender_conexiones(socket_cliente);
free(args);
return NULL;
}


void* gestor_clientes(void* void_args){// Crear un hilo que crea hilos que crean conexiones para cada petición de kernel

hilo_clientes *args = (hilo_clientes*)void_args;
int respuesta;
printf("Estado_servidor: %d\n", estado_cpu);
while(estado_cpu != 0){// mientras el servidor este abierto

hilo_clientes* args_hilo = malloc(sizeof(hilo_clientes));
args_hilo ->log = args->log;
args_hilo ->socket_servidor=args->socket_servidor;

pthread_t hilo_cliente;

respuesta = pthread_create (&hilo_cliente,NULL,hilo_por_cliente,(void*)args_hilo);

if(respuesta != 0){
    log_error(args->log, "Error al crear el hilo para el cliente");
    free(args);
    free(args_hilo);
    continue;
}
sem_wait(&sem_conexion_hecha); //esperar a que un cliente se conecte para esperar otro
pthread_detach(hilo_cliente); 
}
return NULL;
}

int servidor_memoria_kernel (t_log* log, t_config* config){

hilo_clientes* args = malloc(sizeof(hilo_clientes)); 

char * puerto;
int socket_servidor,respuesta;

pthread_t hilo_gestor;


puerto = config_get_string_value(config,"PUERTO_ESCUCHA");

socket_servidor = iniciar_servidor(log,puerto);

args->log=log;
args->socket_servidor=socket_servidor;


if (socket_servidor == -1) {
    log_error(log, "Error al iniciar el servidor");
    return -1;
    }

    log_info(log,"Servidor abierto correctamente");

respuesta = pthread_create(&hilo_gestor,NULL,gestor_clientes,(void*)args);

if(respuesta != 0){
    log_error(log,"Error al crear el hilo_gestor_clientes");
    free (args);
    return -1;
}

pthread_join(hilo_gestor,NULL);
    free(args);
	return socket_servidor;
}

void* funcion_hilo_servidor(void *void_args){
    
    args_hilo *args = (args_hilo*)void_args;

    int socket_servidor = servidor_memoria_kernel(args->log,args->config);
    if (socket_servidor == -1) {
        log_error(args->log, "No se pudo establecer la conexion con la Memoria");
        return NULL;
    }

   return (void*)(intptr_t)socket_servidor;
}

int cliente_memoria_filesystem (t_log* log, t_config* config){

char * ip, * puerto;
int socket_cliente, respuesta;

ip = config_get_string_value(config, "IP_FILESYSTEM");
puerto = config_get_string_value(config, "PUERTO_FILESYSTEM");

 // Verificar que ip y puerto no sean NULL
    if (ip == NULL || puerto == NULL) {
        log_info(log, "No se pudo obtener IP o PUERTO de la configuración");
        return -1;
    }

    // Crear conexion
    socket_cliente = crear_conexion(log, ip, puerto);

    if (socket_cliente == -1) {
        log_info(log, "No se pudo crear la conexion");
        return -1;
    }

   respuesta = cliente_handshake(socket_cliente,log);

   if (respuesta == 0){
    log_info(log,"Handshake de Memoria --> Filesystem realizado correctamente");
   }
   else {
    log_error(log, "Handshake de Memoria --> Filesystem tuvo un error");
   }

    return socket_cliente;

}

void* funcion_hilo_cliente(void* void_args){
    
    args_hilo* args = ((args_hilo*)void_args);


    int socket_cliente = cliente_memoria_filesystem(args->log,args->config);
    if (socket_cliente == -1) {
        log_error(args->log, "No se pudo establecer la conexion con filesystem");
        pthread_exit(NULL);
    }

   return (void*)(intptr_t)socket_cliente;
}

sockets_memoria* hilos_memoria(t_log* log, t_config* config){

pthread_t hilo_servidor;
pthread_t hilo_cliente;

args_hilo* args = malloc(sizeof(args_hilo)); 

args->config=config;
args->log=log;

void* socket_cliente;
void* socket_servidor;

int resultado;

sockets_memoria* sockets=malloc(sizeof(sockets_memoria));

resultado = pthread_create (&hilo_cliente,NULL,funcion_hilo_cliente,(void*)args);

if(resultado != 0){
    log_error(log,"Error al crear el hilo");
    free (args);
    return NULL;
}

log_info(log,"El hilo cliente se creo correctamente");

resultado = pthread_create (&hilo_servidor,NULL,funcion_hilo_servidor,(void*)args);

if(resultado != 0){
    log_error(log,"Error al crear el hilo");
    free (args);
    return NULL;
}

log_info(log,"El hilo servidor se creo correctamente");


pthread_join(hilo_cliente,&socket_cliente);
pthread_join(hilo_servidor,&socket_servidor);

resultado = (intptr_t)socket_cliente;

sockets->socket_cliente=resultado;

resultado = (intptr_t)socket_servidor;

sockets->socket_servidor = resultado;

free(args);
return sockets;
}
