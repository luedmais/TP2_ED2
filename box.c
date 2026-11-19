#include "box.h"

//arquivo usado para funcoes auxiliares dentro do pre processamento e da funcao main

void zeraMetricas(TMetricas *m) {
    m->leituras = 0;
    m->escritas = 0;
    m->comparacoes = 0;
}

void trocaRegistros(TRegistro *a, TRegistro *b) {
    TRegistro temp = *a;
    *a = *b;
    *b = temp;
}

// Extrai uma linha de tamanho fixo do PROVAO.TXT para um TRegistro
static bool parseLinhaProvao(const char *linha, TRegistro *reg) {
    if (strlen(linha) < 70) return false;

    char bufInsc[9];
    char bufNota[6];

    // Colunas 1 a 8 (indices 0..7)
    strncpy(bufInsc, linha, 8);
    bufInsc[8] = '\0';
    reg->inscricao = strtol(bufInsc, NULL, 10);

    // Colunas 10 a 14 (indices 9..13)
    strncpy(bufNota, linha + 9, 5);
    bufNota[5] = '\0';
    reg->nota = strtof(bufNota, NULL);

    // Colunas 16 e 17 (indices 15..16)
    strncpy(reg->estado, linha + 15, 2);
    reg->estado[2] = '\0';

    // Colunas 19 a 68 (indices 18..67)
    strncpy(reg->cidade, linha + 18, 50);
    reg->cidade[50] = '\0';

    // Colunas 70 a 99 (indices 69..98) - trata quebra de linha se a linha for mais curta
    size_t len = strlen(linha);
    size_t maxCurso = (len > 69) ? (len - 69) : 0;
    if (maxCurso > 30) maxCurso = 30;
    strncpy(reg->curso, linha + 69, maxCurso);
    reg->curso[maxCurso] = '\0';
    // Remove '\r' ou '\n' residuais
    for (int i = 0; reg->curso[i] != '\0'; i++) {
        if (reg->curso[i] == '\n' || reg->curso[i] == '\r') {
            reg->curso[i] = '\0';
            break;
        }
    }

    return true;
}

// Comparadores auxiliares para preparar as situaces 1 e 2 no pre-processamento
static int cmpAsc(const void *a, const void *b) {
    float na = ((const TRegistro *)a)->nota;
    float nb = ((const TRegistro *)b)->nota;
    if (na < nb) return -1;
    if (na > nb) return 1;
    return 0;
}

static int cmpDesc(const void *a, const void *b) {
    float na = ((const TRegistro *)a)->nota;
    float nb = ((const TRegistro *)b)->nota;
    if (na > nb) return -1;
    if (na < nb) return 1;
    return 0;
}

// Le os n primeiros registros de PROVAO.TXT, aplica a situacao (1, 2 ou 3) e grava em nomeBinSaida
bool prepararArquivoEntrada(const char *nomeTxt, const char *nomeBinSaida, int n, int situacao) {
    FILE *arqTxt = fopen(nomeTxt, "r");
    if (!arqTxt) {
        perror("Erro ao abrir o arquivo PROVAO.TXT");
        return false;
    }

    TRegistro *buffer = (TRegistro *)malloc(n * sizeof(TRegistro));
    if (!buffer) {
        fprintf(stderr, "Erro de alocacao de memoria no pre-processamento.\n");
        fclose(arqTxt);
        return false;
    }

    char linha[256];
    int lidos = 0;
    while (lidos < n && fgets(linha, sizeof(linha), arqTxt) != NULL) {
        if (parseLinhaProvao(linha, &buffer[lidos])) {
            lidos++;
        }
    }
    fclose(arqTxt);

    if (lidos < n) {
        fprintf(stderr, "Aviso: O arquivo possui apenas %d registros validos (solicitado: %d).\n", lidos, n);
        free(buffer);
        return false;
    }

    // Ordena previamente conforme a situacao escolhida
    if (situacao == 1) {
        qsort(buffer, n, sizeof(TRegistro), cmpAsc);
    } else if (situacao == 2) {
        qsort(buffer, n, sizeof(TRegistro), cmpDesc);
    }
    // situacao == 3 mantem a ordem original aleatoria

    FILE *arqBin = fopen(nomeBinSaida, "wb");
    if (!arqBin) {
        perror("Erro ao criar arquivo binario de entrada");
        free(buffer);
        return false;
    }

    fwrite(buffer, sizeof(TRegistro), n, arqBin);
    fclose(arqBin);
    free(buffer);
    return true;
}

void imprimirArquivoBinario(const char *nomeBin, int n, const char *titulo) {
    FILE *arq = fopen(nomeBin, "rb");
    if (!arq) {
        perror("Erro ao abrir arquivo binario para impressao");
        return;
    }

    printf("\n=== %s ===\n", titulo);
    TRegistro item;
    for (int i = 0; i < n; i++) {
        if (fread(&item, sizeof(TRegistro), 1, arq) != 1) break;
        printf("%08ld | %05.1f | %s | %-50s | %s\n",
               item.inscricao, item.nota, item.estado, item.cidade, item.curso);
    }
    fclose(arq);
}