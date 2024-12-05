#ifndef MAIN_H
#define MAIN_H

#include "server.h"


extern sem_t sem_fin_filesystem;
extern sem_t sem_termina_hilo;
extern pthread_mutex_t mutex_bitmap;
extern pthread_mutex_t mutex_estado_filesystem;
extern pthread_mutex_t cliente_count_mutex;
extern pthread_mutex_t mutex_logs;
extern t_log* log_filesystem;
extern t_config* config;
extern t_bitarray* bitmap;

extern char* mount_dir;
extern uint32_t block_size;
extern int block_count;

extern char* bitmap_path;
extern char* ruta_completa;

void leer_archivo(const char *nombre_archivo);


#endif