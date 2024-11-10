#ifndef PETICIONES_H
#define PETICIONES_H

#include "server.h"
#include "includes/estructurafs.h"

void atender_conexiones(int socket_cliente);
t_bitarray* cargar_bitmap(char* path, int block_count);


#endif
