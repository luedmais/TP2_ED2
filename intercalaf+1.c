#include "intercalaf+1.h"

static TRegistro criarSentinela(void) {
    TRegistro s;
    memset(&s, 0, sizeof(TRegistro));
    s.inscricao = -1;
    s.nota = -1.0f;
    return s;
}

static bool ehSentinela(const TRegistro *r) {
    return (r->inscricao == -1);
}

// Fase 1: Gera runs com seleção por substituição (memória = 19) gravando todas na fita_19.bin
static int gerarBlocosF1(const char *arqEntrada, const char *nomeFitaSaida, TMetricas *m) {
    FILE *in = fopen(arqEntrada, "rb");
    FILE *out = fopen(nomeFitaSaida, "wb");
    if (!in || !out) {
        if (in) fclose(in);
        if (out) fclose(out);
        return 0;
    }

    FilaPrioridade *heap = criarFila(MEM_MAIS_1);
    TRegistro reg;
    TRegistro sentinela = criarSentinela();

    for (int i = 0; i < MEM_MAIS_1; i++) {
        if (fread(&reg, sizeof(TRegistro), 1, in) == 1) {
            m->leituras++;
            inserirHeap(heap, reg, 1, 0, m);
        } else {
            break;
        }
    }

    int totalBlocos = 0;
    float ultimoEscrito = -1.0f;

    while (heap->tamanho > 0) {
        if (heap->itens[0].ativo == 0) {
            fwrite(&sentinela, sizeof(TRegistro), 1, out);
            m->escritas++;
            totalBlocos++;

            reconstruirHeap(heap, m);
            ultimoEscrito = -1.0f;
            continue;
        }

        HeapItem menor = removerMinimoHeap(heap, m);
        fwrite(&menor.reg, sizeof(TRegistro), 1, out);
        m->escritas++;
        ultimoEscrito = menor.reg.nota;

        if (fread(&reg, sizeof(TRegistro), 1, in) == 1) {
            m->leituras++;
            m->comparacoes++;
            if (reg.nota >= ultimoEscrito) {
                inserirHeap(heap, reg, 1, 0, m);
            } else {
                inserirHeap(heap, reg, 0, 0, m);
            }
        }
    }

    if (totalBlocos > 0 || ultimoEscrito >= 0.0f) {
        fwrite(&sentinela, sizeof(TRegistro), 1, out);
        m->escritas++;
        totalBlocos++;
    }

    destruirFila(heap);
    fclose(in);
    fclose(out);
    return totalBlocos;
}

// Distribui os blocos da fita_19.bin nas 19 fitas de entrada (fita_0..fita_18)
static void distribuirBlocosF1(const char *nomeFitaUnica, TMetricas *m) {
    FILE *in = fopen(nomeFitaUnica, "rb");
    FILE *fIn[F_MAIS_1];
    char nomeFita[32];

    for (int i = 0; i < F_MAIS_1; i++) {
        sprintf(nomeFita, "fita_%d.bin", i);
        fIn[i] = fopen(nomeFita, "wb");
    }

    int fitaDestino = 0;
    TRegistro r;
    while (fread(&r, sizeof(TRegistro), 1, in) == 1) {
        m->leituras++;
        fwrite(&r, sizeof(TRegistro), 1, fIn[fitaDestino]);
        m->escritas++;
        if (ehSentinela(&r)) {
            fitaDestino = (fitaDestino + 1) % F_MAIS_1;
        }
    }

    fclose(in);
    for (int i = 0; i < F_MAIS_1; i++) fclose(fIn[i]);
}

void executarIntercalacaoF1(const char *arqEntrada, const char *arqSaida, TMetricas *m) {
    const char *fitaUnica = "fita_19.bin";
    int numBlocos = gerarBlocosF1(arqEntrada, fitaUnica, m);
    TRegistro sentinela = criarSentinela();

    while (numBlocos > 1) {
        // 1. Redistribui os blocos da fita única nas 19 fitas de entrada
        distribuirBlocosF1(fitaUnica, m);

        // 2. Intercala 1 bloco de cada fita de entrada de volta para a fita única (fita_19.bin)
        FILE *fIn[F_MAIS_1];
        char nomeFita[32];
        for (int i = 0; i < F_MAIS_1; i++) {
            sprintf(nomeFita, "fita_%d.bin", i);
            fIn[i] = fopen(nomeFita, "rb");
        }
        FILE *fOut = fopen(fitaUnica, "wb");

        FilaPrioridade *heap = criarFila(F_MAIS_1);
        int blocosGeradosNestaPassada = 0;

        while (1) {
            for (int i = 0; i < F_MAIS_1; i++) {
                TRegistro r;
                if (fread(&r, sizeof(TRegistro), 1, fIn[i]) == 1) {
                    m->leituras++;
                    if (!ehSentinela(&r)) {
                        inserirHeap(heap, r, 1, i, m);
                    }
                }
            }

            if (heap->tamanho == 0) break;

            while (heap->tamanho > 0) {
                HeapItem menor = removerMinimoHeap(heap, m);
                fwrite(&menor.reg, sizeof(TRegistro), 1, fOut);
                m->escritas++;

                TRegistro prox;
                if (fread(&prox, sizeof(TRegistro), 1, fIn[menor.fitaOrigem]) == 1) {
                    m->leituras++;
                    if (!ehSentinela(&prox)) {
                        inserirHeap(heap, prox, 1, menor.fitaOrigem, m);
                    }
                }
            }

            fwrite(&sentinela, sizeof(TRegistro), 1, fOut);
            m->escritas++;
            blocosGeradosNestaPassada++;
        }

        destruirFila(heap);
        for (int i = 0; i < F_MAIS_1; i++) fclose(fIn[i]);
        fclose(fOut);

        numBlocos = blocosGeradosNestaPassada;
    }

    // Copia o único bloco ordenado de fita_19.bin para arqSaida
    FILE *fFinal = fopen(fitaUnica, "rb");
    FILE *out = fopen(arqSaida, "wb");
    if (fFinal && out) {
        TRegistro r;
        while (fread(&r, sizeof(TRegistro), 1, fFinal) == 1) {
            if (ehSentinela(&r)) break;
            fwrite(&r, sizeof(TRegistro), 1, out);
        }
    }
    if (fFinal) fclose(fFinal);
    if (out) fclose(out);

    // Remove as 20 fitas temporárias
    for (int i = 0; i <= F_MAIS_1; i++) {
        char nomeTmp[32];
        sprintf(nomeTmp, "fita_%d.bin", i);
        remove(nomeTmp);
    }
}