#include "includes/estructurafs.h"

t_bitarray* cargar_bitmap(char* mount_dir, size_t block_count) {
    // Construir la ruta completa del archivo bitmap.dat
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/bitmap.dat", mount_dir);
    log_info(log_filesystem, "Cargando bitmap desde %s", filepath);

    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        log_error(log_filesystem, "Error al abrir el archivo %s", filepath);
        return NULL;
    }

    // Obtener el tamaño del archivo
    struct stat st;
    if (stat(filepath, &st) != 0) {
        perror("Error al obtener el tamaño del archivo");
        log_error(log_filesystem, "Error al obtener el tamaño del archivo %s", filepath);
        fclose(file);
        return NULL;
    }
    size_t expected_size = (size_t)ceil((double)block_count / 8.0);
    if (st.st_size != expected_size) {
        log_error(log_filesystem, "Error: el tamaño del archivo no coincide con el tamaño esperado (%zu bytes)", expected_size);
        fclose(file);
        return NULL;
    }

    // Asignar memoria para el bitarray
    char* bitarray_data = malloc(st.st_size);
    if (bitarray_data == NULL) {
        perror("Error al asignar memoria para el bitarray");
        fclose(file);
        return NULL;
    }

    // Leer el contenido del archivo en el bitarray
    size_t read_size = fread(bitarray_data, 1, st.st_size, file);
    if (read_size != st.st_size) {
        perror("Error al leer el archivo");
        free(bitarray_data);
        fclose(file);
        return NULL;
    }

    fclose(file);

    // Crear e inicializar la estructura t_bitarray
    t_bitarray* bitmap = bitarray_create_with_mode(bitarray_data, st.st_size, LSB_FIRST); // o MSB_FIRST según tu necesidad
    if (bitmap == NULL) {
        perror("Error al crear el bitarray");
        free(bitarray_data);
        return NULL;
    }

    return bitmap;
}


//nombre, tamanio, contenido
int crear_archivo_dump(t_args_dump_memory* info, t_bitarray* bitmap, const char* mount_dir, int block_size) {
    
    int bloques_necesarios = (info->tamanio_proceso / block_size) + 1;
    // 1. verifico si hay espacio disponible
    if(!hay_espacio_disponible(bitmap, bloques_necesarios + 1)){
        return -1;
    }

    // 2. reservo los bloques necesarios
    size_t* bloque_reservado = malloc((bloques_necesarios + 1) * sizeof(size_t));
    reservar_bloque(bitmap, bloque_reservado, bloques_necesarios + 1);

    // 3. Creo el archivo
    char filepath[256];
    time_t now = time(NULL);
    snprintf(filepath, sizeof(filepath), "%s/%d-%d-%ld.dmp", mount_dir, info->pid, info->tid, now);
    if (crear_archivo_metadata(filepath, info, bloque_reservado, bloques_necesarios) != 0) {
        free(bloque_reservado);
        return -1;
    }

    //4. escribo el contenido en los bloques reservados
    if (escribir_bloques(mount_dir, bloque_reservado, bloques_necesarios, info, block_size) != 0) {
        free(bloque_reservado);
        return -1;
    }

    free(bloque_reservado);
    return 1;

}

bool hay_espacio_disponible(t_bitarray* bitmap, int bloques_necesarios) {
    int bloques_libres = 0;

    for(int i = 0; i < bitarray_get_max_bit(bitmap); i++) {
        if(!bitarray_test_bit(bitmap, i)) {
            bloques_libres++;
        }
        if(bloques_libres >= bloques_necesarios) {
            return true;
        }
    }

    return false;
}

void reservar_bloque(t_bitarray* bitmap, size_t* bloques_reservados, size_t bloques_necesarios) {
    int contador_reserva = 0;
    for(int i = 0; i < bitarray_get_max_bit(bitmap) && contador_reserva < bloques_necesarios; i++){
        if(!bitarray_test_bit(bitmap, i)) {
            bitarray_set_bit(bitmap, i);
            bloques_reservados[contador_reserva] = i;
            contador_reserva++;
        }
    }
}

int crear_archivo_metadata(char* filepath, t_args_dump_memory* info, size_t* bloque_reservados, size_t bloques_necesarios) {
    FILE* archivo_metadata = fopen(filepath, "w");
    if (archivo_metadata == NULL) {
        perror("Error al crear el archivo de metadata");
        return -1;
    }
    fprintf(archivo_metadata, "TAMANIO=%d\n", info->tamanio_proceso);
    fprintf(archivo_metadata, "BLOQUES=[");
    for (size_t i = 1; i < bloques_necesarios; i++) {
        fprintf(archivo_metadata, "%zu", bloque_reservados[i]);
        if (i < bloques_necesarios - 1) {
            fprintf(archivo_metadata, ",");
        }
    }
    fprintf(archivo_metadata, "]\n");
    fclose(archivo_metadata);
    return 0;
}

int escribir_bloques(const char* mount_dir, size_t* bloque_reservados, size_t bloques_necesarios, t_args_dump_memory* info, int block_size) {
    char bloques_filepath[256];
    snprintf(bloques_filepath, sizeof(bloques_filepath), "%s/bloques.dat", mount_dir);
    int bloques_fd = open(bloques_filepath, O_WRONLY);
    if (bloques_fd == -1) {
        perror("Error al abrir el archivo bloques.dat");
        return -1;
    }

    size_t bytes_written = 0;
    for (size_t i = 1; i < bloques_necesarios; i++) {
        size_t block_index = bloque_reservados[i];
        off_t offset = block_index * block_size;
        size_t bytes_to_write = (info->tamanio_proceso - bytes_written > block_size) ? block_size : info->tamanio_proceso - bytes_written;
        if (pwrite(bloques_fd, info->contenido + bytes_written, bytes_to_write, offset) != bytes_to_write) {
            perror("Error al escribir en el bloque");
            close(bloques_fd);
            return -1;
        }
        bytes_written += bytes_to_write;
        usleep(15000); // Retardo de acceso al bloque
    }

    close(bloques_fd);
    return 0;
}


// void inicializar_bitmap(t_bitarray *bitmap) {
//     int size = bitarray_get_max_bit(bitmap);

//     for(int i = 0; i < size; i++)
//         bitarray_clean_bit(bitmap, i);
// }

// void cerrar_bitmap(t_bitarray *bitmap) {
//     int size = bitarray_get_max_bit(bitmap);

//     munmap(bitmap->bitarray, size);
//     bitarray_destroy(bitmap);
// }