#ifndef FUNCEXECUTE_H
#define FUNCEXECUTE_H

#include "server.h"
#include "mmu.h"


void funcSET(t_contexto_tid*contexto,char* registro, uint32_t valor);
void funcREAD_MEM(t_contexto_pid_send*contextoPid,t_contexto_tid*contextoTid,char* registro_datos, char* registro_direccion) ;
void funcWRITE_MEM(t_contexto_pid_send*contextoPid,t_contexto_tid*contextoTid,char* registro_direccion, char* registro_datos) ;
void funcSUM(t_contexto_tid*contexto,char* registroOrig, char* registroDest);
void funcSUB(t_contexto_tid*contexto,char* registroDest, char* registroOrig);
void funcJNZ(t_contexto_tid*contexto,char* registro, uint32_t num_instruccion);
void funcLOG(t_contexto_tid*contexto,char* registro);

uint32_t obtener_valor_registro(t_contexto_tid*contexto,char* registro);
void valor_registro_cpu(t_contexto_tid*contexto,char* registro, uint32_t valor);
char* encontrarValorDeRegistro(char* registro);
void logRegistro(t_contexto_tid*contexto,char* registro) ;

uint32_t tamanio_registro(char *registro);
uint32_t leer_valor_de_memoria(uint32_t direccionFisica);
int escribir_valor_en_memoria(uint32_t direccionFisica, uint32_t valor); 


#endif