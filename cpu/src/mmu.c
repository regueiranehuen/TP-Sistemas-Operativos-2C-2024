#include "mmu.h"

int traducir_direccion_logica(int direccion_logica) {

    //consultar la base de la particion a memoria
    // supongo que le tendremos que pasar el id del proceso actual (PCB)
    int base_particion = obtener_base_particion();

    //consultar el tamanio de la particion para chequear el desplazamiento
    // supongo que le tendremos que pasar el id del proceso actual (PCB)
    int tam_particion = obtener_tamano_particion();

    // Validacion
    if (direccion_logica < 0 || direccion_logica >= tam_particion) {
        //Segmentation Fault
        //act contexto de ejecucion en memoria y enviar TID al kernel con motivo "SF"
        return -1; // O devolver un valor que indique error
    }

    // Calcular la dirección física
    int direccion_fisica = base_particion + direccion_logica;

    // log minimo y obligatorio
    // lectura/escritura memoria

    return direccion_fisica;
}

