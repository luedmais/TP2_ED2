#include "quicksort.h"

typedef struct {
    TRegistro Item;
    int Ant;
    int Prox;
} TCelulaPivo;

typedef struct {
    TCelulaPivo Itens[TAM_PIVO];
    int Primeiro;
    int Ultimo;
    int CelulasDisp;
    int num_cel_ocupadas;
} TPivo;

// Inicializa o pivo e a lista de celulas disponiveis
//pivo usado para ordenacao externa
static void inicializaPivo(TPivo *pivo) {
    pivo->num_cel_ocupadas = 0;
    pivo->Primeiro = -1;
    pivo->Ultimo = -1;
    pivo->CelulasDisp = 0;
    for (int i = 0; i < TAM_PIVO; i++) {
        pivo->Itens[i].Ant = -1;
        pivo->Itens[i].Prox = (i == TAM_PIVO - 1) ? -1 : i + 1;
    }
}

// Insere o registro do pivo mantendo a ordem das notas
static void insereItemPivo(TRegistro Item, TPivo *pivo, TMetricas *m) {
    if (pivo->num_cel_ocupadas == TAM_PIVO) return;

    int disp = pivo->CelulasDisp;
    pivo->CelulasDisp = pivo->Itens[disp].Prox;
    pivo->Itens[disp].Item = Item;
    pivo->num_cel_ocupadas++;

    if (pivo->num_cel_ocupadas == 1) {
        pivo->Primeiro = disp;
        pivo->Ultimo = disp;
        pivo->Itens[disp].Prox = -1;
        pivo->Itens[disp].Ant = -1;
        return;
    }

    int pos = pivo->Primeiro;
    m->comparacoes++;
    if (Item.nota < pivo->Itens[pos].Item.nota) {
        pivo->Itens[disp].Ant = -1;
        pivo->Itens[disp].Prox = pos;
        pivo->Itens[pos].Ant = disp;
        pivo->Primeiro = disp;
        return;
    }

    int indiceInsercao = pivo->Itens[pos].Prox;
    while (indiceInsercao != -1) {
        m->comparacoes++;
        if (pivo->Itens[indiceInsercao].Item.nota < Item.nota) {
            pos = indiceInsercao;
            indiceInsercao = pivo->Itens[pos].Prox;
        } else {
            break;
        }
    }

    if (indiceInsercao == -1) {
        pivo->Itens[disp].Ant = pos;
        pivo->Itens[disp].Prox = -1;
        pivo->Itens[pos].Prox = disp;
        pivo->Ultimo = disp;
        return;
    }

    pivo->Itens[disp].Ant = pos;
    pivo->Itens[disp].Prox = indiceInsercao;
    pivo->Itens[pos].Prox = disp;
    pivo->Itens[indiceInsercao].Ant = disp;
}

// Retira o registro de menor nota do pivo
static void retiraPrimeiroPivo(TPivo *pivo, TRegistro *Item) {
    if (pivo->num_cel_ocupadas == 0) return;

    int prim = pivo->Primeiro;
    *Item = pivo->Itens[prim].Item;
    int proxTmp = pivo->Itens[prim].Prox;

    pivo->Itens[prim].Prox = pivo->CelulasDisp;
    pivo->CelulasDisp = prim;
    pivo->Primeiro = proxTmp;

    if (pivo->Primeiro != -1) {
        pivo->Itens[pivo->Primeiro].Ant = -1;
    } else {
        pivo->Ultimo = -1;
    }
    pivo->num_cel_ocupadas--;
}

// Retira o registro de maior nota do pivo
static void retiraUltimoPivo(TPivo *pivo, TRegistro *Item) {
    if (pivo->num_cel_ocupadas == 0) return;

    int ult = pivo->Ultimo;
    *Item = pivo->Itens[ult].Item;
    int antTmp = pivo->Itens[ult].Ant;

    pivo->Itens[ult].Prox = pivo->CelulasDisp;
    pivo->CelulasDisp = ult;
    pivo->Ultimo = antTmp;

    if (pivo->Ultimo != -1) {
        pivo->Itens[pivo->Ultimo].Prox = -1;
    } else {
        pivo->Primeiro = -1;
    }
    pivo->num_cel_ocupadas--;
}

// Le um registro pela parte superior do arquivo
static void leSup(FILE *ArqLEs, TRegistro *UltLido, int *Ls, bool *OndeLer, TMetricas *m) {
    fseek(ArqLEs, (long)(*Ls - 1) * sizeof(TRegistro), SEEK_SET);
    fread(UltLido, sizeof(TRegistro), 1, ArqLEs);
    m->leituras++;
    (*Ls)--;
    *OndeLer = false;
}

// Le um registro pela parte inferior do arquivo
static void leInf(FILE *ArqLi, TRegistro *UltLido, int *Li, bool *OndeLer, TMetricas *m) {
    fread(UltLido, sizeof(TRegistro), 1, ArqLi);
    m->leituras++;
    (*Li)++;
    *OndeLer = true;
}

// Escreve um registro pela parte superior do arquivo
static void escreveMax(FILE *ArqLEs, TRegistro R, int *Es, TMetricas *m) {
    fseek(ArqLEs, (long)(*Es - 1) * sizeof(TRegistro), SEEK_SET);
    fwrite(&R, sizeof(TRegistro), 1, ArqLEs);
    m->escritas++;
    (*Es)--;
}

// Escreve um registro pela parte inferior do arquivo
static void escreveMin(FILE *ArqEi, TRegistro R, int *Ei, TMetricas *m) {
    fwrite(&R, sizeof(TRegistro), 1, ArqEi);
    m->escritas++;
    (*Ei)++;
}

// Divide o arquivo em partes menores e maiores
static void particao(FILE *ArqLi, FILE *ArqEi, FILE *ArqLEs, int Esq, int Dir, int *i, int *j, TMetricas *m) {
    int Ls = Dir, Es = Dir;
    int Li = Esq, Ei = Esq;
    float Linf = -1.0f;
    float Lsup = 101.0f;
    bool OndeLer = true;
    TRegistro UltLido, R;
    TPivo pivo;

    inicializaPivo(&pivo);

    fseek(ArqLi, (long)(Li - 1) * sizeof(TRegistro), SEEK_SET);
    fseek(ArqEi, (long)(Ei - 1) * sizeof(TRegistro), SEEK_SET);

    *i = Esq - 1;
    *j = Dir + 1;

    while (Ls >= Li) {
        if (pivo.num_cel_ocupadas < TAM_PIVO) {
            if (OndeLer)
                leSup(ArqLEs, &UltLido, &Ls, &OndeLer, m);
            else
                leInf(ArqLi, &UltLido, &Li, &OndeLer, m);

            insereItemPivo(UltLido, &pivo, m);
            continue;
        }

        if (Ls == Es)
            leSup(ArqLEs, &UltLido, &Ls, &OndeLer, m);
        else if (Li == Ei)
            leInf(ArqLi, &UltLido, &Li, &OndeLer, m);
        else if (OndeLer)
            leSup(ArqLEs, &UltLido, &Ls, &OndeLer, m);
        else
            leInf(ArqLi, &UltLido, &Li, &OndeLer, m);

        m->comparacoes++;
        if (UltLido.nota > Lsup) {
            *j = Es;
            escreveMax(ArqLEs, UltLido, &Es, m);
            continue;
        }

        m->comparacoes++;
        if (UltLido.nota < Linf) {
            *i = Ei;
            escreveMin(ArqEi, UltLido, &Ei, m);
            continue;
        }

        insereItemPivo(UltLido, &pivo, m);

        if (Ei - Esq < Dir - Es) {
            retiraPrimeiroPivo(&pivo, &R);
            escreveMin(ArqEi, R, &Ei, m);
            Linf = R.nota;
        } else {
            retiraUltimoPivo(&pivo, &R);
            escreveMax(ArqLEs, R, &Es, m);
            Lsup = R.nota;
        }
    }

    while (Ei <= Es) {
        retiraPrimeiroPivo(&pivo, &R);
        escreveMin(ArqEi, R, &Ei, m);
    }
}

// Ordena recursivamente as partes do arquivo
static void quicksortRec(FILE *ArqLi, FILE *ArqEi, FILE *ArqLEs, int Esq, int Dir, TMetricas *m) {
    if (Dir - Esq < 1) return;

    int i, j;
    particao(ArqLi, ArqEi, ArqLEs, Esq, Dir, &i, &j, m);

    if (i - Esq < Dir - j) {
        quicksortRec(ArqLi, ArqEi, ArqLEs, Esq, i, m);
        quicksortRec(ArqLi, ArqEi, ArqLEs, j, Dir, m);
    } else {
        quicksortRec(ArqLi, ArqEi, ArqLEs, j, Dir, m);
        quicksortRec(ArqLi, ArqEi, ArqLEs, Esq, i, m);
    }
}

// Abre o arquivo e inicia o quicksort externo
void executarQuicksortExterno(const char *nomeArquivoBin, int n, TMetricas *metricas) {
    FILE *ArqLi = fopen(nomeArquivoBin, "r+b");
    FILE *ArqEi = fopen(nomeArquivoBin, "r+b");
    FILE *ArqLEs = fopen(nomeArquivoBin, "r+b");

    if (!ArqLi || !ArqEi || !ArqLEs) {
        perror("Erro ao abrir arquivo para Quicksort Externo");
        if (ArqLi) fclose(ArqLi);
        if (ArqEi) fclose(ArqEi);
        if (ArqLEs) fclose(ArqLEs);
        return;
    }

    quicksortRec(ArqLi, ArqEi, ArqLEs, 1, n, metricas);

    fflush(ArqLi);
    fflush(ArqEi);
    fflush(ArqLEs);

    fclose(ArqLi);
    fclose(ArqEi);
    fclose(ArqLEs);
}