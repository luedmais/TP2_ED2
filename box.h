#ifndef BOX_H
#define BOX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

// Estrutura do registro conforme colunas do PROVAO.TXT
typedef struct {
    long inscricao;     // Colunas 1 a 8
    float nota;         // Colunas 10 a 14
    char estado[3];     // Colunas 16 a 17 (+ '\0')
    char cidade[51];    // Colunas 19 a 68 (+ '\0')
    char curso[31];     // Colunas 70 a 99 (+ '\0')
} TRegistro;

// Estrutura para contabilização das métricas exigidas no TP2
typedef struct {
    long long leituras;     // Transferências: memória externa -> interna
    long long escritas;     // Transferências: memória interna -> externa
    long long comparacoes;  // Comparações entre valores do campo 'nota'
} TMetricas;

// Funções utilitárias de leitura e preparação dos dados
void zeraMetricas(TMetricas *m);
void trocaRegistros(TRegistro *a, TRegistro *b);
bool prepararArquivoEntrada(const char *nomeTxt, const char *nomeBinSaida, int n, int situacao);
void imprimirArquivoBinario(const char *nomeBin, int n, const char *titulo);

#endif