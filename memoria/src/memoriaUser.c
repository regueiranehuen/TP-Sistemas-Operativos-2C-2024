#include "includes/memoriaUser.h"

int tamanio_memoria;
int retardo_restp;
int esquema;
char* algoritmo_busqueda;
char* particiones;

// Inicializar la memoria con un esquema de partición y tamaño de memoria
t_memoria* inicializar_memoria(t_esquema_particion esquema, int tamano, int* tamanos_particiones, int num_particiones) {
    t_memoria* memoria = malloc(sizeof(t_memoria));
    memoria->memoria = malloc(tamano);
    memset(memoria->memoria, 0, tamano);
    memoria->tabla_segmentos.segmentos = NULL;
    memoria->tabla_segmentos.num_segmentos = 0;
    memoria->tabla_libres.bloques_libres = NULL;
    memoria->tabla_libres.num_bloques_libres = 0;
    memoria->esquema = esquema;
    memoria->tamano_memoria = tamano;

    memoria->tamanos_particiones = malloc(num_particiones * sizeof(int));
    for (int i = 0; i < num_particiones; i++) {
        memoria->tamanos_particiones[i] = tamanos_particiones[i];
    }
    memoria->num_particiones = num_particiones;

    
    // Inicializar un bloque libre que cubre toda la memoria para particiones dinámicas
    if (esquema == PARTICION_DINAMICA) {
        memoria->tabla_libres.bloques_libres = malloc(sizeof(t_segmento_libre));
        memoria->tabla_libres.bloques_libres[0].base = 0;
        memoria->tabla_libres.bloques_libres[0].tamano = tamano;
        memoria->tabla_libres.num_bloques_libres = 1;
    }

    return memoria;
}

// Liberar la memoria reservada y todas sus estructuras internas
void liberar_memoria(t_memoria* memoria) {
    free(memoria->tabla_segmentos.segmentos);
    free(memoria->tabla_libres.bloques_libres);
    free(memoria->memoria);
    free(memoria);
}

// Función auxiliar para buscar y marcar bloques libres contiguos (solo partición dinámica)
void unir_bloques_libres(t_memoria* memoria) {
    for (int i = 0; i < memoria->tabla_libres.num_bloques_libres - 1; i++) {
        t_segmento_libre* actual = &memoria->tabla_libres.bloques_libres[i];
        t_segmento_libre* siguiente = &memoria->tabla_libres.bloques_libres[i + 1];

        if (actual->base + actual->tamano == siguiente->base) {
            actual->tamano += siguiente->tamano;

            for (int j = i + 1; j < memoria->tabla_libres.num_bloques_libres - 1; j++) {
                memoria->tabla_libres.bloques_libres[j] = memoria->tabla_libres.bloques_libres[j + 1];
            }
            memoria->tabla_libres.num_bloques_libres--;
            memoria->tabla_libres.bloques_libres = realloc(memoria->tabla_libres.bloques_libres,
                                                           memoria->tabla_libres.num_bloques_libres * sizeof(t_segmento_libre));
            i--;
        }
    }
}

// Asignar memoria con partición fija
int asignar_memoria_fija(t_memoria* memoria, int pid) {
    for (int i = 0; i < memoria->num_particiones; i++) {
        int base = i * memoria->tamanos_particiones[i]; // Ajuste de base
        int ocupado = 0;

        for (int j = base; j < base + memoria->tamanos_particiones[i]; j++) {
            if (((char*)memoria->memoria)[j] != 0) {
                ocupado = 1;
                break;
            }
        }

        if (!ocupado) {
            for (int j = base; j < base + memoria->tamanos_particiones[i]; j++) {
                ((char*)memoria->memoria)[j] = pid;
            }

            memoria->tabla_segmentos.segmentos = realloc(memoria->tabla_segmentos.segmentos, 
                                                         (memoria->tabla_segmentos.num_segmentos + 1) * sizeof(t_segmento_usuario));
            memoria->tabla_segmentos.segmentos[memoria->tabla_segmentos.num_segmentos].pid = pid;
            memoria->tabla_segmentos.segmentos[memoria->tabla_segmentos.num_segmentos].base = base;
            memoria->tabla_segmentos.segmentos[memoria->tabla_segmentos.num_segmentos].limite = memoria->tamanos_particiones[i]; // Ajuste de límite
            memoria->tabla_segmentos.num_segmentos++;
            return base;
        }
    }
    return -1; 
}

int asignar_memoria_dinamica(t_memoria* memoria, int id_proceso, int tamano_requerido, t_estrategia_busqueda estrategia) {
    int indice_hueco = -1;
    
    if (estrategia == FIRST_FIT) {
        // Estrategia First Fit: selecciona el primer bloque libre que pueda acomodar el tamaño requerido.
        for (int i = 0; i < memoria->tabla_libres.num_bloques_libres; i++) {
            if (memoria->tabla_libres.bloques_libres[i].tamano >= tamano_requerido) {
                indice_hueco = i;
                break;
            }
        }
    } else if (estrategia == BEST_FIT) {
        // Estrategia Best Fit: selecciona el bloque más pequeño que pueda acomodar el tamaño requerido.
        int tamano_minimo = INT_MAX;
        for (int i = 0; i < memoria->tabla_libres.num_bloques_libres; i++) {
            if (memoria->tabla_libres.bloques_libres[i].tamano >= tamano_requerido &&
                memoria->tabla_libres.bloques_libres[i].tamano < tamano_minimo) {
                tamano_minimo = memoria->tabla_libres.bloques_libres[i].tamano;
                indice_hueco = i;
            }
        }
    } else if (estrategia == WORST_FIT) {
        // Estrategia Worst Fit: selecciona el bloque más grande que pueda acomodar el tamaño requerido.
        int tamano_maximo = -1;
        for (int i = 0; i < memoria->tabla_libres.num_bloques_libres; i++) {
            if (memoria->tabla_libres.bloques_libres[i].tamano >= tamano_requerido &&
                memoria->tabla_libres.bloques_libres[i].tamano > tamano_maximo) {
                tamano_maximo = memoria->tabla_libres.bloques_libres[i].tamano;
                indice_hueco = i;
            }
        }
    }

    if (indice_hueco == -1) {
        return -1;
    }

    // Asignar memoria en el hueco seleccionado
    int base = memoria->tabla_libres.bloques_libres[indice_hueco].base;
    memoria->tabla_libres.bloques_libres[indice_hueco].base += tamano_requerido;
    memoria->tabla_libres.bloques_libres[indice_hueco].tamano -= tamano_requerido;

    // Si el bloque libre se ha reducido a cero, eliminarlo de la tabla de bloques libres
    if (memoria->tabla_libres.bloques_libres[indice_hueco].tamano == 0) {
        for (int j = indice_hueco; j < memoria->tabla_libres.num_bloques_libres - 1; j++) {
            memoria->tabla_libres.bloques_libres[j] = memoria->tabla_libres.bloques_libres[j + 1];
        }
        memoria->tabla_libres.num_bloques_libres--;
        memoria->tabla_libres.bloques_libres = realloc(memoria->tabla_libres.bloques_libres, 
                                                       memoria->tabla_libres.num_bloques_libres * sizeof(t_segmento_libre));
    }

    // Retornar la base del hueco asignado
    return base;
}

//Elige el esquema
int asignar_memoria(t_memoria* memoria, int pid, int tamano) {
    if (memoria->esquema == PARTICION_FIJA) {
        return asignar_memoria_fija(memoria, pid);
    } else if (memoria->esquema == PARTICION_DINAMICA) {
        return asignar_memoria_dinamica(memoria, pid, tamano, estrategia);
    }
    return -1;
}

// Función para liberar la memoria asignada a un proceso específico
void liberar_memoria_proceso(t_memoria* memoria, int pid) {
    for (int i = 0; i < memoria->tabla_segmentos.num_segmentos; i++) {
        if (memoria->tabla_segmentos.segmentos[i].pid == pid) {
            int base = memoria->tabla_segmentos.segmentos[i].base;
            int limite = memoria->tabla_segmentos.segmentos[i].limite;

            for (int j = base; j < base + limite; j++) {
                ((char*)memoria->memoria)[j] = 0;
            }

            memoria->tabla_libres.bloques_libres = realloc(memoria->tabla_libres.bloques_libres, 
                                                           (memoria->tabla_libres.num_bloques_libres + 1) * sizeof(t_segmento_libre));
            memoria->tabla_libres.bloques_libres[memoria->tabla_libres.num_bloques_libres].base = base;
            memoria->tabla_libres.bloques_libres[memoria->tabla_libres.num_bloques_libres].tamano = limite;
            memoria->tabla_libres.num_bloques_libres++;

            for (int j = i; j < memoria->tabla_segmentos.num_segmentos - 1; j++) {
                memoria->tabla_segmentos.segmentos[j] = memoria->tabla_segmentos.segmentos[j + 1];
            }
            memoria->tabla_segmentos.num_segmentos--;
            memoria->tabla_segmentos.segmentos = realloc(memoria->tabla_segmentos.segmentos, 
                                                         memoria->tabla_segmentos.num_segmentos * sizeof(t_segmento_usuario));
            unir_bloques_libres(memoria);
            return;
        }
    }
}

//Muestra esquema, bloque y tabla de segmentos para debbugear
void mostrar_estado_memoria(t_memoria* memoria) {
    printf("=== Estado de la Memoria ===\n");
    printf("Esquema de partición: %s\n", 
           memoria->esquema == PARTICION_FIJA ? "Partición Fija" : "Partición Dinámica");
    
    printf("\n-- Bloques de Memoria --\n");
    for (int i = 0; i < tamanio_memoria; i++) {
        printf("%d ", ((char*)memoria->memoria)[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    printf("\n-- Tabla de Segmentos --\n");
    if (memoria->tabla_segmentos.num_segmentos == 0) {
        printf("No hay segmentos asignados actualmente.\n");
    } else {
        for (int i = 0; i < memoria->tabla_segmentos.num_segmentos; i++) {
            t_segmento_usuario segmento = memoria->tabla_segmentos.segmentos[i];
            printf("PID: %d | Dirección Base: %d | Tamaño: %d bytes\n", 
                   segmento.pid, segmento.base, segmento.limite);
        }
    }
    printf("=============================\n");
}
