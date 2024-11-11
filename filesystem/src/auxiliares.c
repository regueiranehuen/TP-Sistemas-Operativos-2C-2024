#include "includes/auxiliares.h"

void inicializar_estructuras(void)
{
    sem_init(&sem_fin_filesystem, 0, 0);
    sem_init(&sem_conexion_hecha, 0, 0);

    config = config_create("filesystem.config");
    t_log_level log_level_int = log_level(config);
    log_filesystem = log_create("filesystem.log", "tp", true, log_level_int);

    path = config_get_string_value(config, "MOUNT_DIR");
    block_count = config_get_int_value(config, "BLOCK_COUNT");

    bitmap = cargar_bitmap(path, block_count);
}