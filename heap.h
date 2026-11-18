#ifndef HEAP_H
#define HEAP_H

#include "box.h"

typedef struct {
    TRegistro reg;
    int ativo;       // 1 = ativo na run atual, 0 = congelado (seleção por substituição)
    int fitaOrigem;  // Índice da fita de onde o registro foi lido (usado na intercalação)
} HeapItem;

typedef struct {
    HeapItem *itens;
    int tamanho;
    int capacidade;
} FilaPrioridade;

FilaPrioridade* criarFila(int capacidade);
void destruirFila(FilaPrioridade *fila);
void inserirHeap(FilaPrioridade *fila, TRegistro reg, int ativo, int fitaOrigem, TMetricas *m);
HeapItem removerMinimoHeap(FilaPrioridade *fila, TMetricas *m);
void reconstruirHeap(FilaPrioridade *fila, TMetricas *m);

#endif