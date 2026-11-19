#include "heap.h"

//arquivo dedicado a construcao e manipulacao de heaps usados no metodo de intercalcao 

FilaPrioridade* criarFila(int capacidade) {
    FilaPrioridade *fila = (FilaPrioridade *)malloc(sizeof(FilaPrioridade));
    fila->capacidade = capacidade;
    fila->tamanho = 0;
    fila->itens = (HeapItem *)malloc(capacidade * sizeof(HeapItem));
    return fila;
}

void destruirFila(FilaPrioridade *fila) {
    if (fila) {
        free(fila->itens);
        free(fila);
    }
}

static void trocarHeapItem(HeapItem *a, HeapItem *b) {
    HeapItem temp = *a;
    *a = *b;
    *b = temp;
}

// Retorna < 0 se 'a' tem maior prioridade (deve subir) que 'b'
static int compararHeapItems(HeapItem a, HeapItem b, TMetricas *m) {
    if (a.ativo != b.ativo) {
        return (a.ativo ? -1 : 1);
    }
    m->comparacoes++;
    if (a.reg.nota < b.reg.nota) return -1;
    if (a.reg.nota > b.reg.nota) return 1;
    return 0;
}

static void heapifyUp(FilaPrioridade *fila, int indice, TMetricas *m) {
    while (indice > 0) {
        int pai = (indice - 1) / 2;
        if (compararHeapItems(fila->itens[pai], fila->itens[indice], m) > 0) {
            trocarHeapItem(&fila->itens[pai], &fila->itens[indice]);
            indice = pai;
        } else {
            break;
        }
    }
}

static void heapifyDown(FilaPrioridade *fila, int indice, TMetricas *m) {
    while (1) {
        int esquerda = 2 * indice + 1;
        int direita = 2 * indice + 2;
        int menor = indice;

        if (esquerda < fila->tamanho && compararHeapItems(fila->itens[esquerda], fila->itens[menor], m) < 0)
            menor = esquerda;
        if (direita < fila->tamanho && compararHeapItems(fila->itens[direita], fila->itens[menor], m) < 0)
            menor = direita;

        if (menor != indice) {
            trocarHeapItem(&fila->itens[indice], &fila->itens[menor]);
            indice = menor;
        } else {
            break;
        }
    }
}

void inserirHeap(FilaPrioridade *fila, TRegistro reg, int ativo, int fitaOrigem, TMetricas *m) {
    if (fila->tamanho >= fila->capacidade) return;
    fila->itens[fila->tamanho].reg = reg;
    fila->itens[fila->tamanho].ativo = ativo;
    fila->itens[fila->tamanho].fitaOrigem = fitaOrigem;
    heapifyUp(fila, fila->tamanho, m);
    fila->tamanho++;
}

HeapItem removerMinimoHeap(FilaPrioridade *fila, TMetricas *m) {
    HeapItem minItem = fila->itens[0];
    fila->itens[0] = fila->itens[fila->tamanho - 1];
    fila->tamanho--;
    if (fila->tamanho > 0) {
        heapifyDown(fila, 0, m);
    }
    return minItem;
}

void reconstruirHeap(FilaPrioridade *fila, TMetricas *m) {
    for (int i = 0; i < fila->tamanho; i++) {
        fila->itens[i].ativo = 1;
    }
    for (int i = (fila->tamanho / 2) - 1; i >= 0; i--) {
        heapifyDown(fila, i, m);
    }
}