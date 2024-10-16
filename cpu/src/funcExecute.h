#ifndef FUNCEXECUTE_H
#define FUNCEXECUTE_H

#include "server.h"


void funcSET(char* registro, uint32_t valor);
void funcSUM(char* registroOrig, char* registroDest);
void funcSUB(char* registroDest, char* registroOrig);
void funcJNZ(char* registro, uint32_t num_instruccion);
void funcLOG(char* registro);
void funcREAD_MEM(char* registro_datos, char* registro_direccion);
void funcWRITE_MEM(char* registro_direccion, char* registro_datos);
uint32_t obtener_valor_registro(char* registro);
void valor_registro_cpu(char* registro, uint32_t valor);
char* encontrarValorDeRegistro(char* registro);
void escribir_valor_en_memoria(uint32_t direccionFisica, uint32_t valor);

uint32_t tamanio_registro(char *registro);
uint32_t traducir_direccion_logica(uint32_t dirLogica);
uint32_t leer_valor_de_memoria(uint32_t direccionFisica);
void logRegistro(char* registro);

#endif