#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include "utils/includes/estructuras.h"
#include "utils/includes/serializacion.h"
#include "server.h"
#include <fcntl.h>

// void inicializar_bitmap(t_bitarray *bitmap);
// void cerrar_bitmap(t_bitarray *bitmap);
t_bitarray* cargar_bitmap(char* mount_dir, uint32_t block_count);
int crear_archivo_dump(t_args_dump_memory* info, t_bitarray* bitmap, const char* mount_dir, uint32_t block_size);
bool hay_espacio_disponible(t_bitarray* bitmap, int bloques_necesarios);
void reservar_bloque(t_bitarray* bitmap, uint32_t* bloques_reservados, uint32_t bloques_necesarios, const char* filepath);
int crear_archivo_metadata(char* filepath, t_args_dump_memory* info, uint32_t* bloque_reservados, uint32_t bloques_necesarios);
int escribir_bloques(const char* mount_dir, uint32_t* bloque_reservados, uint32_t bloques_necesarios, t_args_dump_memory* info, int block_size);
void escribir_bloque_de_puntero(int bloques_fd, uint32_t* bloques_reservados, uint32_t bloques_necesarios, int bloque_size);







#endif