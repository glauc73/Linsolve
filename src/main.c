#include <stddef.h>
#include <stdio.h>
#include "generic_matrix/cMatrix.h"
#include "generic_vector/vector.h"
#include "hash.h"
#include "parse.h"
#include "consistency.h"
#include "basicSolutions.h"

#define INPUT_BUFFER_SIZE 1024
#define MAX(X, Y) X > Y ? X : Y
#define MIN(X, Y) X < Y ? X : Y

extern vector_int* dp;

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

int print_basic_solutions(cmatrix_double** x, size_t n_x, PPLSolution_t vbn, hash_table* ht){
    if(n_x == 0){
        printf("Sistema sem soluções basicas viáveis\n");
        return 0;
    }
    
    size_t m = x[0]->rows;
    
    for(int i = 0; i < n_x; i++){
        for(size_t j = 0; j < m; j++){
            char name[SIZE_KEY + 1] = {0};
            get_key(ht, dp->data[j], name);
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

int main(int argc, const char* argv[]){
    int solve;
    PPLSolution_t vbn;
    if(!setargs(argc, argv, &vbn, &solve)) goto fim;
    
    size_t m, n;
    char input[INPUT_BUFFER_SIZE];
    hash_table table = new_hash_table();
    dp = new_vector_int(0); //guarda os hashs das variaveis distintas da matriz inteira

    printf("informe o numero de equações e de variaveis\n");
    scanf("%zu %zu", &m, &n);
    clearbuf();
    printf("informe o sistema:\n\n");
    
    cmatrix_double* Ab = new_cmatrix_double(m, n + 1);
    
    for(int i = 0; i < m; i++){
        input_data(input);
        if(parse_equation(&table, input, Ab, i) == 0) break;
    }
    dp->sort(dp, NULL);
    
    if(solve){
        cM(Ab, rref, Ab);
        print_solution(Ab, &table);
        goto fim;
    }

    cmatrix_double** x = NULL;
    size_t n_x;
    
    cmatrix_error err = generate_all_BS(&x, &n_x, Ab, vbn);
    print_basic_solutions(x, n_x, vbn, &table);
    
    for(int i = 0; i < n_x; i++) cM(x[i], free);
    free(x);
fim:
    cM(Ab, free);
    delete_hash_table(&table);
    dp->free(&dp);
    return 0;
}
