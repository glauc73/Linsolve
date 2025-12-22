#include "genericsBase.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#ifndef GENERICS_TYPE
#define GENERICS_TYPE int
#endif

#ifndef GENERICS_NAME
#define GENERICS_NAME int
#endif

#ifndef tostr
#define tostr(x) #x
#endif

#ifndef eps
#define eps 1e-8
#endif

#define MATRIX_TYPE CONCAT(matrix_, GENERICS_NAME)

typedef struct MATRIX_TYPE{
    GENERICS_TYPE* data;
    GENERICS_TYPE** at;
    size_t rows, cols;
    void (*mult)(struct MATRIX_TYPE**, const struct MATRIX_TYPE*, const struct MATRIX_TYPE*);
    void (*add)(struct MATRIX_TYPE**, const struct MATRIX_TYPE*, const struct MATRIX_TYPE*);
    void (*sub)(struct MATRIX_TYPE**, const struct MATRIX_TYPE*, const struct MATRIX_TYPE*);
    void (*pow)(struct MATRIX_TYPE**, const struct MATRIX_TYPE*, uint16_t);
    void (*scale)(struct MATRIX_TYPE**, const struct MATRIX_TYPE*, GENERICS_TYPE);
    void (*cpy)(struct MATRIX_TYPE**, const struct MATRIX_TYPE*);
    void (*free)(struct MATRIX_TYPE**);
    void (*identity)(struct MATRIX_TYPE*);
    void (*print)(struct MATRIX_TYPE*);

    void (*ref)(struct MATRIX_TYPE*);
    void (*hstack)(struct MATRIX_TYPE**, const struct MATRIX_TYPE*);
    void (*apply)(struct MATRIX_TYPE**, GENERICS_TYPE (*)(GENERICS_TYPE, void*), void*);
    GENERICS_TYPE (*det)(struct MATRIX_TYPE*);
    
    #if defined(_MATRIX_DOUBLE_) || defined(_MATRIX_FLOAT_)
    void (*rref)(struct MATRIX_TYPE*); 
    int (*inv)(struct MATRIX_TYPE**);
    #endif

    #if !defined(__INTELLISENSE__) && !defined(__clangd__) //Atributos privados /nao mexa
    GENERICS_TYPE det_cache;
    int8_t was_modified;
    #endif
}MATRIX_TYPE;


#define NEW_MATRIX_TYPE CONCAT(new_, MATRIX_TYPE)
static inline MATRIX_TYPE* NEW_MATRIX_TYPE(size_t rows, size_t cols);

static inline int CONCAT(comp_, MATRIX_TYPE)(const MATRIX_TYPE** A, const MATRIX_TYPE* B){
    if(!A || !B) return -1;
    if((*A)->rows != B->rows || (*A)->cols != B->cols){
        printf("O numero de linhas e colunas das matrizes precisam ser iguais\n");
        return -1;
    }
    for(size_t i = 0; i < (*A)->rows; i++){
        for(size_t j = 0; j < (*A)->cols; j++){
            if(!equal((*A)->at[i][j], B->at[i][j])) return 0;
        }
    }
    return 1;
}

static inline void CONCAT(swap_rows_, MATRIX_TYPE)(MATRIX_TYPE* mat, size_t r1, size_t r2){
    if(r1 >= mat->rows || r2 >= mat->rows) return;
    GENERICS_TYPE* temp = mat->at[r1];
    mat->at[r1] = mat->at[r2];
    mat->at[r2] = temp;
}

#if !defined(__INTELLISENSE__) && !defined(__clangd__)


static inline void CONCAT(delete_, MATRIX_TYPE)(MATRIX_TYPE** self){
    if(self == NULL || *self == NULL) return;
    free((*self)->data);
    free((*self)->at);
    free((*self));
    *self = NULL;
}


static inline void CONCAT(copy_, MATRIX_TYPE)(MATRIX_TYPE** self, const MATRIX_TYPE* B){
    if((*self)->rows != B->rows || (*self)->cols != B->cols){
        printf("Matrizes precisam ter dimensões iguais para serem copiadas!\n");
        return;
    } 
    memcpy((*self)->data, B->data, (*self)->rows * (*self)->cols * sizeof(GENERICS_TYPE));

    (*self)->was_modified = B->was_modified;

    if(!(*self)->was_modified){
        (*self)->det_cache = B->det_cache;
    }
}

static inline void CONCAT(add_, MATRIX_TYPE)(MATRIX_TYPE** self, const MATRIX_TYPE* A, const MATRIX_TYPE* B){
    if(A->rows != B->rows || A->cols != B->cols){
        printf("O numero de linhas e colunas das matrizes precisam ser iguais\n");
        return;
    }

    for(size_t i = 0; i < (*self)->rows; i++){
        for(size_t j = 0; j < (*self)->cols; j++){
            (*self)->at[i][j] = A->at[i][j] + B->at[i][j];
        }
    }
    (*self)->was_modified = 1;
}

static inline void CONCAT(sub_, MATRIX_TYPE)(MATRIX_TYPE** self, const MATRIX_TYPE* A, const MATRIX_TYPE* B){
    if(A->rows != B->rows || A->cols != B->cols){
        printf("O numero de linhas e colunas das matrizes precisam ser iguais\n");
        return;
    }

    for(size_t i = 0; i < (*self)->rows; i++){
        for(size_t j = 0; j < (*self)->cols; j++){
            (*self)->at[i][j] = A->at[i][j] - B->at[i][j];
        }
    }
    (*self)->was_modified = 1;
}

static inline void CONCAT(scale_, MATRIX_TYPE)(MATRIX_TYPE** self, const MATRIX_TYPE* A, GENERICS_TYPE k){
    if(!self || !A){
        printf("ponteiro nulo!\n");
        return;
    }

    for(size_t i = 0; i < (*self)->rows; i++){
        for(size_t j = 0; j < (*self)->cols; j++){
            (*self)->at[i][j] = k * A->at[i][j];
        }
    }
    (*self)->was_modified = 1;

}

static inline void CONCAT(mul_, MATRIX_TYPE)(MATRIX_TYPE** self, const MATRIX_TYPE* A, const MATRIX_TYPE* B){
    if(!self || !A || !B){
        printf("ponteiro nulo!\n");
        return;
    }
    if(A->cols != B->rows){
        printf("O numero de colunas de A(%zu x %zu) deve ser igual ao numero de linhas de B(%zu x %zu)\n",
             A->rows, A->cols, B->rows, B->cols
        );
        return;
    }
    
    MATRIX_TYPE* TMP = NEW_MATRIX_TYPE((*self)->rows, (*self)->cols);
    
    for(size_t i = 0; i < TMP->rows; i++){
        for(size_t k = 0; k < A->cols; k++){
            GENERICS_TYPE aik = A->at[i][k];
            for(size_t j = 0; j < TMP->cols; j++){
                TMP->at[i][j] += aik*B->at[k][j];
            }
        }
    }
    (*self)->cpy(self, TMP);
    
    TMP->free(&TMP);

    (*self)->was_modified = 1;
}

static inline void CONCAT(identity_, MATRIX_TYPE)(MATRIX_TYPE* A){
    for(size_t i = 0; i < A->rows; i++){
        for(size_t j = 0; j < A->cols; j++){
            A->at[i][j] = (i == j);
        }
    }
    A->det_cache = 1;
}

static inline void CONCAT(pow_, MATRIX_TYPE)(MATRIX_TYPE** self, const MATRIX_TYPE* A, uint16_t n){
    if(!self || !A){
        printf("ponteiro nulo!\n");
        return;
    }
    if(A->cols != A->rows) {
        printf("Potenciacao so funciona em matrizes quadradas\n");
        return ;
    }

    MATRIX_TYPE* base = NEW_MATRIX_TYPE(A->rows, A->cols);
    base->cpy(&base, A);

    while(n > 0){
        if(n % 2 == 1){
           (*self)->mult(self, *self, base);
        }
        base->mult(&base, base, base);
        n /= 2;
    }
    CONCAT(delete_, MATRIX_TYPE)(&base);

    (*self)->was_modified = 1;
}

static inline void CONCAT(add_rows_, MATRIX_TYPE)(MATRIX_TYPE* mat, size_t r1, size_t r2, double k){
    if(!mat){
        printf("ponteiro Nulo\n");
    }
    for(size_t j = 0; j < mat->cols ; j++)
        mat->at[r1][j] += (GENERICS_TYPE)(double)(mat->at[r2][j] * k);
}

static inline void CONCAT(mul_row_, MATRIX_TYPE)(MATRIX_TYPE* mat, size_t row, GENERICS_TYPE k){
    if(!mat){
        printf("ponteiro Nulo\n");
        return;
    }
    for(size_t j = 0; j < mat->cols ; j++)
        mat->at[row][j] *= k;
    mat->was_modified = 1;
}

static inline int CONCAT(sub_iszero_, MATRIX_TYPE)(MATRIX_TYPE* mat, size_t i0, size_t j0, size_t jmax){
    if(j0 > jmax || j0 >= mat->cols) return 1;
    if(jmax >= mat->cols) jmax = mat->cols - 1;
    for(size_t i = i0; i < mat->rows; i++){
        for(size_t j = j0; j <= jmax; j++){
            if(!equal(mat->at[i][j], 0)) return 0;
        }
    }
    return 1;
}

static inline int CONCAT(echelon_, MATRIX_TYPE)(MATRIX_TYPE* mat){
    int j0 = 0;
    for(size_t i = 0; i < mat->rows; i++){
        size_t cont = 0;
        for(size_t j = j0; j < mat->cols; j++){
            if(equal(mat->at[i][j], 1)){
                if(!CONCAT(sub_iszero_, MATRIX_TYPE)(mat, i + 1, j0, j))
                    return 0;
                j0 = j + 1;
                break;
            }
            cont++;
        }
        if(cont == mat->cols - j0)  
            return CONCAT(sub_iszero_, MATRIX_TYPE)(mat, i + 1, j0, mat->cols - 1);
    }
    return 1;
}

static inline GENERICS_TYPE CONCAT(det_, MATRIX_TYPE)(MATRIX_TYPE* self){
    if(self->cols != self->rows){
        printf("Determinante so e definido para matrizes quadradas\n");
        return 0;
    }
    if(!self->was_modified) return self->det_cache;

    MATRIX_TYPE* TMP = NEW_MATRIX_TYPE(self->rows, self->cols);
    TMP->cpy(&TMP, self);

    GENERICS_TYPE det_fact = 1;
    self->det_cache = 1;

    size_t i, j, i0 = 0;
    for(j = 0; j < TMP->cols; j++){
        i = i0;
        while(i < self->rows && equal(TMP->at[i][j], 0)) i++;
        if(i == self->rows)
            continue;
        if(i != i0){
            CONCAT(swap_rows_, MATRIX_TYPE)(TMP, i0, i);
            self->det_cache *= -1;
        }
    
        for(i = i0 + 1; i < TMP->rows; i++){
            if(!equal(TMP->at[i][j], 0)){
                double pivot = TMP->at[i0][j];
                double aij = TMP->at[i][j];

                CONCAT(mul_row_, MATRIX_TYPE)(TMP, i, pivot);
                det_fact *= pivot;

                CONCAT(add_rows_, MATRIX_TYPE)(TMP, i, i0, -aij);
            }
        }
        i0++;
    }
    
    for(size_t i = 0; i < TMP->rows; i++){
        self->det_cache *= TMP->at[i][i];
    }
    self->det_cache /= det_fact;
    
    TMP->free(&TMP);
    self->was_modified = 0;

    return self->det_cache;
}

static inline void CONCAT(REF_, MATRIX_TYPE)(MATRIX_TYPE* mat){
    if(CONCAT(echelon_, MATRIX_TYPE)(mat))  return;

    size_t i, j, i0 = 0;
    for(j = 0; j < mat->cols; j++){
        i = i0;
        while(i < mat->rows && equal(mat->at[i][j], 0)) i++;
        if(i == mat->rows)
            continue;
        if(i != i0){
            CONCAT(swap_rows_, MATRIX_TYPE)(mat, i0, i);
        }
    
        for(i = i0 + 1; i < mat->rows; i++){
            if(!equal(mat->at[i][j], 0)){
                double pivot = mat->at[i0][j];
                double aij = mat->at[i][j];

                CONCAT(mul_row_, MATRIX_TYPE)(mat, i, pivot);
                CONCAT(add_rows_, MATRIX_TYPE)(mat, i, i0, -aij);
            }
        }
        i0++;
    }
    
    mat->was_modified = 1;
}

static inline void CONCAT(concat_, MATRIX_TYPE)(MATRIX_TYPE** self, const MATRIX_TYPE* A){
    if(A->rows != (*self)->rows){
        printf("A concatenacao horizontal de matrizes exige o mesmo numero de linhas");
        return;
    }

    size_t new_cols = (*self)->cols + A->cols;

    MATRIX_TYPE* new_matrix = NEW_MATRIX_TYPE((*self)->rows, new_cols);

    for(size_t i = 0; i < (*self)->rows; i++){
        for(size_t j = 0; j < new_cols; j++){
            if(j < (*self)->cols){
                new_matrix->at[i][j] = (*self)->at[i][j];
            } else {
                new_matrix->at[i][j] = A->at[i][j - (*self)->cols];
            }
        }
    }

    (*self)->free(self);
    *self = new_matrix;
}


#if defined(_MATRIX_DOUBLE_) || defined(_MATRIX_FLOAT_)

static inline void CONCAT(RREF_, MATRIX_TYPE)(MATRIX_TYPE* mat){
    mat->ref(mat);
    size_t j0 = 0, i = 0;
    for(i = 0; i < mat->rows; i++){
        size_t j = j0;
        while(j < mat->cols && equal(mat->at[i][j], 0)) j++;
        if(j == mat->cols) continue;

        double pivot = mat->at[i][j];
        CONCAT(mul_row_, MATRIX_TYPE)(mat, i, 1/pivot);

        for(int k = i - 1; k >= 0; k--){
            if(!equal(mat->at[k][j], 0)){
                CONCAT(add_rows_, MATRIX_TYPE)(mat, k, i, -mat->at[k][j]);
            }
        }
        j0 = j + 1; // como um pivo na coluna seguinte só pode acontecer mais a direita, procuramos a partir da direita de j;
    }
    mat->was_modified = 1;
}

static inline int CONCAT(inverse_, MATRIX_TYPE)(MATRIX_TYPE** self){
    if((*self)->cols != (*self)->rows){
        printf("Erro: A matriz nao é quadrada, portanto nao possui inversa\n");
        return 0;
    }
    MATRIX_TYPE* I = NEW_MATRIX_TYPE((*self)->rows, (*self)->cols);
    CONCAT(identity_, MATRIX_TYPE)(I);
    
    MATRIX_TYPE* TMP = NEW_MATRIX_TYPE((*self)->rows, (*self)->cols);
    TMP->cpy(&TMP, *self);

    CONCAT(concat_, MATRIX_TYPE)(&TMP, I);
    TMP->rref(TMP);

    for(size_t i = 0; i < TMP->rows; i++){
        if(!equal(TMP->at[i][i], 1)){
            printf("Erro: Nao possui matriz inversa!\n");
            I->free(&I);
            TMP->free(&TMP);
            return 0;
        }
    }

    for(size_t i = 0; i < I->rows; i++){
        for(size_t j = 0; j < I->cols; j++){
            I->at[i][j] = TMP->at[i][j + I->cols];
        }
    }

    (*self)->free(self);
    TMP->free(&TMP);
    *self = I;
    return 1;
}

#endif

static inline void CONCAT(print_, MATRIX_TYPE)(MATRIX_TYPE* self){
    if(!self) {
        printf("Ponteiro Nulo");
        return;
    }

    for(size_t i = 0; i < self->rows; i++){
        for(size_t j = 0; j < self->cols; j++){
            if(equal(self->at[i][j], 0))
                self->at[i][j] = 0;
            printg(self->at[i][j]);
            putchar('\t');
        }
        putchar('\n');
    }
}

#endif

static inline void CONCAT(apply_, MATRIX_TYPE)(MATRIX_TYPE** self, GENERICS_TYPE (*func)(GENERICS_TYPE, void*), void* params){
    if(self == NULL || *self == NULL) {
        printf("Ponteiro Nulo\n");
        return;
    }

    size_t quant = (*self)->rows * (*self)->cols;
    GENERICS_TYPE* ptr = (*self)->data;
    for(size_t i = 0; i < quant; i++){
        ptr[i] = func(ptr[i], params);
    }
}

static inline MATRIX_TYPE* NEW_MATRIX_TYPE(size_t rows, size_t cols){
    MATRIX_TYPE* mat = (MATRIX_TYPE*) malloc(sizeof(MATRIX_TYPE));
    if(!mat){
        printf("memoria insuficiente\n");
        return NULL;
    }

    mat->data = (GENERICS_TYPE*) calloc(rows*cols, sizeof(GENERICS_TYPE));
    if(!mat->data) {
        printf("memoria insuficiente\n");
        free(mat);
        return NULL;
    }

    mat->at = (GENERICS_TYPE**) malloc(rows*sizeof(GENERICS_TYPE*));
    if(!mat->at){
        printf("Memoria insuficiente\n");
        free(mat->data);
        free(mat);
        return NULL;
    }

    mat->rows = rows;
    mat->cols = cols;

    for(size_t i = 0; i < mat->rows; i++)
        mat->at[i] = mat->data + i*mat->cols;
        
    mat->cpy = CONCAT(copy_, MATRIX_TYPE);
    mat->scale = CONCAT(scale_, MATRIX_TYPE);
    mat->add = CONCAT(add_, MATRIX_TYPE);
    mat->sub = CONCAT(sub_, MATRIX_TYPE);
    mat->mult = CONCAT(mul_, MATRIX_TYPE);
    mat->pow = CONCAT(pow_, MATRIX_TYPE);
    mat->free = CONCAT(delete_, MATRIX_TYPE);
    mat->ref = CONCAT(REF_, MATRIX_TYPE);
    mat->det = CONCAT(det_, MATRIX_TYPE);
    mat->print = CONCAT(print_, MATRIX_TYPE);
    mat->identity = CONCAT(identity_, MATRIX_TYPE);
    mat->hstack = CONCAT(concat_, MATRIX_TYPE);
    mat->apply = CONCAT(apply_, MATRIX_TYPE);

    #if defined(_MATRIX_DOUBLE_) || defined(_MATRIX_FLOAT_)
    mat->rref = CONCAT(RREF_, MATRIX_TYPE);
    mat->inv = CONCAT(inverse_, MATRIX_TYPE);
    #endif
    
    mat->was_modified = 1;

    return mat;
}

static inline MATRIX_TYPE* CONCAT(get_, MATRIX_TYPE)(size_t rows, size_t cols){
    MATRIX_TYPE* mat = NEW_MATRIX_TYPE(rows, cols);
    if(!mat){
        printf("Memoria insuficiente\n");
        return NULL;
    }
    for(size_t i = 0; i < mat->rows; i++){
        for(size_t j = 0; j < mat->cols; j++){
            input(mat->at[i][j]);
        }
    }
    return mat;
}

static inline MATRIX_TYPE* CONCAT(init_, MATRIX_TYPE)(MATRIX_TYPE* A){
    if(!A){
        printf("Ponteiro Nulo\n");
        return NULL;
    }
    MATRIX_TYPE* TMP = NEW_MATRIX_TYPE(A->rows, A->cols);
    if(!TMP){
        printf("Memoria insuficiente!\n");
        return NULL;
    }
    TMP->cpy(&TMP, A);
    return TMP;
}