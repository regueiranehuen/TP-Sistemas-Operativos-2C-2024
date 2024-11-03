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

int busqueda_dinamica(int pid, int tamanio_proceso, char* algoritmo_busqueda ,int tamanio_lista){
    int resultado = -1;
    if (strcmp(algoritmo_busqueda, "FIRST") == 0)
        {
            resultado=dinamica_first(pid, tamanio_proceso, tamanio_lista);
        }

        else if (strcmp(algoritmo_busqueda, "BEST") == 0)
        {
            resultado=dinamica_best(pid, tamanio_proceso, tamanio_lista);
        }
        else if (strcmp(algoritmo_busqueda, "WORST") == 0)
        {
            resultado=dinamica_worst(pid,tamanio_proceso,tamanio_lista);
        }
        return resultado;
}

int dinamica_first(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion;
    t_particiones *particion_aux;
    if (tamanio_lista == 0 && tamanio_proceso <= tamanio_memoria)
    {
        particion = malloc(sizeof(t_particiones));
        particion->base = 0;
        particion->limite = tamanio_proceso;
        particion->ocupada = true;
        particion->pid = pid;
        particion->tamanio = tamanio_proceso;
        list_add(lista_particiones, particion);
        return 0;
    }
    else
    {
        for (int i = 1; i < tamanio_lista; i++)
        {
            particion = list_get(lista_particiones, i - 1);

            if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
            {
                particion->pid = pid;
                particion->ocupada = true;
                return 0;
            }
            else
            {
                particion_aux = list_get(lista_particiones, i);
                if (particion_aux == NULL)
                {
                    if (tamanio_proceso <= (tamanio_memoria - particion->limite))
                    {
                        t_particiones *particion_nueva = malloc(sizeof(t_particiones));
                        particion_nueva->base = particion->limite;
                        particion_nueva->limite = tamanio_proceso;
                        particion_nueva->tamanio = tamanio_proceso;
                        particion_nueva->pid = pid;
                        particion_nueva->ocupada = true;
                        list_add_in_index(lista_particiones, i, particion_nueva);
                        return 0;
                    }

                    if ((particion_aux->base - particion_aux->limite) >= tamanio_proceso)
                    {
                        t_particiones *particion_nueva = malloc(sizeof(t_particiones));
                        particion_nueva->base = particion->limite;
                        particion_nueva->limite = tamanio_proceso;
                        particion_nueva->tamanio = tamanio_proceso;
                        particion_nueva->pid = pid;
                        particion_nueva->ocupada = true;
                        list_add_in_index(lista_particiones, i, particion_nueva);
                        return 0;
                    }
                }
            }
        }
        return -1;
    }
}
int dinamica_best(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion = malloc(sizeof(t_particiones));
    t_particiones *particion_a_agregar = malloc(sizeof(t_particiones));
    int ubicacion_ideal = 0;
    int index = -1;
    bool entreParticiones = false;
    if (tamanio_lista == 0)
    { // caso donde la lista de particiones este vacia
        if (tamanio_proceso <= tamanio_memoria)
        {
            particion->base = 0;
            particion->limite = tamanio_proceso;
            particion->ocupada = true;
            particion->pid = pid;
            particion->tamanio = tamanio_proceso;
            list_add(lista_particiones, particion);
            return 0;
        }
    }
    else
    {
        for (int i = 1; i < tamanio_lista; i++)
        {
            particion = list_get(lista_particiones, i - 1);
            if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
            { // caso donde la particion de la lista pueda alojar al proceso
                if (ubicacion_ideal == 0)
                { // caso donde todavia no se encontro una ubicacion ideal
                    ubicacion_ideal = particion->tamanio - tamanio_proceso;
                    index = i - 1;
                    entreParticiones = false;
                }
                else if (ubicacion_ideal > particion->tamanio - tamanio_proceso)
                { // caso donde el proceso pueda alojarse en una particion
                    ubicacion_ideal = particion->tamanio - tamanio_proceso;
                    index = i - 1;
                    entreParticiones = false;
                }
            }
            else
            {
                t_particiones *particion_aux = list_get(lista_particiones, i);
                if (i == 2 && tamanio_proceso > (particion_aux->limite))
                { // caso donde todavia no se encontro una ubicacion ideal y el proceso pueda ser alojado entre dos particiones
                    ubicacion_ideal = particion_aux->limite;
                    index = i - 1;
                    entreParticiones = true;
                    particion_a_agregar->base = 0;
                    particion_a_agregar->limite = particion_aux->base;
                    particion_a_agregar->tamanio = particion_aux->base;
                }
                if (ubicacion_ideal > (particion_aux->base - particion->limite))
                { // caso donde el proceso pueda alojarse entre dos particiones y el tamanio de dicha nueva particion es menor que la guardada
                    particion_a_agregar->base = particion->limite;
                    particion_a_agregar->limite = particion_aux->base;
                    particion_a_agregar->tamanio = particion_aux->base - particion->limite;
                    index = 1;
                    entreParticiones = true;
                }
            }
        }
        if (ubicacion_ideal == 0)
        {
            return -1;
        }
        else
        {
            if (entreParticiones)
            {
                list_add_in_index(lista_particiones, index, particion_a_agregar);    
            }
            else
            {
                particion = list_get(lista_particiones, index);
                particion->pid = pid;
                particion->tamanio = tamanio_proceso - particion->base;
                particion->ocupada = true;
            }
            return 0;
        }
        return -1;
    }
    return -1;
}
int dinamica_worst(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion = malloc(sizeof(t_particiones));
    t_particiones *particion_a_agregar = malloc(sizeof(t_particiones));
    int ubicacion_ideal = 0;
    int index = -1;
    bool entreParticiones = false;
    if (tamanio_lista == 0)
    { // caso donde la lista de particiones este vacia
        if (tamanio_proceso <= tamanio_memoria)
        {
            particion->base = 0;
            particion->limite = tamanio_proceso;
            particion->ocupada = true;
            particion->pid = pid;
            particion->tamanio = tamanio_proceso;
            list_add(lista_particiones, particion);
            return 0;
        }
    }
    else
    {
        for (int i = 1; i < tamanio_lista; i++)
        {
            particion = list_get(lista_particiones, i - 1);
            if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
            { // caso donde la particion de la lista pueda alojar al proceso
                if (ubicacion_ideal == 0)
                { // caso donde todavia no se encontro una ubicacion ideal
                    ubicacion_ideal = particion->tamanio - tamanio_proceso;
                    index = i - 1;
                    entreParticiones = false;
                }
                else if (ubicacion_ideal < particion->tamanio - tamanio_proceso)
                { // caso donde el proceso pueda alojarse en una particion
                    ubicacion_ideal = particion->tamanio - tamanio_proceso;
                    index = i - 1;
                    entreParticiones = false;
                }
            }
            else
            {
                t_particiones *particion_aux = list_get(lista_particiones, i);
                if (i == 2 && tamanio_proceso > (particion_aux->limite))
                { // caso donde todavia no se encontro una ubicacion ideal y el proceso pueda ser alojado entre dos particiones
                    if (ubicacion_ideal == 0)
                    {
                        ubicacion_ideal = particion_aux->limite;
                        index = i - 1;
                        entreParticiones = true;
                        particion_a_agregar->base = 0;
                        particion_a_agregar->limite = particion_aux->base;
                        particion_a_agregar->tamanio = particion_aux->base;
                    }
                }
                if (ubicacion_ideal < (particion_aux->base - particion->limite))
                { // caso donde el proceso pueda alojarse entre dos particiones y el tamanio de dicha nueva particion es menor que la guardada
                    particion_a_agregar->base = particion->limite;
                    particion_a_agregar->limite = particion_aux->base;
                    particion_a_agregar->tamanio = particion_aux->base - particion->limite;
                    index = 1;
                    entreParticiones = true;
                }
            }
        }
        if (ubicacion_ideal == 0)
        {
            return -1;
        }
        else
        {
            if (entreParticiones)
            {
                list_add_in_index(lista_particiones, index, particion_a_agregar);
            }
            else
            {
                particion = list_get(lista_particiones, index);
                particion->pid = pid;
                particion->tamanio = tamanio_proceso - particion->base;
                particion->ocupada = true;
            }
            return 0;
        }
        return -1;
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
        resultado = busqueda_dinamica(pid,tamanio_proceso,algoritmo_busqueda,tamanio_lista);
    }
    return resultado;
}
