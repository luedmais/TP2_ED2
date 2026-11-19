#ifndef INTERCALAF_MAIS_1_H
#define INTERCALAF_MAIS_1_H

#include "heap.h"

#define F_MAIS_1 19      // 19 fitas de entrada + 1 fita de saida = 20 fitas
#define MEM_MAIS_1 19    // Memoria interna maxima de 19 registros

void executarIntercalacaoF1(const char *arqEntrada, const char *arqSaida, TMetricas *m);

#endif