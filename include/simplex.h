#include "../include/genericsBase.h"
#include "../include/generic_matrix/cMatrix.h"
#include "generic_vector/vector.h"
#include "parse.h"
#include "hash.h"
#include <float.h>

#define true 1
#define false 0

typedef enum{
    SIMPLEX_MIN,
    SIMPLEX_MAX
}Optimization_t;

typedef enum{
    SIMPLEX_UNIQUE_SOLUTION,
    SIMPLEX_INFINITY_SOLUTIONS,
    SIMPLEX_ILIMITED_SOLUTION,
    SIMPLEX_NULL_POINTER
}simplex_status;

extern vector_int* dp;
extern hash_table ht;

static inline int set_initial_vnb(cmatrix_double* Ab, int* basis, int n_basis, int* VB, int nVB, restrict_t* t){
    int m = Ab->rows;
    if(m != n_basis) return 0;
    int col = Ab->cols - 1;              // primeira coluna de folga/excesso
    // 1) folgas (LE, básicas) e excessos (GE, não básicas); EQ não tem coluna
    for(int i = 0; i < m; i++){
        if(t[i] == EQ) continue;
        if(col >= nVB) return 0;
        if(t[i] == LE){
            basis[i] = col;
            VB[col] = true;
        }
        col++;
    }
    // 2) artificiais, na ordem das linhas, para GE e EQ
    for(int i = 0; i < m; i++){
        if(t[i] == LE) continue;
        if(col >= nVB) return 0;
        basis[i] = col;
        VB[col] = true;
        col++;
    }
    if(col != nVB) return 0;
    return 1;
}

static inline simplex_status simplex_iterative(cmatrix_double* T, int* basis, int* VB, Optimization_t maximize){
    size_t row_func = T->rows - 1;
    int nVB = (T->cols - 1);
    int nbasis = T->rows - 1;

    int b_idx = T->cols - 1, N = T->cols;
    while(1){
        int ismax = true;
        for(int j = 0; j < T->cols - 1; j++){
            if((T->data[row_func*N + j] <= 0 && maximize) || 
                (T->data[row_func*N + j] >= 0 && !maximize)) 
                continue;

            ismax = false;
            double min_b_a = DBL_MAX, pivot;
            int min_idx = 0, pos = false;

            for(int i = 0; i < T->rows - 1; i++){
                if(T->data[i*N + j] <= 0) continue;
                pos = true;
                double b_a = T->data[i*N + b_idx] / T->data[i*N + j];
                if(b_a < min_b_a) {min_b_a = b_a; min_idx = i;}
            }
            
            if(!pos) {return SIMPLEX_ILIMITED_SOLUTION;};
            
            VB[j] = true;
            VB[basis[min_idx]] = false;
            basis[min_idx] = j;
            
            pivot = T->data[min_idx*N + j];
            mul_row_cmatrix_double(T, min_idx, 1.0/pivot);
            
            cM(T, print);
            
            for(int k = 0; k < T->rows; k++){
                if(k == min_idx) continue;
                add_rows_cmatrix_double(T, k, min_idx, -T->data[k*N + j]);
                cM(T, print);
            }
        }
        if(ismax) break;
    }

    for(int i = 0; i < T->cols - 1; i++)
        if(fabs(T->data[row_func*N + i]) < 1e-6 && !VB[i]) return SIMPLEX_INFINITY_SOLUTIONS;

    return SIMPLEX_UNIQUE_SOLUTION;
}

static inline void canonize_function_row(cmatrix_double* T, int* basis, int nbasis, size_t row_fn){
    size_t N = T->cols;
    for(int i = 0; i < nbasis; i++){
        double c = T->data[row_fn*N + basis[i]];
        if(fabs(c) < 1e-6) continue;
        add_rows_cmatrix_double(T, row_fn, i, -c);
    }
}

static inline void drop_artificials(cmatrix_double** pT, cmatrix_double* function, int n_art){
    cmatrix_double* T = *pT;

    int b_idx = T->cols - 1;
    cmatrix_double* B = new_cmatrix_double(T->rows - 1, 1);
    for(int i = 0; i < B->rows; i++) B->data[i] = T->data[i*T->cols + b_idx];

    cM(T, slice, T, 0, T->rows - 2, 0, b_idx - 1 - n_art);
    cM(T, hstack, T, B);
    cM(B, free);

    cmatrix_double* new_fn = new_cmatrix_double(1, T->cols);
    size_t ncopy = function->cols < T->cols - 1 ? function->cols : T->cols - 1;
    memcpy(new_fn->data, function->data, ncopy * sizeof(double));   // b fica 0
    cM(T, vstack, T, new_fn);
    cM(new_fn, free);

    *pT = T;
}

static inline int append_function_row(cmatrix_double** pT, cmatrix_double* function, int two_fases, int n_art, int* basis, int nbasis){
    if(!pT || !*pT || !(*pT)->data) return 0;
    cmatrix_double* T = *pT;

    cmatrix_double* new_fn = new_cmatrix_double(1, T->cols);
    if(!new_fn || !new_fn->data) return 0;

    if(two_fases){
        for(int i = 0; i < n_art; i++)
            new_fn->data[T->cols - 2 - i] = 1;
    }else{
        if(!function || !function->data){ cM(new_fn, free); return 0; }
        size_t ncopy = function->cols < T->cols ? function->cols : T->cols;
        memcpy(new_fn->data, function->data, ncopy * sizeof(double));
    }

    cM(T, vstack, T, new_fn);
    cM(new_fn, free);
    *pT = T;

    return 1;
}

static inline simplex_status simplex(cmatrix_double* x, cmatrix_double* Ab, cmatrix_double* function, Optimization_t maximize, restrict_t* t){
    if(!x || !x->data || !Ab || !Ab->data || !function || !function->data) return SIMPLEX_NULL_POINTER;
    int m = Ab->rows, n = Ab->cols - 1, nt = Ab->rows;
    int nVB = n + 2*m, nbasis = Ab->rows;
    int* VB = (int*)calloc(nVB, sizeof(int));
    int* basis = (int*)calloc(Ab->rows, sizeof(int));

    int n_art = 0;
    cmatrix_double* T = add_slack_columns(Ab, t, &n_art);
    int two_fases = n_art > 0;

    append_function_row(&T, function, two_fases, n_art, basis, nbasis);
    int row_fn = T->rows - 1;
    set_initial_vnb(Ab, basis, Ab->rows, VB, nVB, t);
    
    if(two_fases) canonize_function_row(T, basis, nbasis, row_fn);
    simplex_status res = simplex_iterative(T, basis, VB, two_fases ? SIMPLEX_MIN : maximize);
    
    if(res == SIMPLEX_ILIMITED_SOLUTION) goto fim;
    if(two_fases){
        drop_artificials(&T, function, n_art);
        canonize_function_row(T, basis, nbasis, row_fn);
        res = simplex_iterative(T, basis, VB, maximize);
    }

    int N = T->cols, b_idx = N - 1;
    for(int i = 0; i < T->rows - 1; i++)   
        x->data[basis[i]] = T->data[i*N + b_idx];

fim:
    free(VB);
    free(basis);
    cM(T, free);

    return res;
}
