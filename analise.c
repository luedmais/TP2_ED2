#include "box.h"
#include "heap.h"
#include "intercala2f.h"
#include "intercalaf+1.h"
#include "quicksort.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TOTAL_REGISTROS_PROVAO 471705


int main(int argc, char *argv[]) {
    //tratamento de erros
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Erro: Numero invalido de argumentos.\n");
        return EXIT_FAILURE;
    }
   // converso dos argumentos para inteiros
    int metodo = atoi(argv[1]);
    int quantidade = atoi(argv[2]);
    int situacao = atoi(argv[3]);
    bool imprimir = false;

    if (metodo < 1 || metodo > 3) {
        fprintf(stderr, "Erro: Metodo invalido (%d). Escolha entre 1, 2 ou 3.\n", metodo);
        return EXIT_FAILURE;
    }

    if (quantidade <= 0 || quantidade > TOTAL_REGISTROS_PROVAO) {
        fprintf(stderr, "Erro: Quantidade invalida (%d). Informe um valor entre 1 e %d.\n",
                quantidade, TOTAL_REGISTROS_PROVAO);
        return EXIT_FAILURE;
    }

    if (situacao < 1 || situacao > 3) {
        fprintf(stderr, "Erro: Situacao invalida (%d). Escolha entre 1, 2 ou 3.\n", situacao);
        return EXIT_FAILURE;
    }

    if (argc == 5) {
        if (strlen(argv[4]) == 2 && argv[4][0] == '-' && toupper((unsigned char)argv[4][1]) == 'P') {
            imprimir = true;
        } else {
            fprintf(stderr, "Erro: Argumento opcional invalido ('%s').\n", argv[4]);
            return EXIT_FAILURE;
        }
    }

    //definicao dos nomes dos arquivos
    const char *arqOriginalTxt = "PROVAO.TXT";
    const char *arqEntradaBin = "entrada.bin";
    const char *arqSaidaBin = "saida.bin";

    //Pre-processamento: extrai os n registros e organiza conforme a situação (1, 2 ou 3)
    if (!prepararArquivoEntrada(arqOriginalTxt, arqEntradaBin, quantidade, situacao)) {
        fprintf(stderr, "Erro: Falha ao preparar o arquivo de entrada a partir de '%s'.\n", arqOriginalTxt);
        return EXIT_FAILURE;
    }

    // Se [-P] foi informado, mostra os registros antes da ordenacao
    if (imprimir) {
        imprimirArquivoBinario(arqEntradaBin, quantidade, "DADOS ANTES DA ORDENACAO");
    }

    TMetricas metricas;
    zeraMetricas(&metricas);

    char nomeMetodo[60];
    const char *arquivoResultado = arqSaidaBin;

    //medicao do tempo de execucao dos metodos
    clock_t tempoIni = clock();

    //chamada dos metodos de acordo com a escolha do usuario
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

    // Se [-P] foi informado, mostra os registros
    if (imprimir) {
        imprimirArquivoBinario(arquivoResultado, quantidade, "DADOS APOS A ORDENACAO");
    }

    // Exibicao dos resultados
    printf("\n========================================================\n");
    printf("Metodo utilizado          : %s\n", nomeMetodo);
    printf("Quantidade de registros   : %d\n", quantidade);
    printf("Situacao inicial          : %d\n", situacao);
    printf("Transferencias (Leitura)  : %lld\n", metricas.leituras);
    printf("Transferencias (Escrita)  : %lld\n", metricas.escritas);
    printf("Comparacoes de chave      : %lld\n", metricas.comparacoes);
    printf("Tempo de execucao         : %.6f segundos\n", tempoDeExecucao);
    printf("========================================================\n");

    // Limpeza dos arquivos binarios auxiliares
    remove(arqEntradaBin);
    remove(arqSaidaBin);

    return EXIT_SUCCESS;
}