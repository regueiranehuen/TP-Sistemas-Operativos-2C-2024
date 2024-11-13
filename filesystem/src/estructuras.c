#include "includes/estructuras.h"

t_bitarray* cargar_bitmap(char* mount_dir, uint32_t block_count) {

    int length_path = strlen(mount_dir)+1;
    length_path += 12;
    bitmap_path = malloc(length_path);//<-- Aura
 
    snprintf(bitmap_path, length_path, "%s/bitmap.dat", mount_dir);
    log_info(log_filesystem, "Cargando bitmap desde %s", bitmap_path);

    FILE*arch = fopen(bitmap_path,"rb");

    if (arch == NULL) {
        log_info(log_filesystem, "No se encontró el archivo bitmap.dat, lo creamos");
        arch = fopen(bitmap_path,"w+b");
        if (arch == NULL) {
            log_error(log_filesystem, "Error al crear el archivo bitmap.dat");
            return NULL;
        }
    }

    struct stat st;
    stat(bitmap_path, &st);
    uint32_t expected_size = (uint32_t)ceil((double)block_count / 8.0);
    if (st.st_size != expected_size) {
        log_error(log_filesystem, "El tamaño del archivo %s no es el esperado. Esperado: %u, Actual: %lu", bitmap_path, expected_size, st.st_size);
        fclose(arch);
        return NULL;
    }
    char* bitarray_data = malloc(st.st_size);

    fread(bitarray_data, 1, st.st_size, arch);
    fclose(arch);

    t_bitarray* bitmap = bitarray_create_with_mode(bitarray_data, st.st_size, LSB_FIRST);
    
    return bitmap;
}

char* crear_archivo_dump(t_args_dump_memory* info, t_bitarray* bitmap, const char* mount_dir, uint32_t block_size) {

    int bloques_necesarios = ceil((info->tamanio_particion_proceso / block_size)) + 1;
    // 1. verifico si hay espacio disponible
    pthread_mutex_lock(&mutex_bitmap);
    if(!hay_espacio_disponible(bitmap, bloques_necesarios)){ 
        return NULL;
    }
    // 2. reservo los bloques necesarios
    char*filepath = malloc(256);

    time_t now = time(NULL);
    snprintf(filepath, 256, "%s/%d-%d-%ld.dmp", mount_dir, info->pid, info->tid, now);
    uint32_t* bloques_reservados = malloc((bloques_necesarios) * sizeof(uint32_t));
    int*index_bloque_indices=malloc(sizeof(int));
    *index_bloque_indices=-1;
    reservar_bloque(bitmap, bloques_reservados, bloques_necesarios, filepath, index_bloque_indices);
    pthread_mutex_unlock(&mutex_bitmap);

    
    int indice_bloque_indices=*index_bloque_indices;
    free(index_bloque_indices);

    if (indice_bloque_indices == -1){
        filepath = NULL;
        return filepath;
    }
    //Crear el archivo de metadata con los datos requeridos y el siguiente formato: <PID>-<TID>-<TIMESTAMP>.dmp.
    // 3. Creo el archivo
    if (crear_archivo_metadata(filepath, info,indice_bloque_indices) != 0) {
        free(bloques_reservados);
        filepath = NULL;
        return filepath;
    }
    log_info(log_filesystem, "## Archivo Creado: %s - Tamaño: %d", filepath, info->tamanio_particion_proceso);

    // 4. escribo el contenido en los bloques reservados
    if (escribir_bloques(mount_dir, bloques_reservados, bloques_necesarios, info, block_size) != 0) {
        free(bloques_reservados);
        filepath = NULL;
        return filepath;
    }
    FILE* archivo_dump = fopen(filepath,"w");
    fwrite(info->contenido,info->tamanio_particion_proceso,1,archivo_dump);//creación del archivo dump del proceso y grabación del contenido
    log_info(log_filesystem,"## Archivo Creado: %s - Tamaño: %d",filepath,info->tamanio_particion_proceso);
    fclose(archivo_dump);
    free(bloques_reservados);
    return filepath;
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

void reservar_bloque(t_bitarray* bitmap, uint32_t* bloques_reservados, uint32_t bloques_necesarios,char* filepath, int* index_bloque_indices) {
    int contador_reserva = 0;
    int bloques_libres = 0;

    bool primero=true;

    

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
            log_info(log_filesystem, "## Bloque asignado: %d - Archivo: %s - Bloques Libres: %d", i, filepath, bloques_libres);

            // Log de acceso a bloque
            const char* tipo_bloque = (contador_reserva == 1) ? "ÍNDICE" : "DATOS";
            log_info(log_filesystem, "## Acceso Bloque - Archivo: %s - Tipo Bloque: %s - Bloque File System %d", filepath, tipo_bloque, i);
        }
    }
    
    FILE*arch=fopen(bitmap_path,"wb");
    fwrite(bitmap->bitarray,bitmap->size,1,arch);//escribir en el archivo los bits modificados

    fclose(arch);
    
}

int crear_archivo_metadata(char* filepath, t_args_dump_memory* info,int index_bloque_indices) {
    FILE* archivo_metadata = fopen(filepath, "w");

    if (archivo_metadata == NULL) {
        log_error(log_filesystem, "Error al crear el archivo de metadata");
        return -1;
    }
    
    fwrite(&info->tamanio_particion_proceso,sizeof(int),1,archivo_metadata); // Tamanio del archivo a crear para el proceso
    fwrite(&index_bloque_indices,sizeof(int),1,archivo_metadata); // Número de bloque que corresponde al bloque de índices

    fclose(archivo_metadata);
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
        log_info(log_filesystem, "No se encontró el archivo bloques.dat, lo creamos");
        arch = fopen(path,"wb");
    }

    escribir_bloque_de_puntero(arch, bloques_reservados, bloques_necesarios, block_size);

    uint32_t bytes_written = 0;
    //[2,4,5,6]
    void* puntero = info->contenido;

    // tamanio_particion_proceso = 200 

    // block_size=20
    
    // bytes_written = 20

    for (int i = 1; i < bloques_necesarios; i++) {
        uint32_t block_index = bloques_reservados[i];
        off_t offset = block_index * block_size;

        fseek(arch,offset,SEEK_SET);  // Desde el principio del archivo, me desplazo offset

        
        uint32_t bytes_to_write = (info->tamanio_particion_proceso - bytes_written >= block_size) ? block_size : info->tamanio_particion_proceso - bytes_written;
         
        // Tanto el size del int (-1 de inicializacion) como el del uint32 es de 4 bytes, entonces la cantidad de datos a escribir en el bloque es: 
        uint32_t cant_datos_a_escribir = bytes_to_write / sizeof(uint32_t);  

        for (int j=0; j < cant_datos_a_escribir; j++){
            uint32_t dato = *(uint32_t*)puntero;
            
            if (fwrite(&dato, sizeof(uint32_t), 1, arch) != 1){ // fwrite devuelve la cantidad de elementos que escribís
                log_error(log_filesystem, "Error al escribir en el bloque");
                fclose(arch);
                return -1;
            }
            
            bytes_written += bytes_to_write;
            puntero += sizeof(uint32_t);
            
            usleep(retardo);
        }

    }

    fclose(arch);
    return 0;
}

void escribir_bloque_de_puntero(FILE* arch, uint32_t* bloques_reservados, uint32_t bloques_necesarios, int bloque_size) {
    int indice_bloque_puntero = bloques_reservados[0];
    off_t offset = indice_bloque_puntero * bloque_size;
    fseek(arch, offset, SEEK_SET);
    for (int i = 1; i < bloques_necesarios; i++) {
        if (fwrite(&bloques_reservados[i], sizeof(uint32_t), 1, arch) != 1) {
            log_error(log_filesystem, "Error al escribir en el bloque de punteros");
        }
    }
}