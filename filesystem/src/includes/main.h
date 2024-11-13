#ifndef MAIN_H
#define MAIN_H

#include "server.h"


extern sem_t sem_fin_filesystem;
extern pthread_mutex_t mutex_bitmap;
extern t_log* log_filesystem;
extern t_config* config;
extern t_bitarray* bitmap;

extern char* mount_dir;
extern uint32_t block_size;
extern int block_count;

extern char* bitmap_path;

#endif