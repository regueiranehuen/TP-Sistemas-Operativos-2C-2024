#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include "utils/includes/estructuras.h"
#include "utils/includes/serializacion.h"
#include "server.h"
#include "main.h"
#include <fcntl.h> // La idea sería dejar de usar esta librería

// void inicializar_bitmap(t_bitarray *bitmap);
// void cerrar_bitmap(t_bitarray *bitmap);
t_bitarray* cargar_bitmap(char* mount_dir, uint32_t block_count);
char* crear_archivo_dump(t_args_dump_memory* info, t_bitarray* bitmap, const char* mount_dir, uint32_t block_size);
bool hay_espacio_disponible(t_bitarray* bitmap, int bloques_necesarios);
void reservar_bloque(t_bitarray* bitmap, uint32_t* bloques_reservados, uint32_t bloques_necesarios,char* filepath, int* index_bloque_indices, char* nombre_arch);
int crear_archivo_metadata(char* filepath, t_args_dump_memory* info,int index_bloque_indices, char* nombre_arch);
int escribir_bloques(const char* mount_dir, uint32_t* bloque_reservados, uint32_t bloques_necesarios, t_args_dump_memory* info, int block_size);
void escribir_bloque_de_puntero(FILE* arch, uint32_t* bloques_reservados, uint32_t bloques_necesarios, int bloque_size);
void imprimir_contenido_bitmap(t_bitarray* bitmap, uint32_t block_count);
void imprimir_archivo_bloques(const char* mount_dir);
void mostrar_contenido_archivo_metadata(const char* filepath);







#endif