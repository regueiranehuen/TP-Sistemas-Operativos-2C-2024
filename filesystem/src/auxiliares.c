#include "includes/auxiliares.h"

void inicializar_estructuras(void){
    
    sem_init(&sem_fin_filesystem, 0, 0);
    sem_init(&sem_conexion_hecha, 0, 0);
    sem_init(&sem_termina_hilo,0,0);
    pthread_mutex_init(&mutex_bitmap,NULL);
    config = config_create("filesystem.config");
    t_log_level log_level_int = log_level(config);
    log_filesystem = log_create("filesystem.log", "tp", true, log_level_int);

    mount_dir = config_get_string_value(config, "MOUNT_DIR");
    block_count = config_get_int_value(config, "BLOCK_COUNT");
    block_size = config_get_double_value(config, "BLOCK_SIZE");
    

    bitmap = cargar_bitmap(mount_dir, block_count);

}

uint32_t bytes_a_escribir(t_args_dump_memory* info,uint32_t bytes_escritos){
    
    if (info->tamanio_particion_proceso - bytes_escritos >= block_size){
        return block_size;
    }
    else{

        return info->tamanio_particion_proceso - bytes_escritos;
    }
}

void detectar_cierre(int socket_cliente){
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket_cliente,&readfds);

    int actividad = select(socket_cliente + 1,&readfds,NULL,NULL,NULL);

    if (actividad < 0)
    {
        log_error(log_filesystem,"Error en select (socket conexion memoria)");
        exit(EXIT_FAILURE);
    }

    // Si se detecta actividad en el socket de memoria
    if (FD_ISSET(socket_cliente, &readfds))
    {
        log_info(log_filesystem,"Se detectó actividad en el socket de conexion con memoria");
    }
}