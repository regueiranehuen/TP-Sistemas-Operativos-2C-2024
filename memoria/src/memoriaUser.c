#include "includes/memoriaUser.h"


t_memoria* inicializar_memoria(t_esquema_particion esquema, int tamano, int* lista_particiones, int num_particiones) {
    t_memoria* memoria = malloc(sizeof(t_memoria));
    memoria->memoria = malloc(tamano);
    memset(memoria->memoria, 0, tamano);
    memoria->tabla_segmentos.segmentos = NULL;
    memoria->tabla_segmentos.num_segmentos = 0;
    memoria->tabla_libres.bloques_libres = NULL;
    memoria->tabla_libres.num_bloques_libres = 0;
    memoria->esquema = esquema;
    memoria->tamano_memoria = tamano;

    memoria->lista_particiones = malloc(num_particiones * sizeof(int));
    for (int i = 0; i < num_particiones; i++) {
        memoria->lista_particiones[i] = lista_particiones[i];
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

void liberar_memoria(t_memoria* memoria) {
    free(memoria->tabla_segmentos.segmentos);
    free(memoria->tabla_libres.bloques_libres);
    free(memoria->memoria);
    free(memoria->lista_particiones);
    free(memoria);
}

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

int asignar_memoria_fija(t_memoria* memoria, int pid) {
    for (int i = 0; i < memoria->num_particiones; i++) {
        int base = i * memoria->lista_particiones[i];
        int ocupado = 0;

        for (int j = base; j < base + memoria->lista_particiones[i]; j++) {
            if (((char*)memoria->memoria)[j] != 0) {
                ocupado = 1;
                break;
            }
        }

        if (!ocupado) {
            for (int j = base; j < base + memoria->lista_particiones[i]; j++) {
                ((char*)memoria->memoria)[j] = pid;
            }

            memoria->tabla_segmentos.segmentos = realloc(memoria->tabla_segmentos.segmentos, 
                                                         (memoria->tabla_segmentos.num_segmentos + 1) * sizeof(t_segmento_usuario));
            memoria->tabla_segmentos.segmentos[memoria->tabla_segmentos.num_segmentos].pid = pid;
            memoria->tabla_segmentos.segmentos[memoria->tabla_segmentos.num_segmentos].base = base;
            memoria->tabla_segmentos.segmentos[memoria->tabla_segmentos.num_segmentos].limite = memoria->lista_particiones[i];
            memoria->tabla_segmentos.num_segmentos++;
            return base;
        }
    }
    return -1; 
}

int asignar_memoria_dinamica(t_memoria* memoria, int id_proceso, int tamano_requerido, t_estrategia_busqueda estrategia) {
    int indice_hueco = -1;
    
    if (estrategia == FIRST_FIT) {
        for (int i = 0; i < memoria->tabla_libres.num_bloques_libres; i++) {
            if (memoria->tabla_libres.bloques_libres[i].tamano >= tamano_requerido) {
                indice_hueco = i;
                break;
            }
        }
    } else if (estrategia == BEST_FIT) {
        int tamano_minimo = INT_MAX;
        for (int i = 0; i < memoria->tabla_libres.num_bloques_libres; i++) {
            if (memoria->tabla_libres.bloques_libres[i].tamano >= tamano_requerido &&
                memoria->tabla_libres.bloques_libres[i].tamano < tamano_minimo) {
                tamano_minimo = memoria->tabla_libres.bloques_libres[i].tamano;
                indice_hueco = i;
            }
        }
    } else if (estrategia == WORST_FIT) {
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

    int base = memoria->tabla_libres.bloques_libres[indice_hueco].base;
    memoria->tabla_libres.bloques_libres[indice_hueco].base += tamano_requerido;
    memoria->tabla_libres.bloques_libres[indice_hueco].tamano -= tamano_requerido;

    if (memoria->tabla_libres.bloques_libres[indice_hueco].tamano == 0) {
        for (int j = indice_hueco; j < memoria->tabla_libres.num_bloques_libres - 1; j++) {
            memoria->tabla_libres.bloques_libres[j] = memoria->tabla_libres.bloques_libres[j + 1];
        }
        memoria->tabla_libres.num_bloques_libres--;
        memoria->tabla_libres.bloques_libres = realloc(memoria->tabla_libres.bloques_libres, 
                                                       memoria->tabla_libres.num_bloques_libres * sizeof(t_segmento_libre));
    }

    return base;
}

int asignar_memoria(t_memoria* memoria, int pid, int tamano) {
    if (memoria->esquema == PARTICION_FIJA) {
        return asignar_memoria_fija(memoria, pid);
    } else if (memoria->esquema == PARTICION_DINAMICA) {
        return asignar_memoria_dinamica(memoria, pid, tamano, memoria->estrategia); // Pass the search strategy from memoria
    }
    return -1;
}

void liberar_memoria_proceso(t_memoria* memoria, int pid) {
    bool encontrado = false; // Bandera para indicar si se encontró el proceso
    for (int i = 0; i < memoria->tabla_segmentos.num_segmentos; i++) {
        if (memoria->tabla_segmentos.segmentos[i].pid == pid) {
            encontrado = true; // Se encontró el proceso
            int base = memoria->tabla_segmentos.segmentos[i].base;
            int limite = memoria->tabla_segmentos.segmentos[i].limite;

            // Limpiar la memoria del proceso
            for (int j = base; j < base + limite; j++) {
                ((char*)memoria->memoria)[j] = 0;
            }

            // Añadir el bloque liberado a la tabla de bloques libres
            memoria->tabla_libres.bloques_libres = realloc(memoria->tabla_libres.bloques_libres, 
                                                           (memoria->tabla_libres.num_bloques_libres + 1) * sizeof(t_segmento_libre));
            memoria->tabla_libres.bloques_libres[memoria->tabla_libres.num_bloques_libres].base = base;
            memoria->tabla_libres.bloques_libres[memoria->tabla_libres.num_bloques_libres].tamano = limite;
            memoria->tabla_libres.num_bloques_libres++;

            // Unir bloques libres para compactar
            unir_bloques_libres(memoria);

            // Remover segmento de la tabla de segmentos
            for (int j = i; j < memoria->tabla_segmentos.num_segmentos - 1; j++) {
                memoria->tabla_segmentos.segmentos[j] = memoria->tabla_segmentos.segmentos[j + 1];
            }
            memoria->tabla_segmentos.num_segmentos--;
            memoria->tabla_segmentos.segmentos = realloc(memoria->tabla_segmentos.segmentos, 
                                                         memoria->tabla_segmentos.num_segmentos * sizeof(t_segmento_usuario));
            break; // Salir del bucle después de liberar el segmento
        }
    }
    if (!encontrado) {
        printf("No se encontró el proceso con PID: %d", pid);
    }
}

void mostrar_memoria(t_memoria* memoria) {
    printf("Estado de la memoria (%d bytes):\n", memoria->tamano_memoria);
    for (int i = 0; i < memoria->tabla_segmentos.num_segmentos; i++) {
        printf("Proceso PID: %d, Base: %d, Limite: %d\n", memoria->tabla_segmentos.segmentos[i].pid, 
               memoria->tabla_segmentos.segmentos[i].base, memoria->tabla_segmentos.segmentos[i].limite);
    }

    printf("Bloques libres:\n");
    for (int i = 0; i < memoria->tabla_libres.num_bloques_libres; i++) {
        printf("Base: %d, Tamano: %d\n", memoria->tabla_libres.bloques_libres[i].base, 
               memoria->tabla_libres.bloques_libres[i].tamano);
    }
}