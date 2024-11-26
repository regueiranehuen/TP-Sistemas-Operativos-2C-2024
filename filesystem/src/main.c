#include "includes/main.h"

t_log *log_filesystem;
t_config *config;
sem_t sem_fin_filesystem;
pthread_mutex_t mutex_bitmap;
t_bitarray *bitmap;
char *mount_dir;
int block_count;
uint32_t block_size;

char* bitmap_path;
char* ruta_completa;

int main(int argc, char *argv[]){

    int socket_servidor;
    estado_filesystem = 1;
    
    inicializar_estructuras();

    socket_servidor = hilo_filesystem(log_filesystem, config);
    sem_wait(&sem_fin_filesystem);

    leer_archivo("/home/utnso/tp-2024-2c-Go-se-fue-C-queda/mount_dir/bitmap.dat"); //Es para probar nomas
    leer_archivo("/home/utnso/tp-2024-2c-Go-se-fue-C-queda/mount_dir/bloques.dat");
    leer_archivo("/home/utnso/tp-2024-2c-Go-se-fue-C-queda/mount_dir/files/");

    config_destroy(config);
    log_destroy(log_filesystem);
    close(socket_servidor);
    //crear una func que borre el bitmap cada vez que se vuelva a correr
    //borrar el crear_archivo_dump
    //borrar el bloques.dat

    return 0;
}

void leer_archivo(const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "rb"); // Abrir en modo lectura binaria
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    // Obtener tamaño del archivo
    fseek(archivo, 0, SEEK_END);
    long tamano = ftell(archivo);
    rewind(archivo);

    // Leer contenido del archivo
    char *contenido = (char *)malloc(tamano + 1);
    if (contenido == NULL) {
        perror("Error al asignar memoria");
        fclose(archivo);
        return;
    }

    size_t leidos = fread(contenido, 1, tamano, archivo);
    if (leidos != tamano) {
        perror("Error al leer el archivo");
        free(contenido);
        fclose(archivo);
        return;
    }

    contenido[tamano] = '\0'; // Asegurarse de que el contenido esté terminado en NULL para cadenas

    // Imprimir contenido
    printf("Contenido de '%s':\n", nombre_archivo);
    printf("%s\n", contenido);

    // Liberar memoria y cerrar archivo
    free(contenido);
    fclose(archivo);
}
