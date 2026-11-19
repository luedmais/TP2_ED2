#ifndef QUICKSORT_H
#define QUICKSORT_H

#include "box.h"

#define TAM_PIVO 9 // 9 registros no pivo + 1 registro UltLido = 10 registros em memória interna

void executarQuicksortExterno(const char *nomeArquivoBin, int n, TMetricas *metricas);

#endif