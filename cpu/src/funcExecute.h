#ifdef FUNCEXECUTE_H_
#define FUNCEXECUTE_H_

#include "main.h"


void funcSET(char *registro, char* valor);
void funcREAD_MEM(char* registroDatos, char* registroDireccion);
void funcWRITE_MEM(char* registroDireccion, char* registroDatos);
void funcSUM(char* registroOrig, char* registroDest);
void funcSUB(char* registroDest, char* registroOrig);
void funcJNZ(char* registro, char* numInstruccion);
void funcLOG(char* registro);

#endif