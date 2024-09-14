#include "funcExecute.h"

bool cicloInstrucciones = true;

void ciclo_de_instruccion() {

    cicloInstrucciones=true;

    while(cicloInstrucciones) {

        fetch();
        decode();
        execute();
        checkInterrupt();
    }
}

void fetch() {


}

void decode() {


}

void execute() {


}

void checkInterrupt() {
 // pseudo
 //if(hay_interrupciones)
 //
 //if(contexto->tid == tid_interrupt)
 //actualizar contexto_ejecucion en Memoria
 //y devolver el TID al kernel con motivo de interrupcion
 //t_paquete* paquete = crear_paquete(INTERRUPCION);
 //agregar_contextoEjec_a_paquete(paquete, algo);
 //enviar_paquete(paquete)
 //eliminar_paquete(paquete);
 //el else no lo pondria porque de todas maneras se lo descarta igual
}