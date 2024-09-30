#include "mmu.h"
#include "server.h"

int traducir_direccion_logica(int direccion_logica) {

    if(direccion_logica <= contexto->registros->Limite) {
        //sf
        enviar_contexto(sockets->socket_cliente, contexto, SEGMENTATION_FAULT);
    }
}

