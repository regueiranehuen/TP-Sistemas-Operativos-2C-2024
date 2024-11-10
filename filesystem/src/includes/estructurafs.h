#ifndef ESTRUCTURAFS_H
#define ESTRUCTURAFS_H

#include "utils/includes/estructuras.h"
#include "utils/includes/serializacion.h"
#include "server.h"
#include <fcntl.h>

// void inicializar_bitmap(t_bitarray *bitmap);
// void cerrar_bitmap(t_bitarray *bitmap);
bool hay_espacio_disponible(t_bitarray* bitmap, int bloques_necesarios);
void reservar_bloque(t_bitarray* bitmap, size_t* reserved_blocks, size_t bloques_necesarios);
int crear_archivo_metadata(char* filepath, t_args_dump_memory* info, size_t* reserved_blocks, size_t bloques_necesarios);
int escribir_bloques(const char* mount_dir, size_t* reserved_blocks, size_t bloques_necesarios, t_args_dump_memory* info, int block_size);
int crear_archivo_dump(t_args_dump_memory* info, t_bitarray* bitmap, const char* mount_dir, int block_size);








#endif


