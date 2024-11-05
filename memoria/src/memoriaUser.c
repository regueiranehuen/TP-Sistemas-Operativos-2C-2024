#include "includes/memoriaUser.h"

//Falta la parte de la creacion y eliminacion de hilos


t_memoria* inicializar_memoria(t_esquema_particion esquema, int tamano, int* lista_particiones) {
    
    memoria->memoria = malloc(tamano);
    memset(memoria->memoria, 0, tamano);

    int num_particiones = tamano / lista_particiones[0];
    memoria->lista_particiones = malloc(num_particiones * sizeof(int));
    for (int i = 0; i < num_particiones; i++) {
        memoria->lista_particiones[i] = lista_particiones[i];
    }
    memoria->num_particiones = num_particiones;

    // Inicializar un bloque libre que cubre toda la memoria para particiones dinámicas
    if (esquema == PARTICION_DINAMICA) {
        memoria->tabla_libres.bloques_libres = malloc(sizeof(t_particion_libre));
        memoria->tabla_libres.bloques_libres[0].base = 0;
        memoria->tabla_libres.bloques_libres[0].tamano = tamano;
        memoria->tabla_libres.num_bloques_libres = 1;
    }

    return memoria;
}

void liberar_memoria(t_memoria* memoria) {
    free(memoria->tabla_particiones.particiones);
    free(memoria->tabla_libres.bloques_libres);
    free(memoria->memoria);
    free(memoria->lista_particiones);
    free(memoria);
}

void unir_bloques_libres(t_memoria* memoria) {
    for (int i = 0; i < memoria->tabla_libres.num_bloques_libres - 1; i++) {
        t_particion_libre* actual = &memoria->tabla_libres.bloques_libres[i];
        t_particion_libre* siguiente = &memoria->tabla_libres.bloques_libres[i + 1];

        if (actual->base + actual->tamano == siguiente->base) {
            actual->tamano += siguiente->tamano;

            for (int j = i + 1; j < memoria->tabla_libres.num_bloques_libres - 1; j++) {
                memoria->tabla_libres.bloques_libres[j] = memoria->tabla_libres.bloques_libres[j + 1];
            }
            memoria->tabla_libres.num_bloques_libres--;
            memoria->tabla_libres.bloques_libres = realloc(memoria->tabla_libres.bloques_libres,
                                                           memoria->tabla_libres.num_bloques_libres * sizeof(t_particion_libre));
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

            memoria->tabla_particiones.particiones = realloc(memoria->tabla_particiones.particiones, 
                                                         (memoria->tabla_particiones.num_particiones + 1) * sizeof(t_contexto_pid_send));
            memoria->tabla_particiones.particiones[memoria->tabla_particiones.num_particiones].pid = pid;
            memoria->tabla_particiones.particiones[memoria->tabla_particiones.num_particiones].base = base;
            memoria->tabla_particiones.particiones[memoria->tabla_particiones.num_particiones].limite = memoria->lista_particiones[i];
            memoria->tabla_particiones.num_particiones++;
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
                                                       memoria->tabla_libres.num_bloques_libres * sizeof(t_particion_libre));
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

//solo para dinamica
void liberar_memoria_proceso(t_memoria* memoria, int pid) {
    bool encontrado = false; // Bandera para indicar si se encontró el proceso
    for (int i = 0; i < memoria->tabla_particiones.num_particiones; i++) {
        if (memoria->tabla_particiones.particiones[i].pid == pid) {
            encontrado = true; // Se encontró el proceso
            int base = memoria->tabla_particiones.particiones[i].base;
            int limite = memoria->tabla_particiones.particiones[i].limite;

            // Limpiar la memoria del proceso
            for (int j = base; j < base + limite; j++) {
                ((char*)memoria->memoria)[j] = 0;
            }

            // Añadir el bloque liberado a la tabla de bloques libres
            memoria->tabla_libres.bloques_libres = realloc(memoria->tabla_libres.bloques_libres, 
                                                           (memoria->tabla_libres.num_bloques_libres + 1) * sizeof(t_particion_libre));
            memoria->tabla_libres.bloques_libres[memoria->tabla_libres.num_bloques_libres].base = base;
            memoria->tabla_libres.bloques_libres[memoria->tabla_libres.num_bloques_libres].tamano = limite;
            memoria->tabla_libres.num_bloques_libres++;

            // Unir bloques libres para compactar
            unir_bloques_libres(memoria);

            // Remover particion de la tabla de particions
            for (int j = i; j < memoria->tabla_particiones.num_particiones - 1; j++) {
                memoria->tabla_particiones.particiones[j] = memoria->tabla_particiones.particiones[j + 1];
            }
            memoria->tabla_particiones.num_particiones--;
            memoria->tabla_particiones.particiones = realloc(memoria->tabla_particiones.particiones, 
                                                         memoria->tabla_particiones.num_particiones * sizeof(t_contexto_pid_send));
            break; // Salir del bucle después de liberar el particion
        }
    }
    if (!encontrado) {
        printf("No se encontró el proceso con PID: %d", pid);
    }
}

void mostrar_memoria(t_memoria* memoria) {
    printf("Estado de la memoria (%d bytes):\n", memoria->tamano_memoria);
    for (int i = 0; i < memoria->tabla_particiones.num_particiones; i++) {
        printf("Proceso PID: %d, Base: %d, Limite: %d\n", memoria->tabla_particiones.particiones[i].pid, 
               memoria->tabla_particiones.particiones[i].base, memoria->tabla_particiones.particiones[i].limite);
    }

    printf("Bloques libres:\n");
    for (int i = 0; i < memoria->tabla_libres.num_bloques_libres; i++) {
        printf("Base: %d, Tamano: %d\n", memoria->tabla_libres.bloques_libres[i].base, 
               memoria->tabla_libres.bloques_libres[i].tamano);
    }
}