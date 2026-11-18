#ifndef INTERCALA2F_H
#define INTERCALA2F_H

#include "heap.h"

#define F_2F 10          // 10 fitas de entrada e 10 fitas de saída (2f = 20)
#define MEM_2F 10        // Memória interna máxima de 10 registros

void executarIntercalacao2f(const char *arqEntrada, const char *arqSaida, TMetricas *m);

#endif