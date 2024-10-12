#ifndef FUNCEXECUTE_H
#define FUNCEXECUTE_H

#include "server.h"


void funcSET(char *registro, char* valor);
void funcREAD_MEM(char* registroDatos, char* registroDireccion);
void funcWRITE_MEM(char* registroDireccion, char* registroDatos);
void funcSUM(char* registroOrig, char* registroDest);
void funcSUB(char* registroDest, char* registroOrig);
void funcJNZ(char* registro, char* num_instruccion);
void funcLOG(char* registro);

uint32_t obtener_valor_registro(char* parametro);
void valor_registro_cpu(char* registro, char* valor);
char* encontrarValorDeRegistro(char* register_to_find_value);

#endif