#include "box.h"
#include "intercala2f.h"
#include "intercalaf+1.h"
#include "quicksort.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TOTAL_REGISTROS_PROVAO 471705

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Erro: Numero invalido de argumentos.\n");
        exibirUso();
        return EXIT_FAILURE;
    }

    int metodo = atoi(argv[1]);
    int quantidade = atoi(argv[2]);
    int situacao = atoi(argv[3]);
    bool imprimir = false;

    if (metodo < 1 || metodo > 3) {
        fprintf(stderr, "Erro: Metodo invalido (%d). Escolha entre 1, 2 ou 3.\n", metodo);
        exibirUso();
        return EXIT_FAILURE;
    }

    if (quantidade <= 0 || quantidade > TOTAL_REGISTROS_PROVAO) {
        fprintf(stderr, "Erro: Quantidade invalida (%d). Informe um valor entre 1 e %d.\n",
                quantidade, TOTAL_REGISTROS_PROVAO);
        exibirUso();
        return EXIT_FAILURE;
    }

    if (situacao < 1 || situacao > 3) {
        fprintf(stderr, "Erro: Situacao invalida (%d). Escolha entre 1, 2 ou 3.\n", situacao);
        exibirUso();
        return EXIT_FAILURE;
    }

    if (argc == 5) {
        if (strlen(argv[4]) == 2 && argv[4][0] == '-' && toupper((unsigned char)argv[4][1]) == 'P') {
            imprimir = true;
        } else {
            fprintf(stderr, "Erro: Argumento opcional invalido ('%s').\n", argv[4]);
            exibirUso();
            return EXIT_FAILURE;
        }
    }

    const char *arqOriginalTxt = "PROVAO.TXT";
    const char *arqEntradaBin = "entrada.bin";
    const char *arqSaidaBin = "saida.bin";

    // Pré-processamento: extrai os n registros e organiza conforme a situação (1, 2 ou 3)
    if (!prepararArquivoEntrada(arqOriginalTxt, arqEntradaBin, quantidade, situacao)) {
        fprintf(stderr, "Erro: Falha ao preparar o arquivo de entrada a partir de '%s'.\n", arqOriginalTxt);
        return EXIT_FAILURE;
    }

    // Se [-P] foi informado, mostra os registros antes da ordenação (fora do cronômetro)
    if (imprimir) {
        imprimirArquivoBinario(arqEntradaBin, quantidade, "DADOS ANTES DA ORDENACAO");
    }

    TMetricas metricas;
    zeraMetricas(&metricas);

    char nomeMetodo[60];
    const char *arquivoResultado = arqSaidaBin;

    // Início da medição de tempo do método de ordenação externa
    clock_t tempoIni = clock();

    switch (metodo) {
        case 1:
            strcpy(nomeMetodo, "Intercalacao Balanceada de Varios Caminhos (2f)");
            executarIntercalacao2f(arqEntradaBin, arqSaidaBin, &metricas);
            break;

        case 2:
            strcpy(nomeMetodo, "Intercalacao Balanceada de Varios Caminhos (f + 1)");
            executarIntercalacaoF1(arqEntradaBin, arqSaidaBin, &metricas);
            break;

        case 3:
            strcpy(nomeMetodo, "Quicksort Externo");
            // O Quicksort Externo ordena in-place diretamente no arquivo de entrada
            executarQuicksortExterno(arqEntradaBin, quantidade, &metricas);
            arquivoResultado = arqEntradaBin;
            break;
    }

    clock_t tempoFim = clock();
    double tempoDeExecucao = ((double)(tempoFim - tempoIni)) / CLOCKS_PER_SEC;

    // Se [-P] foi informado, mostra os registros após a ordenação (fora do cronômetro)
    if (imprimir) {
        imprimirArquivoBinario(arquivoResultado, quantidade, "DADOS APOS A ORDENACAO");
    }

    // Exibição dos resultados exigidos pelo TP2
    printf("\n========================================================\n");
    printf("Metodo utilizado          : %s\n", nomeMetodo);
    printf("Quantidade de registros   : %d\n", quantidade);
    printf("Situacao inicial          : %d\n", situacao);
    printf("Transferencias (Leitura)  : %lld\n", metricas.leituras);
    printf("Transferencias (Escrita)  : %lld\n", metricas.escritas);
    printf("Comparacoes de chave      : %lld\n", metricas.comparacoes);
    printf("Tempo de execucao         : %.6f segundos\n", tempoDeExecucao);
    printf("========================================================\n");

    // Limpeza dos arquivos binários auxiliares
    remove(arqEntradaBin);
    remove(arqSaidaBin);

    return EXIT_SUCCESS;
}