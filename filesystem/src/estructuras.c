#include "includes/estructuras.h"

t_bitarray* cargar_bitmap(char* mount_dir, uint32_t block_count) {

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/bitmap.dat", mount_dir);
    log_info(log_filesystem, "Cargando bitmap desde %s", filepath);

    FILE* file = fopen(filepath, "rb");
    struct stat st;
    stat(filepath, &st);
    uint32_t expected_size = (uint32_t)ceil((double)block_count / 8.0);
    if (st.st_size != expected_size) {
        log_error(log_filesystem, "El tamaño del archivo %s no es el esperado. Esperado: %u, Actual: %lu", filepath, expected_size, st.st_size);
        fclose(file);
        return NULL;
    }
    char* bitarray_data = malloc(st.st_size);

    
    fread(bitarray_data, 1, st.st_size, file);
    fclose(file);

    t_bitarray* bitmap = bitarray_create_with_mode(bitarray_data, st.st_size, LSB_FIRST);

    return bitmap;
}


int crear_archivo_dump(t_args_dump_memory* info, t_bitarray* bitmap, const char* mount_dir, uint32_t block_size) {
    int bloques_necesarios = ceil((info->tamanio_proceso / block_size)) + 1;
    // 1. verifico si hay espacio disponible
    if(!hay_espacio_disponible(bitmap, bloques_necesarios)){ 
        return -1;
    }
    
    // 2. reservo los bloques necesarios
    uint32_t* bloque_reservado = malloc((bloques_necesarios) * sizeof(uint32_t));
    char filepath[256];
    time_t now = time(NULL);
    snprintf(filepath, sizeof(filepath), "%s/%d-%d-%ld.dmp", mount_dir, info->pid, info->tid, now);
    reservar_bloque(bitmap, bloque_reservado, bloques_necesarios, filepath);

    // 3. Creo el archivo
    if (crear_archivo_metadata(filepath, info, bloque_reservado, bloques_necesarios) != 0) {
        free(bloque_reservado);
        return -1;
    }
    
    log_info(log_filesystem, "## Archivo Creado: %s - Tamaño: %d", filepath, info->tamanio_proceso);

    // 4. escribo el contenido en los bloques reservados
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

void reservar_bloque(t_bitarray* bitmap, uint32_t* bloques_reservados, uint32_t bloques_necesarios, const char* filepath) {
    int contador_reserva = 0;
    int bloques_libres = 0;

    for(int i = 0; i < bitarray_get_max_bit(bitmap); i++) {
        if(!bitarray_test_bit(bitmap, i)) {
            bloques_libres++;
        }
    }
    uint32_t* bloque_reservado = malloc((bloques_necesarios) * sizeof(uint32_t));
    


    for(int i = 0; i < bitarray_get_max_bit(bitmap) && contador_reserva < bloques_necesarios; i++) {
        if(!bitarray_test_bit(bitmap, i)) {
            bitarray_set_bit(bitmap, i);
            bloques_reservados[contador_reserva] = i; // ?
            contador_reserva++;
            bloques_libres--;

            // Log de asignación de bloque
            log_info(log_filesystem, "## Bloque asignado: %d - Archivo: %s - Bloques Libres: %d", i, filepath, bloques_libres);

            // Log de acceso a bloque
            const char* tipo_bloque = (contador_reserva == 1) ? "ÍNDICE" : "DATOS";
            log_info(log_filesystem, "## Acceso Bloque - Archivo: %s - Tipo Bloque: %s - Bloque File System %d", filepath, tipo_bloque, i);
        }
    }
}

int crear_archivo_metadata(char* filepath, t_args_dump_memory* info, uint32_t* bloque_reservados, uint32_t bloques_necesarios) {
    FILE* archivo_metadata = fopen(filepath, "w");
    if (archivo_metadata == NULL) {
        log_error(log_filesystem, "Error al crear el archivo de metadata");
        return -1;
    }
    log_info(log_filesystem, "TAMANIO=%d\n", info->tamanio_proceso);
    log_info(log_filesystem, "BLOQUES=[");
    for (uint32_t i = 1; i < bloques_necesarios; i++) {
        log_info(log_filesystem, "%u", bloque_reservados[i]);
        if (i < bloques_necesarios - 1) {
            log_info(log_filesystem, ",");
        }
    }
    log_info(log_filesystem, "]\n");
    fclose(archivo_metadata);
    return 0;
}

int escribir_bloques(const char* mount_dir, uint32_t* bloque_reservados, uint32_t bloques_necesarios, t_args_dump_memory* info, int block_size) {
    int retardo = config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");

    char bloques_filepath[256];
    snprintf(bloques_filepath, sizeof(bloques_filepath), "%s/bloques.dat", mount_dir);
    int bloques_fd = open(bloques_filepath, O_WRONLY);
    if (bloques_fd == -1) {
        log_error(log_filesystem, "Error al abrir el archivo bloques.dat");
        return -1;
    }

    escribir_bloque_de_puntero(bloques_fd, bloque_reservados, bloques_necesarios,block_size);

    //cambiar info->lista_datos --> usar list_get + if para comprobar que el dato entra en el bloque
    uint32_t bytes_written = 0;
    for (uint32_t i = 0; i < bloques_necesarios; i++) {
        uint32_t block_index = bloque_reservados[i];
        off_t offset = block_index * block_size;
        uint32_t bytes_to_write = (info->tamanio_proceso - bytes_written > block_size) ? block_size : info->tamanio_proceso - bytes_written;
        if (pwrite(bloques_fd, info->lista_datos + bytes_written, bytes_to_write, offset) != bytes_to_write) {
            log_error(log_filesystem, "Error al escribir en el bloque");
            close(bloques_fd);
            return -1;
        }
        bytes_written += bytes_to_write;
        usleep(retardo); // Retardo de acceso al bloque x config
    }

    close(bloques_fd);
    return 0;
}

void escribir_bloque_de_puntero(int bloques_fd, uint32_t* bloques_reservados, uint32_t bloques_necesarios, int bloque_size){

    uint32_t bloque_puntero = bloques_reservados[0];
    off_t offset = bloque_puntero * bloque_size;

    for(uint32_t i = 1; i < bloques_necesarios; i++) {
    if(pwrite(bloques_fd, &bloques_reservados[i], sizeof(uint32_t), offset) != sizeof(uint32_t)){
        log_error(log_filesystem, "Error al escribir en el bloque de punteros");
        }
        offset += sizeof(uint32_t);
    }
}

