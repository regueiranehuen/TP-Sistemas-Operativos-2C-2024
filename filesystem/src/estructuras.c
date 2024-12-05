#include "includes/estructuras.h"

t_bitarray* cargar_bitmap(char* mount_dir, uint32_t block_count) {
    int length_path = strlen(mount_dir) + 1 + 12; // "/bitmap.dat" = 12 caracteres
    bitmap_path = malloc(length_path);
    if (!bitmap_path) {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "Error al asignar memoria para la ruta del bitmap");
        pthread_mutex_unlock(&mutex_logs);
        return NULL;
    }
    snprintf(bitmap_path, length_path, "%sbitmap.dat", mount_dir);
    pthread_mutex_lock(&mutex_logs);
    log_info(log_filesystem, "Cargando bitmap desde %s", bitmap_path);
    pthread_mutex_unlock(&mutex_logs);

    FILE* arch = fopen(bitmap_path, "rb");

    if (arch == NULL) {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_filesystem, "No se encontró el archivo bitmap.dat, lo creamos");
        pthread_mutex_unlock(&mutex_logs);

        // Crear el archivo vacío con el tamaño adecuado
        arch = fopen(bitmap_path, "wb");
        if (arch == NULL) {
            pthread_mutex_lock(&mutex_logs);
            log_error(log_filesystem, "Error al crear el archivo bitmap.dat");
            pthread_mutex_unlock(&mutex_logs);
            free(bitmap_path);
            return NULL;
        }

        // Calcular el tamaño del bitmap
        uint32_t bitmap_size = (uint32_t)ceil((double)block_count / 8.0);
        pthread_mutex_lock(&mutex_logs);
        log_info(log_filesystem, "Tamaño del bitmap: %u bytes", bitmap_size);
        pthread_mutex_unlock(&mutex_logs);
        char* empty_bitmap = calloc(bitmap_size, sizeof(char));
        if (!empty_bitmap) {
            pthread_mutex_lock(&mutex_logs);
            log_error(log_filesystem, "Error al asignar memoria para el bitmap vacío");
            pthread_mutex_unlock(&mutex_logs);
            fclose(arch);
            free(bitmap_path);
            return NULL;
        }

        // Escribir el bitmap vacío en el archivo
        if (fwrite(empty_bitmap, 1, bitmap_size, arch) != bitmap_size) {
            pthread_mutex_lock(&mutex_logs);
            log_error(log_filesystem, "Error al escribir el bitmap vacío en el archivo");
            pthread_mutex_unlock(&mutex_logs);
            free(empty_bitmap);
            fclose(arch);
            free(bitmap_path);
            return NULL;
        }
        free(empty_bitmap);
        fclose(arch);

        // Reabrir el archivo para lectura
        arch = fopen(bitmap_path, "rb");
        if (arch == NULL) {
            pthread_mutex_lock(&mutex_logs);
            log_error(log_filesystem, "Error al reabrir el archivo bitmap.dat después de crearlo");
            pthread_mutex_unlock(&mutex_logs);
            free(bitmap_path);
            return NULL;
        }
    }

    // Leer el tamaño del archivo y validarlo
    struct stat st;
    if (stat(bitmap_path, &st) != 0) {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "Error al obtener el tamaño del archivo bitmap.dat");
        pthread_mutex_unlock(&mutex_logs);
        fclose(arch);
        free(bitmap_path);
        return NULL;
    }

    uint32_t expected_size = (uint32_t)ceil((double)block_count / 8.0);
    if ((uint32_t)st.st_size != expected_size) {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "El tamaño del archivo %s no es el esperado. Esperado: %u, Actual: %lu", 
                  bitmap_path, expected_size, st.st_size);
        pthread_mutex_unlock(&mutex_logs);
        fclose(arch);
        free(bitmap_path);
        return NULL;
    }

    // Cargar el contenido del archivo en memoria
    char* bitarray_data = malloc(st.st_size);
    if (!bitarray_data) {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "Error al asignar memoria para el contenido del bitmap");
        pthread_mutex_unlock(&mutex_logs);
        fclose(arch);
        free(bitmap_path);
        return NULL;
    }

    if (fread(bitarray_data, 1, st.st_size, arch) != st.st_size) {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "Error al leer el contenido del archivo bitmap.dat");
        pthread_mutex_unlock(&mutex_logs);
        free(bitarray_data);
        fclose(arch);
        free(bitmap_path);
        return NULL;
    }
    fclose(arch);

    // Crear el bitarray
    t_bitarray* bitmap = bitarray_create_with_mode(bitarray_data, st.st_size, LSB_FIRST);
    //imprimir_contenido_bitmap(bitmap, block_count);

    
    //free(bitmap_path);
    return bitmap;
}

void imprimir_contenido_bitmap(t_bitarray* bitmap, uint32_t block_count) {
    pthread_mutex_lock(&mutex_bitmap);
    if (!bitmap) {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "Bitmap no cargado");
        pthread_mutex_unlock(&mutex_logs);
        pthread_mutex_unlock(&mutex_bitmap);
        return;
    }

    for (uint32_t i = 0; i < block_count; i++) {
        int bit = bitarray_test_bit(bitmap, i);
        pthread_mutex_lock(&mutex_logs);
        log_info(log_filesystem,"Bloque %u: %s\n", i, bit ? "Ocupado" : "Libre");
        pthread_mutex_unlock(&mutex_logs);
    }
    pthread_mutex_unlock(&mutex_bitmap);
}

char* crear_archivo_dump(t_args_dump_memory* info, t_bitarray* bitmap, const char* mount_dir, uint32_t block_size) {
    int bloques_necesarios = ceil((info->tamanio_particion_proceso / block_size)) + 1;

    // 1. Verifico si hay espacio disponible
    pthread_mutex_lock(&mutex_bitmap);
    if (!hay_espacio_disponible(bitmap, bloques_necesarios)) {
        pthread_mutex_unlock(&mutex_bitmap);
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "No hay espacio suficiente para el archivo dump");
        pthread_mutex_unlock(&mutex_logs);
        return NULL;
    }
    pthread_mutex_unlock(&mutex_bitmap);
    // 2. Reservo los bloques necesarios
    char* filepath = malloc(256);
    char* files = "files";
    char* nombre_arch = malloc(256);
    ruta_completa = malloc(256);

    time_t now = time(NULL);
    snprintf(nombre_arch, 256, "%i-%i-%ld.dmp", info->pid, info->tid, now);
    snprintf(filepath, 256, "%s", mount_dir);
    snprintf(ruta_completa, 256, "%s%s/%s", filepath, files, nombre_arch);
    
    uint32_t* bloques_reservados = malloc((bloques_necesarios) * sizeof(uint32_t));
    int* index_bloque_indices = malloc(sizeof(int));
    *index_bloque_indices = -1;
    reservar_bloque(bitmap, bloques_reservados, bloques_necesarios, filepath, index_bloque_indices, nombre_arch);
    

    int indice_bloque_indices = *index_bloque_indices;
    free(index_bloque_indices);

    if (indice_bloque_indices == -1) {
        filepath = NULL;
        return filepath;
    }

    // Mostrar el nombre del archivo
    pthread_mutex_lock(&mutex_logs);
    log_info(log_filesystem, "Nombre del archivo creado: %s", nombre_arch);
    pthread_mutex_unlock(&mutex_logs);

    // Crear el archivo de metadata
    if (crear_archivo_metadata(ruta_completa, info, indice_bloque_indices, nombre_arch) != 0) {
        free(bloques_reservados);
        ruta_completa = NULL;
        return ruta_completa;
    }
    pthread_mutex_lock(&mutex_logs);
    log_info(log_filesystem, "Archivo de metadata creado: %s - Tamaño: %d", ruta_completa, info->tamanio_particion_proceso);
    pthread_mutex_unlock(&mutex_logs);

    // 4. Escribir el contenido en los bloques reservados
    if (escribir_bloques(mount_dir, bloques_reservados, bloques_necesarios, info, block_size) != 0) {
        free(bloques_reservados);
        filepath = NULL;
        return filepath;
    }

    // FILE* archivo_dump = fopen(ruta_completa, "w");
    // fwrite(info->contenido, info->tamanio_particion_proceso, 1, archivo_dump); // Creación del archivo dump del proceso y grabación del contenido
    // fclose(archivo_dump);

    free(bloques_reservados);

    // Mostrar el contenido del archivo metadata
    

    imprimir_contenido_bitmap(bitmap, block_count);
    free(nombre_arch);
    free(filepath);
    return ruta_completa;
}

void mostrar_contenido_archivo_metadata(const char* filepath) {
    FILE* archivo = fopen(filepath, "r");
    if (archivo == NULL) {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "No se pudo abrir el archivo metadata: %s", filepath);
        pthread_mutex_unlock(&mutex_logs);
        return;
    }

    pthread_mutex_lock(&mutex_logs);
    log_info(log_filesystem, "Contenido del archivo metadata (%s):", filepath);
    pthread_mutex_unlock(&mutex_logs);

    char linea[256];
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_filesystem,"%s", linea);
        pthread_mutex_unlock(&mutex_logs);  
    }

    fclose(archivo);
}


bool hay_espacio_disponible(t_bitarray* bitmap, int bloques_necesarios) {
    int bloques_libres = 0;

    
    for(int i = 0; i < block_count; i++) {
        
        if(!bitarray_test_bit(bitmap, i)) {
            bloques_libres++;
        }
        
        if(bloques_libres >= bloques_necesarios) {
            return true;
        }
    }
    pthread_mutex_lock(&mutex_logs);
    log_warning(log_filesystem, "BLOQUES LIBRES: %d, : BLOQUES NECESARIOS: %d", bloques_libres, bloques_necesarios);
    pthread_mutex_unlock(&mutex_logs);

    return false;
}

void reservar_bloque(t_bitarray* bitmap, uint32_t* bloques_reservados, uint32_t bloques_necesarios,char* filepath, int* index_bloque_indices, char* nombre_arch) {
    int contador_reserva = 0;
    int bloques_libres = 0;

    bool primero=true;

    
    pthread_mutex_lock(&mutex_bitmap);
    for(int i = 0; i < bitarray_get_max_bit(bitmap); i++) {
        if(!bitarray_test_bit(bitmap, i)) {
            bloques_libres++;
        }
    }
    
    for(int i = 0; i < bitarray_get_max_bit(bitmap) && contador_reserva < bloques_necesarios; i++) {
        if(!bitarray_test_bit(bitmap, i)) {
            
            bitarray_set_bit(bitmap, i);

            bloques_reservados[contador_reserva] = i; // ?

            if (primero){ // Al primer bloque que encuentro lo vuelvo bloque de indices
                *index_bloque_indices=i; // Guardo la posicion del bloque de indices en el bitmap. Despues uso esto para crear el archivo de metadata
                primero=false;
            }

            contador_reserva++;
            bloques_libres--;

            // Log de asignación de bloque
            pthread_mutex_lock(&mutex_logs);
            log_debug(log_filesystem, "## Bloque asignado: %d - Archivo: %s - Bloques Libres: %d", i, nombre_arch, bloques_libres);
            pthread_mutex_unlock(&mutex_logs);

            // Log de acceso a bloque
            const char* tipo_bloque = (contador_reserva == 1) ? "ÍNDICE" : "DATOS";
            pthread_mutex_lock(&mutex_logs);
            log_debug(log_filesystem, "## Acceso Bloque - Archivo: %s - Tipo Bloque: %s - Bloque File System %d", nombre_arch, tipo_bloque, i);
            pthread_mutex_unlock(&mutex_logs);
        }
    }
    
    FILE*arch=fopen(bitmap_path,"wb");
    fwrite(bitmap->bitarray,bitmap->size,1,arch);//escribir en el archivo los bits modificados
    pthread_mutex_unlock(&mutex_bitmap);
    fclose(arch);
    
}

int crear_archivo_metadata(char* filepath, t_args_dump_memory* info, int index_bloque_indices, char* nombre_arch) {
    
    // Crear el archivo de metadata
    pthread_mutex_lock(&mutex_logs);
    log_info(log_filesystem, "Creando archivo de metadata %s", filepath);
    pthread_mutex_unlock(&mutex_logs);
    FILE* archivo_metadata = fopen(filepath, "w");
    
    if (archivo_metadata == NULL) {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "Error al crear el archivo de metadata: %s", strerror(errno));
        pthread_mutex_unlock(&mutex_logs);
        return -1;
    }

    // Escribir los valores de SIZE y INDEX_BLOCK en el archivo de metadata
    fprintf(archivo_metadata, "SIZE=%d\n", info->tamanio_particion_proceso);  // Escribir el tamaño del archivo
    fprintf(archivo_metadata, "INDEX_BLOCK=%d\n", index_bloque_indices);      // Escribir el índice de bloque

    // Asegurarse de que los datos se escriban correctamente al archivo
    fflush(archivo_metadata);
    
    // Verificar si ocurrió algún error de escritura
    if (ferror(archivo_metadata)) {
        pthread_mutex_lock(&mutex_logs);
        log_error(log_filesystem, "Error al escribir en el archivo de metadata");
        pthread_mutex_unlock(&mutex_logs);
        fclose(archivo_metadata);
        return -1;
    }

    fclose(archivo_metadata);

    // Mostrar el contenido del archivo de metadata para verificar su escritura
    mostrar_contenido_archivo_metadata(ruta_completa);

    return 0;
}


int escribir_bloques(const char* mount_dir, uint32_t* bloques_reservados, uint32_t bloques_necesarios, t_args_dump_memory* info, int block_size) {

    int retardo = config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");

    int length_path = strlen(mount_dir)+1;
    length_path += 12;
    char* path = malloc(length_path);//<-- Aura
    
    snprintf(path, length_path, "%s/bloques.dat", mount_dir);

    FILE*arch = fopen(path,"r+b"); // Abrimos el archivo en modo lectura/escritura y apuntamos al principio

    if (arch == NULL) {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_filesystem, "No se encontró el archivo bloques.dat, lo creamos");
        pthread_mutex_unlock(&mutex_logs);

        // Crear el archivo con el tamaño adecuado sin redimensionarlo después
        uint32_t file_size = block_count * block_size;

        // Abrir el archivo en modo binario de escritura
        arch = fopen(path, "wb");
        if (arch == NULL) {
            pthread_mutex_lock(&mutex_logs);
            log_error(log_filesystem, "Error al crear el archivo bloques.dat");
            pthread_mutex_unlock(&mutex_logs);
            free(path);
            return -1;
        }

        // Establecer el tamaño del archivo directamente
        fseek(arch, file_size - 1, SEEK_SET);
        fputc(0, arch); // Escribir un byte en la última posición para asegurar el tamaño
    }

    escribir_bloque_de_puntero(arch, bloques_reservados, bloques_necesarios, block_size);

    uint32_t bytes_written = 0;

    void* puntero = info->contenido;

    for (int i = 1; i < bloques_necesarios; i++) {
        uint32_t block_index = bloques_reservados[i];
        off_t offset = block_index * block_size;

        fseek(arch,offset,SEEK_SET);  // Desde el principio del archivo, me desplazo offset
        
        uint32_t bytes_to_write = bytes_a_escribir(info,bytes_written);
        
        if (fwrite(puntero,bytes_to_write,1,arch)!=1){
            pthread_mutex_lock(&mutex_logs);
            log_error(log_filesystem, "Error al escribir en el bloque");
            pthread_mutex_unlock(&mutex_logs);
            fclose(arch);
            return -1;
        }
        bytes_written += bytes_to_write;
        puntero += bytes_to_write;

        usleep(retardo);
    }

    fclose(arch);
    imprimir_archivo_bloques(mount_dir);
    free(path);
    return 0;
}

void escribir_bloque_de_puntero(FILE* arch, uint32_t* bloques_reservados, uint32_t bloques_necesarios, int bloque_size) {
    int indice_bloque_puntero = bloques_reservados[0];
    off_t offset = indice_bloque_puntero * bloque_size;
    fseek(arch, offset, SEEK_SET);
    for (int i = 1; i < bloques_necesarios; i++) {
        if (fwrite(&bloques_reservados[i], sizeof(uint32_t), 1, arch) != 1) {
            pthread_mutex_lock(&mutex_logs);
            log_error(log_filesystem, "Error al escribir en el bloque de punteros");
            pthread_mutex_unlock(&mutex_logs);
        }
    }
}


void imprimir_archivo_bloques(const char* filepath) {
    
    FILE* arch = fopen(filepath, "rb");
    if (arch == NULL) {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_filesystem,"Error: No se pudo abrir el archivo %s\n", filepath);
        pthread_mutex_lock(&mutex_logs);
        return;
    }
    pthread_mutex_lock(&mutex_logs);
    log_info(log_filesystem,"Contenido del archivo bloques.dat:");
    pthread_mutex_unlock(&mutex_logs);
    uint32_t total_bloques = block_count; // Bloques necesarios según block_count
    uint32_t bloque_index = 0;
    uint8_t* buffer = malloc(block_size);
    if (!buffer) {
        pthread_mutex_lock(&mutex_logs);
        log_info(log_filesystem,"Error: No se pudo asignar memoria para el buffer");
        pthread_mutex_unlock(&mutex_logs);
        fclose(arch);
        return;
    }

    // Leer y mostrar bloques hasta total_bloques
    while (fread(buffer, 1, block_size, arch) == block_size && bloque_index < total_bloques) {
        log_info(log_filesystem,"Bloque %u:\n", bloque_index);
        for (uint32_t i = 0; i < block_size; i++) {
            if (i % 16 == 0) {
            pthread_mutex_lock(&mutex_logs);
            log_info(log_filesystem,"\n%04X: ", i); // Imprime la dirección de inicio de la línea
            pthread_mutex_unlock(&mutex_logs);
            }
            pthread_mutex_lock(&mutex_logs);
            log_info(log_filesystem,"%02X ", buffer[i]); // Imprime el byte en formato hexadecimal
            pthread_mutex_unlock(&mutex_logs);
        }
        bloque_index++;
    }

    free(buffer);
    fclose(arch);
    pthread_mutex_lock(&mutex_logs);
    log_info(log_filesystem,"Fin del archivo bloques.dat\n");
    pthread_mutex_unlock(&mutex_logs);
}