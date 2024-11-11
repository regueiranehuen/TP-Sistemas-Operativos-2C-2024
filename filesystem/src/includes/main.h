#ifndef MAIN_H
#define MAIN_H

#include "server.h"

extern t_log* log_filesystem;
extern t_config* config;
extern t_bitarray* bitmap;

extern char* path;
extern int block_count; 
extern int block_size;

#endif