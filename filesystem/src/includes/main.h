#ifndef MAIN_H
#define MAIN_H

#include "server.h"


extern sem_t sem_fin_filesystem;
extern t_log* log_filesystem;
extern t_config* config;
extern t_bitarray* bitmap;

extern char* path;
extern uint32_t block_size;
extern int block_count;

#endif