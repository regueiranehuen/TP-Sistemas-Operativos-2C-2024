#ifndef UTILSSERVER_H
#define UTILSSERVER_H

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>




extern t_log* logger;

void* recibir_buffer(int*, int);

t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

#endif 
