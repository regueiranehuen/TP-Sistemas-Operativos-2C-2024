#include "includes/memUsuario.h"

void *memoria;
t_list *lista_particiones;
int tamanio_memoria;

void inicializar_Memoria(t_config *config)
{
    tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");

    memoria = malloc(tamanio_memoria);
    char *esquema = config_get_string_value(config, "ESQUEMA");

    lista_particiones = list_create();

    if (strcmp(esquema, "FIJAS"))
    {
        char **particiones = config_get_array_value(config, "PARTICIONES");
        cargar_particiones_lista(particiones);
    }

    if(strcmp(esquema, "DINAMICAS")){
        t_particiones* particion_inicial = malloc(sizeof(t_particiones));
        particion_inicial->base = 0;
        particion_inicial->limite = tamanio_memoria;
        particion_inicial->tamanio = tamanio_memoria;
        particion_inicial->ocupada = false;
        list_add(lista_particiones,particion_inicial);
    }

}

void cargar_particiones_lista(char **particiones)
{
    int i = 0;
    int base = 0;
    while (particiones[i] != NULL)
    {
        t_particiones *particion = malloc(sizeof(t_particiones));
        int valor = atoi(particiones[i]); // Convertir string a entero
        particion->base = base;
        particion->limite = valor + base;
        particion->tamanio = particion->limite - particion->base;
        particion->ocupada = false;
        base = particion->limite;
        list_add(lista_particiones, particion); // Agregar el puntero del entero a la lista

        i++;
    }
}

int busqueda_fija(int pid, int tamanio_proceso, char *algoritmo_busqueda, int tamanio_lista)
{
    int resultado = -1;
    if (strcmp(algoritmo_busqueda, "FIRST"))
    {
        resultado = fija_first(pid, tamanio_proceso, tamanio_lista);
    }
    if (strcmp(algoritmo_busqueda, "BEST") == 0)
    {
        resultado = fija_best(pid, tamanio_proceso, tamanio_lista);
    }
    if (strcmp(algoritmo_busqueda, "WORST") == 0)
    {
        resultado = fija_worst(pid, tamanio_proceso, tamanio_lista);
    }
    return resultado;
}
int fija_first(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion;
    for (int i = 0; i < tamanio_lista; i++)
    {
        particion = list_get(lista_particiones, i);
        if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
        {
            particion->pid = pid;
            particion->ocupada = true;
            return 0;
        }
    }
    return -1;
}
int fija_best(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion;
    int tamanio_ideal = -1;
    int index = 0;
    for (int i = 0; i < tamanio_lista; i++)
    {
        particion = list_get(lista_particiones, i);
        if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
        {
            if (tamanio_ideal > (particion->tamanio - tamanio_proceso) || tamanio_ideal == -1)
            {
                tamanio_ideal = particion->tamanio - tamanio_proceso;
                index = i;
            }
        }
    }
    if (tamanio_ideal == -1)
    { // no se encontro una particion para el proceso
        return -1;
    }
    particion = list_get(lista_particiones, index);
    particion->pid = pid;
    particion->ocupada = true;
    return 0;
}
int fija_worst(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion;
    int tamanio_ideal = -1;
    int index = -1;
    for (int i = 0; i < tamanio_lista; i++)
    {
        particion = list_get(lista_particiones, i);
        if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
        {
            if (tamanio_ideal < (particion->tamanio - tamanio_proceso) || tamanio_ideal == -1)
            {
                tamanio_ideal = particion->tamanio - tamanio_proceso;
                index = i;
            }
        }
    }
    if (tamanio_ideal == -1)
    { // no se encontro una particion para el proceso
        return -1;
    }
    particion = list_get(lista_particiones, index);
    particion->pid = pid;
    particion->ocupada = true;
    return 0;
}

int busqueda_dinamica(int pid, int tamanio_proceso, char *algoritmo_busqueda, int tamanio_lista)
{
    int resultado = -1;
    if (strcmp(algoritmo_busqueda, "FIRST") == 0)
    {
        resultado = dinamica_first(pid, tamanio_proceso, tamanio_lista);
    }

    else if (strcmp(algoritmo_busqueda, "BEST") == 0)
    {
        resultado = dinamica_best(pid, tamanio_proceso, tamanio_lista);
    }
    else if (strcmp(algoritmo_busqueda, "WORST") == 0)
    {
        resultado = dinamica_worst(pid, tamanio_proceso, tamanio_lista);
    }
    return resultado;
}

int dinamica_first(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion;
    
        for (int i = 1; i < tamanio_lista; i++)
        {
            particion = list_get(lista_particiones, i - 1);

            if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
            {
                particion->pid = pid;
                particion->ocupada = true;
                particion->limite = tamanio_proceso + particion->base;
                t_particiones *particion_aux = list_get(lista_particiones, i);
                if (particion_aux != NULL)
                {
                    if (particion->limite != particion_aux->base)
                    {

                        t_particiones *particion_restante = malloc(sizeof(t_particiones));
                        particion_restante->base = particion->limite;

                        particion_restante->ocupada = false;

                        if (particion_aux->ocupada)
                        {
                            particion_restante->limite = particion_aux->base;
                        }
                        else
                        {
                            particion_restante->limite = particion_aux->limite;
                            list_remove(lista_particiones, i);
                        }
                        particion_restante->tamanio = particion_restante->limite - particion_restante->base;
                        list_add_in_index(lista_particiones,i,particion_restante);
                    }
                }
                else{
                    if(particion->limite != tamanio_memoria){
                        t_particiones *particion_restante = malloc(sizeof(t_particiones));
                        particion_restante->base = particion ->limite;
                        particion_restante->limite = tamanio_memoria;
                        particion_restante->ocupada = false;
                        particion_restante->tamanio = particion_restante->limite - particion_restante->base;
                        list_add_in_index(lista_particiones,i,particion_restante);
                    }
                }
                return 0;
            }
        }
        return -1;
    }


int dinamica_best(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion = malloc(sizeof(t_particiones));
    int ubicacion_ideal = 0;
    int index = -1;
    
        for (int i = 0; i < tamanio_lista; i++)
        {
            particion = list_get(lista_particiones, i);
            if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
            { // caso donde la particion de la lista pueda alojar al proceso
                if (ubicacion_ideal == 0)
                { // caso donde todavia no se encontro una ubicacion ideal
                    ubicacion_ideal = particion->tamanio - tamanio_proceso;
                    index = i;
                }
                else if (ubicacion_ideal > particion->tamanio - tamanio_proceso)
                { // caso donde el proceso pueda alojarse en una particion
                    ubicacion_ideal = particion->tamanio - tamanio_proceso;
                    index = i;
                }
            }
        }
        if (ubicacion_ideal == 0)
        {
            return -1;
        }
        else
        {

            particion = list_get(lista_particiones, index);
            particion->pid = pid;
            particion->tamanio = tamanio_proceso - particion->base;
            particion->ocupada = true;
            particion->limite = particion->base + tamanio_proceso;

            t_particiones *particion_aux = list_get(lista_particiones, index + 1);
            if (particion_aux == NULL)
            {
                if(particion->limite != tamanio_memoria){
                        t_particiones *particion_restante = malloc(sizeof(t_particiones));
                        particion_restante->base = particion ->limite;
                        particion_restante->limite = tamanio_memoria;
                        particion_restante->ocupada = false;
                        particion_restante->tamanio = particion_restante->limite - particion_restante->base;
                        list_add_in_index(lista_particiones,index + 1,particion_restante);
                    }
            }
            else
            {
                if (particion->limite != particion_aux->base)
                    {

                        t_particiones *particion_restante = malloc(sizeof(t_particiones));
                        particion_restante->base = particion->limite;

                        particion_restante->ocupada = false;

                        if (particion_aux->ocupada)
                        {
                            particion_restante->limite = particion_aux->base;
                        }
                        else
                        {
                            particion_restante->limite = particion_aux->limite;
                            list_remove(lista_particiones, index+1);
                        }
                        particion_restante->tamanio = particion_restante->limite - particion_restante->base;
                        list_add_in_index(lista_particiones,index+1,particion_restante);
                    }
            }

            return 0;
        }
        return -1;
    }
int dinamica_worst(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion = malloc(sizeof(t_particiones));
    int ubicacion_ideal = 0;
    int index = -1;

        for (int i = 1; i < tamanio_lista; i++)
        {
            particion = list_get(lista_particiones, i);
            if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
            { // caso donde la particion de la lista pueda alojar al proceso
                if (ubicacion_ideal == 0)
                { // caso donde todavia no se encontro una ubicacion ideal
                    ubicacion_ideal = particion->tamanio - tamanio_proceso;
                    index = i;
                }
                else if (ubicacion_ideal < particion->tamanio - tamanio_proceso)
                { // caso donde el proceso pueda alojarse en una particion
                    ubicacion_ideal = particion->tamanio - tamanio_proceso;
                    index = i;
                }
            }
        }
        if (ubicacion_ideal == 0)
        {
            return -1;
        }
        else
        {

            particion = list_get(lista_particiones, index);
            particion->pid = pid;
            particion->tamanio = tamanio_proceso - particion->base;
            particion->ocupada = true;
            particion->limite = particion->base + tamanio_proceso;

            t_particiones *particion_aux = list_get(lista_particiones, index + 1);
            if (particion_aux == NULL)
            {
                if(particion->limite != tamanio_memoria){
                        t_particiones *particion_restante = malloc(sizeof(t_particiones));
                        particion_restante->base = particion ->limite;
                        particion_restante->limite = tamanio_memoria;
                        particion_restante->ocupada = false;
                        particion_restante->tamanio = particion_restante->limite - particion_restante->base;
                        list_add_in_index(lista_particiones,index + 1,particion_restante);
                    }
            }
            else
            {
                if (particion->limite != particion_aux->base)
                    {

                        t_particiones *particion_restante = malloc(sizeof(t_particiones));
                        particion_restante->base = particion->limite;

                        particion_restante->ocupada = false;

                        if (particion_aux->ocupada)
                        {
                            particion_restante->limite = particion_aux->base;
                        }
                        else
                        {
                            particion_restante->limite = particion_aux->limite;
                            list_remove(lista_particiones, index+1);
                        }
                        particion_restante->tamanio = particion_restante->limite - particion_restante->base;
                        list_add_in_index(lista_particiones,index+1,particion_restante);
                    }
            }

            return 0;
        }
        
        return -1;
    }

int inicializar_proceso(int pid, int tamanio_proceso, t_config *config)
{
    int tamanio_lista = list_size(lista_particiones);
    char *esquema = config_get_string_value(config, "PARTICIONES");
    char *algoritmo_busqueda = config_get_string_value(config, "ALGORITMO_BUSQUEDA");
    int resultado = -1;

    if (strcmp(esquema, "FIJAS") == 0)
    {
        resultado = busqueda_fija(pid, tamanio_proceso, algoritmo_busqueda, tamanio_lista);
    }
    else if (strcmp(esquema, "DINAMICAS") == 0)
    {
        resultado = busqueda_dinamica(pid, tamanio_proceso, algoritmo_busqueda, tamanio_lista);
    }
    return resultado;
}

void liberar_espacio_proceso(int pid)
{
    for (int i = 0; i < list_size(lista_particiones); i++)
    {
        t_particiones *particion = list_get(lista_particiones, i);
        if (particion->ocupada && particion->pid)
        {
            particion->ocupada = false;
            particion->pid = -1;
        }
    }
    fusionar_particiones_libres(lista_particiones); // acomodar la lista
}

void fusionar_particiones_libres(t_list *lista_particiones)
{
    int i = 0;

    while (i < list_size(lista_particiones) - 1)
    {
        t_particiones *particion_actual = list_get(lista_particiones, i);
        t_particiones *particion_siguiente = list_get(lista_particiones, i + 1);

        // Verificar si ambas particiones están libres y son adyacentes
        if (!particion_actual->ocupada && !particion_siguiente->ocupada &&
            particion_actual->limite == particion_siguiente->base)
        {

            // Fusionar las particiones
            particion_actual->limite = particion_siguiente->limite;

            // Eliminar la partición siguiente ya que fue fusionada
            list_remove_and_destroy_element(lista_particiones, i + 1, free);

            // No incrementamos `i` ya que necesitamos verificar la nueva partición fusionada con la siguiente
        }
        else
        {
            // Si no se fusiona, avanzamos al siguiente par de particiones
            i++;
        }
    }
}

void escritura_datos_archivo(int pid, int tid)
{
    t_particiones *particion = busqueda_particion(pid);
    char *path = generar_nombre_archivo(pid, tid);
    char *ruta_absoluta = obtener_ruta_absoluta(path);
    FILE *archivo = fopen(ruta_absoluta, "wb");
    void *puntero = memoria + particion->base;

    size_t tamanio_a_escribir = particion->limite - particion->base;

    size_t bytes_escritos = fwrite(puntero, 1, tamanio_a_escribir, archivo);
    if (bytes_escritos != tamanio_a_escribir)
    {
        perror("Error al escribir en el archivo");
    }

    fclose(archivo);
}

t_particiones *busqueda_particion(int pid)
{
    for (int i = 0; i < list_size(lista_particiones); i++)
    {
        t_particiones *particion = list_get(lista_particiones, i);
        if (particion->pid)
        {
            return particion;
        }
    }
    return NULL;
}

char *generar_nombre_archivo(int pid, int tid)
{
    // Obtener el tiempo actual
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    // Crear un buffer para el timestamp en formato YYYYMMDD-HHMMSS
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", tm_now);

    // Asignar memoria para el nombre del archivo
    char *nombre_archivo = malloc(256); // Asegúrate de que el tamaño sea suficiente
    if (nombre_archivo == NULL)
    {
        perror("Error al asignar memoria");
        return NULL; // Manejar error de memoria
    }

    // Formatear el nombre del archivo
    sprintf(nombre_archivo, "%d-%d-%s.dmp", pid, tid, timestamp);

    return nombre_archivo;
}