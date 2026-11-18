#include "intercala2f.h"

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

// Fase 1: Gera runs com seleção por substituição (memória = 10) e distribui nas fitas 0..9
static int gerarBlocos2f(const char *arqEntrada, TMetricas *m) {
    FILE *in = fopen(arqEntrada, "rb");
    if (!in) return 0;

    FILE *fitas[F_2F];
    char nomeFita[32];
    for (int i = 0; i < F_2F; i++) {
        sprintf(nomeFita, "fita_%d.bin", i);
        fitas[i] = fopen(nomeFita, "wb");
    }

    FilaPrioridade *heap = criarFila(MEM_2F);
    TRegistro reg;
    TRegistro sentinela = criarSentinela();

    // Carrega os primeiros 10 registros
    for (int i = 0; i < MEM_2F; i++) {
        if (fread(&reg, sizeof(TRegistro), 1, in) == 1) {
            m->leituras++;
            inserirHeap(heap, reg, 1, 0, m);
        } else {
            break;
        }
    }

    int fitaAtual = 0;
    int totalBlocos = 0;
    float ultimoEscrito = -1.0f;

    while (heap->tamanho > 0) {
        if (heap->itens[0].ativo == 0) {
            // Fim do bloco atual: grava sentinela e avança para a próxima fita
            fwrite(&sentinela, sizeof(TRegistro), 1, fitas[fitaAtual]);
            m->escritas++;
            totalBlocos++;

            fitaAtual = (fitaAtual + 1) % F_2F;
            reconstruirHeap(heap, m);
            ultimoEscrito = -1.0f;
            continue;
        }

        HeapItem menor = removerMinimoHeap(heap, m);
        fwrite(&menor.reg, sizeof(TRegistro), 1, fitas[fitaAtual]);
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

    // Fecha o último bloco ativo
    if (totalBlocos > 0 || ultimoEscrito >= 0.0f) {
        fwrite(&sentinela, sizeof(TRegistro), 1, fitas[fitaAtual]);
        m->escritas++;
        totalBlocos++;
    }

    destruirFila(heap);
    fclose(in);
    for (int i = 0; i < F_2F; i++) fclose(fitas[i]);

    return totalBlocos;
}

void executarIntercalacao2f(const char *arqEntrada, const char *arqSaida, TMetricas *m) {
    int numBlocos = gerarBlocos2f(arqEntrada, m);

    int baseEntrada = 0;   // Fitas 0..9
    int baseSaida = F_2F;  // Fitas 10..19
    TRegistro sentinela = criarSentinela();

    // Fase 2: Intercalação balanceada de 10 caminhos até restar 1 único bloco
    while (numBlocos > 1) {
        FILE *fIn[F_2F], *fOut[F_2F];
        char nomeFita[32];

        for (int i = 0; i < F_2F; i++) {
            sprintf(nomeFita, "fita_%d.bin", baseEntrada + i);
            fIn[i] = fopen(nomeFita, "rb");
            sprintf(nomeFita, "fita_%d.bin", baseSaida + i);
            fOut[i] = fopen(nomeFita, "wb");
        }

        FilaPrioridade *heap = criarFila(F_2F);
        int blocosGeradosNestaPassada = 0;
        int fitaDestino = 0;

        while (1) {
            // Carrega 1 registro do próximo bloco disponível de cada fita de entrada
            for (int i = 0; i < F_2F; i++) {
                TRegistro r;
                if (fread(&r, sizeof(TRegistro), 1, fIn[i]) == 1) {
                    m->leituras++;
                    if (!ehSentinela(&r)) {
                        inserirHeap(heap, r, 1, i, m);
                    }
                }
            }

            // Se nenhuma fita tinha mais blocos, a passada terminou
            if (heap->tamanho == 0) break;

            // Intercala os blocos ativos no heap para fOut[fitaDestino]
            while (heap->tamanho > 0) {
                HeapItem menor = removerMinimoHeap(heap, m);
                fwrite(&menor.reg, sizeof(TRegistro), 1, fOut[fitaDestino]);
                m->escritas++;

                TRegistro prox;
                if (fread(&prox, sizeof(TRegistro), 1, fIn[menor.fitaOrigem]) == 1) {
                    m->leituras++;
                    if (!ehSentinela(&prox)) {
                        inserirHeap(heap, prox, 1, menor.fitaOrigem, m);
                    }
                }
            }

            // Encerra o bloco intercalado na fita de saída atual
            fwrite(&sentinela, sizeof(TRegistro), 1, fOut[fitaDestino]);
            m->escritas++;
            blocosGeradosNestaPassada++;
            fitaDestino = (fitaDestino + 1) % F_2F;
        }

        destruirFila(heap);
        for (int i = 0; i < F_2F; i++) {
            fclose(fIn[i]);
            fclose(fOut[i]);
        }

        numBlocos = blocosGeradosNestaPassada;
        // Inverte papéis: fitas de saída tornam-se entrada na próxima passada
        int tmp = baseEntrada;
        baseEntrada = baseSaida;
        baseSaida = tmp;
    }

    // Copia o único bloco final (localizado no início de baseEntrada + 0) para arqSaida
    char nomeFitaFinal[32];
    sprintf(nomeFitaFinal, "fita_%d.bin", baseEntrada);
    FILE *fFinal = fopen(nomeFitaFinal, "rb");
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

    // Limpa fitas temporárias
    for (int i = 0; i < 2 * F_2F; i++) {
        char nomeTmp[32];
        sprintf(nomeTmp, "fita_%d.bin", i);
        remove(nomeTmp);
    }
}