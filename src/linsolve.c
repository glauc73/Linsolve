#include <stddef.h>
#include <stdio.h>
#include "generic_matrix/cMatrix.h"
#include "generic_vector/vector.h"
#include "hash.h"
#include "parse.h"
#include "consistency.h"
#include "basicSolutions.h"
#include "../include/simplex.h"

#define INPUT_BUFFER_SIZE 1024
#define MAX(X, Y) X > Y ? X : Y
#define MIN(X, Y) X < Y ? X : Y

extern vector_int* dp;
extern hash_table ht;

void input_data(char* str){
    fgets(str, INPUT_BUFFER_SIZE, stdin);
    *(str + strcspn(str, "\n")) = '\0';
}

int setargs(int argc, const char* argv[], PPLSolution_t* vbn, int* solve_sistem){
    if(argc < 2){
        printf(
            "chamada sem argumentos. use:\n"
            "-s para solucionar o sistema\n"
            "-sb para gerar as soluções basicas\n"
            "-sbv para gerar as básicas viáveis\n"
        );
        return 0;
    }
    if(strcmp(argv[1], "-sbv") == 0){
        *vbn = BASIC_FEASIBLE_SOLUTION;
    }else if(strcmp(argv[1], "-sb") == 0){
        *vbn = BASIC_SOLUTION;
    }else if(strcmp(argv[1], "-s") == 0){
        *solve_sistem = 1;
    }else{
        printf(
            "argumentos invalidos. use:\n"
            "-s para solucionar o sistema\n"
            "-sb para gerar as soluções basicas\n"
            "-sbv para gerar as básicas viáveis\n"
        );
        return 0;
    }
    return 1;
}

int print_basic_solutions(cmatrix_double** x, size_t n_x, PPLSolution_t vbn){
    if(n_x == 0){
        printf("******** Sistema sem soluções basicas viáveis ********\n\n\n");
        return 0;
    }
    
    size_t m = x[0]->rows;
    
    for(int i = 0; i < n_x; i++){
        for(size_t j = 0; j < m; j++){
            char name[SIZE_KEY + 1] = {0};
            get_key(&ht, dp->data[j], name);
            printf("%s = %.2lf\n", name, x[i]->data[j]);
        }
        putchar('\n');
    }
    
    if(vbn)
        printf("numero de soluções básicas viáveis: %zu\n", n_x);
    else
        printf("numero de soluções básicas: %zu\n", n_x);
    return 1;
}

extern FILE* step_file;

static int sort_with_columns(cmatrix_double** A){
    if(!A || !*A) return 0;
    size_t nv = dp->len;
    if(nv < 2) return 1;

    int original[nv];                       
    memcpy(original, dp->data, nv * sizeof(int));

    dp->sort(dp, NULL);                         

    cmatrix_double* old = *A;
    cmatrix_double* sorted = new_cmatrix_double(old->rows, old->cols);
    if(!sorted) return 0;

    for(size_t k = 0; k < nv; k++){
        size_t from = 0;                             
        while(original[from] != dp->data[k]) from++;

        for(size_t i = 0; i < old->rows; i++)
            sorted->at[i][k] = old->at[i][from];     
        ht.buckets[dp->data[k]].term.col = k;
    }
    for(size_t i = 0; i < old->rows; i++)            
        sorted->at[i][old->cols - 1] = old->at[i][old->cols - 1];

    cM(old, free);
    *A = sorted;
    return 1;
}

static inline void print_solution_vars(cmatrix_double* x){
    if(!x || !x->data) return;

    size_t n = x->rows < dp->len ? x->rows : dp->len;   // só variáveis de decisão
    for(size_t i = 0; i < n; i++){
        char name[SIZE_KEY + 1] = {0};
        get_key(&ht, dp->data[i], name);
        printf("[%s = %.2f]\n", name, x->data[i]);
    }
}

int main(int argc, const char* argv[]){
    step_file = fopen("steps.txt", "w");
    int solve = 0;
    PPLSolution_t vbn;
    dp = new_vector_int(0); //guarda os hashs das variaveis distintas da matriz inteira
    ht = new_hash_table();
    
    //if(!setargs(argc, argv, &vbn, &solve)) goto fim;
    
    size_t m, n;
    char input[INPUT_BUFFER_SIZE];

    scanf("%zu %zu", &m, &n);
    clearbuf();
    
    cmatrix_double* Ab = new_cmatrix_double(m, n + 1);
    restrict_t *t = calloc(Ab->rows, sizeof(*t));
    for(int i = 0; i < m; i++){
        input_data(input);
        if(parse_equation(input, Ab, i, &t[i]) == 0) break;
    }
    
    sort_with_columns(&Ab);
    
    if(solve){
        fprintf(step_file, "========================Sistema========================\n\n");
        cM(Ab, fprint, step_file);
        fprintf(step_file, "=======================================================\n\n");
        cM(Ab, rref, Ab);
        print_solution(Ab, &ht);
        goto fim;
    }
    

    /*cmatrix_double** x = NULL;
    size_t n_x;
    
    cmatrix_error err = generate_all_BS(&x, &n_x, T, vbn, &ht);
    print_basic_solutions(x, n_x, vbn);
    
    for(int i = 0; i < n_x; i++) cM(x[i], free);
    free(x);
    cM(Ab, free);
    */

    cmatrix_double* x = new_cmatrix_double(Ab->cols - 1, 1);
    cmatrix_double* fn= new_cmatrix_double(1, Ab->cols);
    fn->data[0] = 1.0;
    fn->data[1] = 1.0;
    
    cM(fn, print); 
    simplex_status stts = simplex(x, Ab, fn, SIMPLEX_MAX, t);
    printf("%d\n", stts); 
    print_solution_vars(x);

fim:
    cM(x, free);
    cM(Ab, free);
    fclose(step_file);
    delete_hash_table(&ht);
    dp->free(&dp);
    free(t);
    return 0;
}
