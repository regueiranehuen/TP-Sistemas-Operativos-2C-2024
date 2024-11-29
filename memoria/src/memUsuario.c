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

    uint32_t *ptr = (uint32_t *)memoria;
    size_t num_elementos = tamanio_memoria / sizeof(uint32_t); // Número de elementos uint32_t

    for (size_t i = 0; i < num_elementos; i++)
    {
        ptr[i] = 0xFFFFFFFF; // Usar 0xFFFFFFFF para mayor claridad
    }

    if (strcmp(esquema, "DINAMICAS") == 0)
    {
        t_particiones *particion_inicial = malloc(sizeof(t_particiones));
        particion_inicial->base = 0;
        particion_inicial->limite = tamanio_memoria;
        particion_inicial->tamanio = tamanio_memoria;
        particion_inicial->ocupada = false;
        list_add(lista_particiones, particion_inicial);
    }
    else if (strcmp(esquema, "FIJAS") == 0)
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

t_particiones *busqueda_fija(int pid, int tamanio_proceso, char *algoritmo_busqueda, int tamanio_lista)
{
    t_particiones *particion = NULL;

    if (strcmp(algoritmo_busqueda, "FIRST") == 0)
    {
        printf("entre a first\n");
        particion = fija_first(pid, tamanio_proceso, tamanio_lista);
    }
    if (strcmp(algoritmo_busqueda, "BEST") == 0)
    {
        particion = fija_best(pid, tamanio_proceso, tamanio_lista);
    }
    if (strcmp(algoritmo_busqueda, "WORST") == 0)
    {
        particion = fija_worst(pid, tamanio_proceso, tamanio_lista);
    }
    return particion;
}
t_particiones *fija_first(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion;
    for (int i = 0; i < tamanio_lista; i++)
    {
        particion = list_get(lista_particiones, i);
        if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
        {
            particion->pid = pid;
            particion->ocupada = true;
            return particion;
        }
    }
    return NULL;
}
t_particiones *fija_best(int pid, int tamanio_proceso, int tamanio_lista)
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
        return NULL;
    }
    particion = list_get(lista_particiones, index);
    particion->pid = pid;
    particion->ocupada = true;
    return particion;
}
t_particiones *fija_worst(int pid, int tamanio_proceso, int tamanio_lista)
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
        return NULL;
    }
    particion = list_get(lista_particiones, index);
    particion->pid = pid;
    particion->ocupada = true;
    return particion;
}

t_particiones *busqueda_dinamica(int pid, int tamanio_proceso, char *algoritmo_busqueda, int tamanio_lista)
{
    t_particiones *particion = NULL;
    if (strcmp(algoritmo_busqueda, "FIRST") == 0)
    {
        particion = dinamica_first(pid, tamanio_proceso, tamanio_lista);
    }

    else if (strcmp(algoritmo_busqueda, "BEST") == 0)
    {
        particion = dinamica_best(pid, tamanio_proceso, tamanio_lista);
    }
    else if (strcmp(algoritmo_busqueda, "WORST") == 0)
    {
        particion = dinamica_worst(pid, tamanio_proceso, tamanio_lista);
    }
    return particion;
}

t_particiones *dinamica_first(int pid, int tamanio_proceso, int tamanio_lista)
{
    t_particiones *particion;

    for (int i = 0; i < tamanio_lista; i++)
    {
        particion = list_get(lista_particiones, i);

        if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
        {
            particion->pid = pid;
            particion->ocupada = true;
            particion->limite = tamanio_proceso + particion->base;
            particion->tamanio = tamanio_proceso;

            acomodar_particion_siguiente(particion,i,tamanio_lista);

            return particion;
        }
    }
    return NULL;
}

t_particiones *dinamica_best(int pid, int tamanio_proceso, int tamanio_lista)
{
    int ubicacion_ideal = -1;
    int index = -1;

    for (int i = 0; i < tamanio_lista; i++)
    {
        t_particiones *particion = list_get(lista_particiones, i);
        if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
        { // caso donde la particion de la lista pueda alojar al proceso
            if (ubicacion_ideal == -1 || ubicacion_ideal > (particion->tamanio - tamanio_proceso))
            { // caso donde todavia no se encontro una ubicacion ideal
                ubicacion_ideal = particion->tamanio - tamanio_proceso;
                index = i;
            }
        }
    }
    if (ubicacion_ideal == -1)
    {
        return NULL;
    }
    else
    {

        t_particiones *particion = list_get(lista_particiones, index);
        particion->pid = pid;
        particion->tamanio = tamanio_proceso;
        particion->ocupada = true;
        particion->limite = particion->base + tamanio_proceso;

        acomodar_particion_siguiente(particion,index,tamanio_lista);

        return particion;
    }
    return NULL;
}
t_particiones *dinamica_worst(int pid, int tamanio_proceso, int tamanio_lista)
{

    int ubicacion_ideal = -1;
    int index = -1;

    for (int i = 0; i < tamanio_lista; i++)
    {
        t_particiones* particion = list_get(lista_particiones, i);
        if (!particion->ocupada && particion->tamanio >= tamanio_proceso)
        { // caso donde la particion de la lista pueda alojar al proceso
            if (ubicacion_ideal == -1)
            { // caso donde todavia no se encontro una ubicacion ideal
                ubicacion_ideal = particion->tamanio - tamanio_proceso;
                index = i;
            }
            else if (ubicacion_ideal < (particion->tamanio - tamanio_proceso))
            { // caso donde el proceso pueda alojarse en una particion
                ubicacion_ideal = particion->tamanio - tamanio_proceso;
                index = i;
            }
        }
    }
    if (ubicacion_ideal == -1)
    {
        return NULL;
    }
    else
    {

        t_particiones* particion = list_get(lista_particiones, index);
        particion->pid = pid;
        particion->tamanio = tamanio_proceso;
        particion->ocupada = true;
        particion->limite = particion->base + tamanio_proceso;

        acomodar_particion_siguiente(particion,index,tamanio_lista);

        return particion;
    }

    return NULL;
}

void acomodar_particion_siguiente(t_particiones *particion, int index, int tamanio_lista)
{
    if (index + 1 == tamanio_lista)
    {

        if (particion->limite != tamanio_memoria)
        {
            t_particiones *particion_restante = malloc(sizeof(t_particiones));
            particion_restante->base = particion->limite;
            particion_restante->limite = tamanio_memoria;
            particion_restante->ocupada = false;
            particion_restante->tamanio = particion_restante->limite - particion_restante->base;
            list_add_in_index(lista_particiones, index + 1, particion_restante);
        }
    }
    else
    {
        t_particiones *particion_aux = list_get(lista_particiones, index + 1);
        if (particion->limite != particion_aux->base)
        {
            list_remove(lista_particiones, index + 1);
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
            }
            particion_restante->tamanio = particion_restante->limite - particion_restante->base;
            list_add_in_index(lista_particiones, index + 1, particion_restante);
            list_add_in_index(lista_particiones, index + 2, particion_aux);
        }
    }
}


t_particiones *inicializar_proceso(int pid, int tamanio_proceso, t_config *config)
{
    int tamanio_lista = list_size(lista_particiones);
    char *esquema = config_get_string_value(config, "ESQUEMA");
    char *algoritmo_busqueda = config_get_string_value(config, "ALGORITMO_BUSQUEDA");
    t_particiones *particion = NULL;
    printf("esquema:%s\n", esquema);
    if (strcmp(esquema, "FIJAS") == 0)
    {
        printf("entre a fijas\n");
        particion = busqueda_fija(pid, tamanio_proceso, algoritmo_busqueda, tamanio_lista);
    }
    else if (strcmp(esquema, "DINAMICAS") == 0)
    {
        particion = busqueda_dinamica(pid, tamanio_proceso, algoritmo_busqueda, tamanio_lista);
    }
    return particion;
}

void liberar_espacio_proceso(int pid)
{
    for (int i = 0; i < list_size(lista_particiones); i++)
    {
        t_particiones *particion = list_get(lista_particiones, i);
        if (particion->ocupada && particion->pid == pid)
        {
            particion->ocupada = false;
            particion->pid = -1;
            char *esquema = config_get_string_value(config, "ESQUEMA");
            if (strcmp(esquema, "DINAMICAS") == 0)
            {
                fusionar_particiones_libres(lista_particiones, particion, i); // Se acomoda la lista si la particion tiene particiones vecinas desocupadas
            }
            return;
        }
    }
}

void fusionar_particiones_libres(t_list *lista_particiones, t_particiones *particion_actual, int indice) {
    printf("Indice: %d\n", indice);
    int tamanio_lista = list_size(lista_particiones);

    // Si la partición actual es la primera
    if (indice == 0) {
        printf("Base partición actual: %d\n", particion_actual->base);
        
        // Verificar si existe una partición siguiente
        if (indice + 1 < tamanio_lista) {
            t_particiones *particion_siguiente = list_get(lista_particiones, indice + 1);
            printf("Base partición siguiente: %d\n", particion_siguiente->base);

            if (!particion_siguiente->ocupada) {
                // Fusionar con la partición siguiente
                particion_actual->limite = particion_siguiente->limite;
                particion_actual->tamanio = particion_actual->limite - particion_actual->base;

                list_remove_and_destroy_element(lista_particiones, indice + 1, free);  // Eliminar partición siguiente
                return;
            }
        }
    } else {
        t_particiones *particion_anterior = list_get(lista_particiones, indice - 1);
        printf("Base partición anterior: %d\n", particion_anterior->base);
        printf("Base partición actual: %d\n", particion_actual->base);

        // Si la partición actual tiene partición anterior y siguiente
        if (indice + 1 < tamanio_lista) {
            t_particiones *particion_siguiente = list_get(lista_particiones, indice + 1);
            printf("Base partición siguiente: %d\n", particion_siguiente->base);

            if (!particion_anterior->ocupada && !particion_siguiente->ocupada) {
                // Fusionar con anterior y siguiente
                particion_anterior->limite = particion_siguiente->limite;
                particion_anterior->tamanio = particion_anterior->limite - particion_anterior->base;

                list_remove_and_destroy_element(lista_particiones, indice, free); // Eliminar partición actual
                list_remove_and_destroy_element(lista_particiones, indice, free); // Eliminar partición siguiente
                return;
            } else if (!particion_anterior->ocupada) {
                // Fusionar con la partición anterior
                particion_anterior->limite = particion_actual->limite;
                particion_anterior->tamanio = particion_anterior->limite - particion_anterior->base;
                list_remove_and_destroy_element(lista_particiones, indice, free);  // Eliminar partición actual
                return;
            } else if (!particion_siguiente->ocupada) {
                // Fusionar con la partición siguiente
                particion_actual->limite = particion_siguiente->limite;
                particion_actual->tamanio = particion_actual->limite - particion_actual->base;
                list_remove_and_destroy_element(lista_particiones, indice + 1, free);  // Eliminar partición siguiente
                return;
            }
        } else {
            // Si no hay partición siguiente
            if (!particion_anterior->ocupada) {
                // Fusionar solo con la partición anterior
                particion_anterior->limite = particion_actual->limite;
                particion_anterior->tamanio = particion_anterior->limite - particion_anterior->base;
                list_remove_and_destroy_element(lista_particiones, indice, free);  // Eliminar partición actual
                return;
            }
        }
    }
}

t_list* lectura_datos_proceso(int pid)
{
    t_particiones *particion = busqueda_particion(pid);
    
    void *puntero = (uint32_t*)memoria + particion->base;

    int tamanio_particion = (particion->tamanio)/4; //se leeran por bloques de 4 bytes

    t_list* lista_datos = list_create();

    for(int i = 0; i<tamanio_particion ; i++){
        uint32_t valor;

        memcpy(&valor, puntero, sizeof(uint32_t));  // Lee 4 bytes en "valor"

        if(valor != 0xFFFFFFFF){//bloque con datos

            uint32_t *dato = malloc(sizeof(uint32_t));
            *dato = valor;
            list_add(lista_datos, dato);
            
        }
        
        puntero += 4;

    }

    if(list_size(lista_datos) == 0 ){//ningun dato escrito
        list_destroy(lista_datos);
        return NULL;
    }

    return lista_datos;
}

t_particiones *busqueda_particion(int pid)
{
    for (int i = 0; i < list_size(lista_particiones); i++)
    {
        t_particiones *particion = list_get(lista_particiones, i);
        
        if (particion == NULL) {
            continue; // Salta al siguiente elemento
        }

        if (particion->pid == pid)
        {
            return particion;
        }
    }
    return NULL;
}


uint32_t leer_Memoria(uint32_t direccionFisica)
{
    uint32_t * valor = malloc(sizeof(uint32_t));
    memcpy(valor,memoria+direccionFisica,sizeof(uint32_t));

    if (*valor == 0xFFFFFFFF)
    {
        return 0xFFFFFFFF;
    }

    return *valor;
}

int escribir_Memoria(t_write_mem *info)
{
    uint32_t * valor = malloc(sizeof(uint32_t));
    *valor = info->valor;
    memcpy(memoria + info->direccionFisica,valor,sizeof(uint32_t));
    free(valor);
    return 0;
}

t_particiones*obtener_particion(int pid){
    for (int i = 0; i< list_size(lista_particiones);i++){
        t_particiones*actual=list_get(lista_particiones,i);

        if (actual->pid == pid){
            return actual;
        }
    }
    return NULL;
}

void liberar_lista_particiones(t_list*lista){
    for (int i = 0; i < list_size(lista); i++){
        t_particiones*actual=list_get(lista,i);
        list_remove(lista,i);
        free(actual);
    }
    free(lista);
}