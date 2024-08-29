#ifndef CLIENTE_H
#define CLIENTE_H

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

#include "utils/utils_cliente.h"


t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log* logger, t_paquete * paquete);
void terminar_programa(int,int, t_log*, t_config*);

#endif 