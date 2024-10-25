#ifndef SEMAFOROSCOMPARTIDOS_H
#define SEMAFOROSCOMPARTIDOS_H

#include <semaphore.h>
#include <pthread.h>

extern pthread_mutex_t mutex_conexion_kernel_dispatch;
extern pthread_mutex_t mutex_conexion_kernel_interrupt;

#endif