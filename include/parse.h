#ifndef PARSE_H_
#define PARSE_H_

#include "generic_vector/vector.h"
#include "generic_matrix/cMatrix.h"
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
static inline void put_row_matrix(cmatrix_double* A, size_t row, double independent){
    for(int j = 0; j < dp->len; j++){
        int index = dp->data[j];
        pair content = hash_at(&ht, index);
        A->at[row][content.col] = content.coef;

        ht.buckets[index].term.coef = 0;
    }
    
    A->at[row][A->cols - 1] = independent;
}

//extrai os coeficientes do sistema apontado por ptr e insere na linha *row* de A
//return:
//0 -> erro
//1 -> sucesso

int s_dvez = 1; 

typedef enum{
    GE = -1, EQ, LE
}restrict_t;

static inline int parse_equation(char* ptr, cmatrix_double* A, size_t row, restrict_t* tipo){
    if(!ptr) return 0;
    char* start = ptr;
    double coef = 1, ind = 0;
    static int maxcol = 0;
    int hv_equal = 0;
    vector_int* dvez = new_vector_int(64);
    restrict_t rt = EQ;

    if(row == 0) maxcol = 0; //zera o contador de colunas

    while(*ptr){
        skipspace(&ptr);
        if(!*ptr) break;

        if(*ptr == '+' || *ptr == '-'){
            coef = (*ptr == '+') ? 1 : -1;
            ptr++;
        }
        else if(isdigit(*ptr) || *ptr == '.'){
            coef *= strtod(ptr, &ptr);
        }
        else if(isalpha(*ptr)){
            char name[SIZE_KEY + 1] = {0};

            if(extract_unknown(ptr, name) == 0){
                dvez->free(&dvez);
                return 0;
            }

            int index = hash(name);
            dvez->push_back(dvez, index);

            pair term = {.coef = coef};

            if(hash_contains(&ht, name) == 0){
                dp->push_back(dp, index);
                term.col = maxcol++;
                if(maxcol > (int)A->cols - 1){
                    printf("ERRO: informou sistema (%zu x %zu) mas passou %d variaveis\n",
                           A->rows, A->cols - 1, maxcol);
                    dvez->free(&dvez);
                    return 0;
                }
            }else{
                term.col = hash_get_col(&ht, name);
            }

            hash_update(&ht, name, term);
            ptr += strlen(name);   // consome o nome inteiro (x1, x10...)
        }
        else if(*ptr == '<' || *ptr == '>' || *ptr == '='){
            if(*ptr == '<') rt = LE;
            else if(*ptr == '>') rt = GE;
            else rt = EQ;

            ptr++;
            if(*ptr == '=') ptr++;        // aceita <= e >=

            hv_equal++;
            ind = strtod(ptr, &ptr);      // termo independente
        }
        else if(*ptr == '/'){
            char* back = ptr - 1;
            while(back > start && (isdigit(*back) || *back == '.')) back--;

            char* end;
            double den = strtod(ptr + 1, &end);
            if(end == ptr + 1 || den == 0){
                printf("Erro: denominador invalido\n");
                dvez->free(&dvez);
                return 0;
            }
            ptr = end;

            if(hv_equal)
                ind /= den;
            else if(isalpha(*back) && dvez->len > 0)
                ht.buckets[dvez->data[dvez->len - 1]].term.coef /= den;
            else
                coef /= den;
        }
        else{
            ptr++;
        }
    }

    if(hv_equal != 1){
        printf("Erro: equações lineares possuem exatamente um '=', '<=' ou '>='\n");
        dvez->free(&dvez);
        return 0;
    }

    put_row_matrix(A, row, ind);
    *tipo = rt;
    dvez->free(&dvez);
    return 1;
}

static inline cmatrix_double* add_slack_columns(cmatrix_double* A, restrict_t* tipo, int* n_a_vars){
    if(!A || !A->data || !tipo) return NULL;

    size_t m = A->rows, N = A->cols, n = N - 1;
    int* a_vars = malloc(m*sizeof(int));

    cmatrix_double* X = new_cmatrix_double(m, n);
    cmatrix_double* B = new_cmatrix_double(m, 1);
    if(!X || !B) return NULL;

    cM(B, getCol, A, n);
    cM(X, slice, A, 0, A->rows - 1, 0, n - 1);

    int cont_avar = 0;
    // acrescenta uma coluna de folga/excesso por restrição não-EQ
    for(size_t i = 0; i < m; i++){
        cmatrix_double* S = new_cmatrix_double(m, 1);   // zerada
        if(tipo[i] != LE) a_vars[cont_avar++] = i;
        if(tipo[i] == EQ) continue;
        S->data[i] = (tipo[i] == LE) ? +1.0 : -1.0;
        
        cM(X, hstack, X, S);
        cM(S, free);
    }
    *n_a_vars = cont_avar;
    cmatrix_double* aX = new_cmatrix_double(m, 1);
    for(int i = 0; i < *n_a_vars; i++){
        aX->data[a_vars[i]] = 1;
        cM(X, hstack, X, aX);
        aX->data[a_vars[i]] = 0;
    }
    cM(aX, free);
    // por último, a coluna b
    cM(X, hstack, X, B);
    cM(B, free);
    return X;
}
#endif
