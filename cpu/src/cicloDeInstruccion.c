#include "includes/cicloDeInstruccion.h"

bool cicloInstrucciones = true;

void ciclo_de_instruccion() {

    cicloInstrucciones=true;

    while(cicloInstrucciones) {

        fetch();
        decode();
        execute();
        //dentro del pcb esta el pc con demas registris
        //pcb->pc++;
        checkInterrupt();
    }
}

void fetch() {

    //Pedir instruccion a Memoria
    //pedir_instruccion_memoria();
    //instruccion = recibir_instruccion(conexion_memoria);
    
}

void decode() {
    if (strcmp(instruccion->parametros1, "SET") == 0) {
        return SET;
    } else if (strcmp(instruccion->parametros1, "READ_MEM") == 0) {
        return READ_MEM;
    } else if (strcmp(instruccion->parametros1, "WRITE_MEM") == 0) {
        return WRITE_MEM;
    } else if (strcmp(instruccion->parametros1, "SUM") == 0) {
        return SUM;
    } else if (strcmp(instruccion->parametros1, "SUB") == 0) {
        return SUB;
    } else if (strcmp(instruccion->parametros1, "JNZ") == 0) {
        return JNZ;
    } else if (strcmp(instruccion->parametros1, "LOG") == 0) {
        return LOG;
    } else if (strcmp(instruccion->parametros1, "DUMP_MEMORY") == 0) {
        return DUMP_MEMORY;
    } else if (strcmp(instruccion->parametros1, "IO") == 0) {
        return IO;
    } else if (strcmp(instruccion->parametros1, "PROCESS_CREATE") == 0) {
        return PROCESS_CREATE ;
    } else if (strcmp(instruccion->parametros1, "THREAD_CREATE") == 0) {
        return THREAD_CREATE ;
    } else if (strcmp(instruccion->parametros1, "THREAD_JOIN") == 0) {
        return THREAD_JOIN ;
    } else if (strcmp(instruccion->parametros1, "THREAD_CANCEL") == 0) {
        return THREAD_CANCEL ;
    } else if (strcmp(instruccion->parametros1, "MUTEX_CREATE") == 0) {
        return MUTEX_CREATE ;
    } else if (strcmp(instruccion->parametros1, "MUTEX_LOCK") == 0) {
        return MUTEX_LOCK ;
    } else if (strcmp(instruccion->parametros1, "MUTEX_UNLOCK") == 0) {
        return MUTEX_UNLOCK ;
    } else if (strcmp(instruccion->parametros1, "THREAD_EXIT") == 0) {
        return THREAD_EXIT;
    } else if (strcmp(instruccion->parametros1, "IO_FS_READ") == 0) {
        return PROCESS_EXIT;
    } 
    
    return -1; // Código de operación no válido

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