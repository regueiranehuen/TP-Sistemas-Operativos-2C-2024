#ifdef EJECUCIONINSTRUCCIONES_H_
#define EJECUCIONINSTRUCCIONES_H_
#include "main.h"


void funSET(char *registro, char* valor);
void funcREAD_MEM(char* registroDatos, char* registroDireccion);
void funWRITE_MEM(char* registroDireccion, char* registroDatos);
void funcSUM(char* registroOrig, char* registroDest);
void funcSUB(char* registroDest, char* registroOrig);
void funcJNZ(char* registro, char* numInstruccion);
void funcLOG(char* registro);

#endif