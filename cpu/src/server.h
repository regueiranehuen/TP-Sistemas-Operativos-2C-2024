#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "utils/utils_server.h"

void modo_server(t_log * logger);
void procesar_operaciones(int nuevo_socket);
void acabar_log(t_log*logger);
void iterator(char* value);

#endif /* SERVER_H_ */