#include "generic_vector/vector.h"
#include <ctype.h>


vector_int* dp;

static inline void skipspace(char** ptr){
    while(**ptr == ' ') (*ptr)++;
}

static inline void clearbuf(){
    int c;
    while(c = getchar() != '\n' && c != EOF);
}

static inline int extract_unknown(char* ptr, char* unknown){
    unknown[0] = *ptr++;
    int i = 1;
    if(isalpha(*ptr)){
        printf("Erro: sistemas lineares nao possuem produtos de incognitas\n");
        return 0;
    }
    while(isdigit(*ptr)){
        if(i >= SIZE_KEY){
            printf("Erro: esse programa so resolve sistemas ate 100 x 100\n");
            return 0;
        }
        unknown[i] = *ptr++;
        i++;
    }
    unknown[SIZE_KEY] = '\0';
}

//adiciona os coeficientes guardados em ht nos indices dvez e os termos independentes do sistema à matriz A
static inline void put_row_matrix(hash_table* ht, cmatrix_double* A, size_t row, vector_int* dvez, double independent){
    for(int j = 0; j < dvez->len; j++){
        int index = dvez->data[j];
        pair content = hash_at(ht, index);
        A->at[row][content.col] = content.coef;

        ht->buckets[index].term.coef = 0;
    }
    A->at[row][A->cols - 1] = independent;
}

//extrai os coeficientes do sistema apontado por ptr e insere na linha *row* de A
//return:
//0 -> erro
//1 -> sucesso
static inline int parse_equation(hash_table *ht, char* ptr, cmatrix_double* A,  size_t row){
    //ht é um hash que guarda um termo da soma (coeficiente, coluna)
    if(!ptr) return 0;
    double coef = 1, ind; //ind guarda o termo independente
    static int maxcol = 0; //variavel statica para contar o maximo de colunas
    int hv_equal = 0; //flag que diz se tem igualdade
    vector_int* dvez = new_vector_int(64); //vetor que guarda os hashs das incognitas dessa linha

    while(*ptr){
        skipspace(&ptr);
        if(*ptr == '+' || *ptr == '-'){
            int x = *ptr - '+';
            coef = 1 - x; //se *ptr == '+', coef == 1; se *ptr == '-', coef == -1;
            ptr++;
        }
        else if(isdigit(*ptr)){
            coef *= strtod(ptr, &ptr); //converte o coeficiente para double e aplica o sinal
        }
        else if(isalpha(*ptr)){
            char name[SIZE_KEY + 1] = {0}; //string que guarda o nome de uma incognita

            if(extract_unknown(ptr, name) == 0) { // coloca a incognita encontrada em name
                dvez->free(&dvez);
                return 0;
            }

            int index = hash(name);
            dvez->push_back(dvez, index); //coloca o indice nas incognitas da vez (da linha)

            pair term = {.coef = coef}; // inicializa o termo

            if(hash_contains(ht, name) == 0){ //se nao tiver conteudo nessa chave, entao é uma nova incognita
                dp->push_back(dp, index); //guarda o hash na dp para acessar coluna -> name em O(1)
                term.col = maxcol++; //acresenta uma coluna, pois achou nova variavel
                if(maxcol > A->cols - 1){
                    printf("ERRO: informou sistema (%zu x %zu) mas passou %d variaveis\n", A->rows, A->cols - 1, maxcol);
                    dvez->free(&dvez);
                    return 0;
                }
            }else{
                term.col = hash_get_col(ht, name); //se a incognita ja esta la, recebe a coluna dela
            }

            hash_update(ht, name, term); //atualiza o conteudo do hash na posição de key
            ptr++;
        }
        else if(*ptr == '='){
            ptr++;
            hv_equal++;
            ind = strtod(ptr, &ptr); //captura o termo independente
        }
        else{
            ptr++; // ignora qualquer outra coisa
        }
    }
    put_row_matrix(ht, A, row, dvez, ind); //adiciona os coeficientes extraidos na matriz A
    dvez->free(&dvez);

    if(hv_equal != 1) {
        printf("Erro: equações lineares possuem exatamente uma igualdade\n");
        return 0;
    }
    return 1;
}

