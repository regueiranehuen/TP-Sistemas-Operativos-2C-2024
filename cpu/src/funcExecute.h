#ifndef FUNCEXECUTE_H
#define FUNCEXECUTE_H

#include "server.h"
#include "mmu.h"


void funcSET(char *registro, char* valor);
void funcREAD_MEM(char* registroDatos, char* registroDireccion);
void funcWRITE_MEM(char* registroDireccion, char* registroDatos);
void funcSUM(char* registroOrig, char* registroDest);
void funcSUB(char* registroDest, char* registroOrig);
void funcJNZ(char* registro, char* num_instruccion);
void funcLOG(char* registro);

uint32_t obtener_valor_registro(char* registro);
void valor_registro_cpu(char* registro, char* valor);
char* encontrarValorDeRegistro(char* registro);

uint32_t tamanio_registro(char *registro);
char *leer_valor_de_memoria(uint32_t direccionFisica, uint32_t tamanio);
void escribir_valor_en_memoria(uint32_t direccionFisica, char *valor, uint32_t tamanio);

#endif