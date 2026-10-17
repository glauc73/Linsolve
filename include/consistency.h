#include "generic_matrix/cMatrix.h"

extern vector_int* dp;

typedef enum{
    SI = -1,
    SPI,
    SPD
}Consistency;

Consistency Analisys_solution(cmatrix_double* A, int quant_var){
    size_t i, j, r = 0; //r é a quantidade de linhas nao nulas
    for(i = 0; i < A->rows; i++){
        int hv_pivot = 0;
        for(j = 0; j < dp->len; j++){
            if(equal(A->at[i][j], 1)){ //encontrou o pivo numa linha
                hv_pivot = 1;
                r++; //adiciona a quantidade de linahs nao nulas
                break;
            }
        }
        if(!hv_pivot && !equal(A->at[i][A->cols - 1], 0)) return SI; //se na linha nao encontrou pivo, testa se é SI
    }
    if(r < quant_var) return SPI; //se r < quantidade de variaveis, entao existem r - quant_var variaveis livres

    return SPD;
}

void print_SPI_solution(cmatrix_double* A, hash_table* ht){
    int i, j, pivo_col = 0;
    char name_pivot[SIZE_KEY + 1] = {0}, free_var[SIZE_KEY + 1] = {0};
    for(i = 0; i < A->rows; i++){
        double ind = A->at[i][A->cols - 1];
        int hv_pivot = 0, hv_var = 0; //flag ativada ao encontrar um pivot
        for(j = pivo_col; j < dp->len; j++){ //cada pivo so ocorre na coluna mais à direita
            if(equal(A->at[i][j], 1) && !hv_pivot){ //so executa enquanto nao achar pivo
                hv_pivot = 1;
                get_key(ht, dp->data[j], name_pivot);//obtem o nome a partir da coluna no hash
                printf("%s = ", name_pivot); //ao encontrar o primeiro 1 da coluna, liga a flag e printa a variavel naquela coluna

                if(fabs(ind) < eps) ind = 0;  //remove -0.00
                else printf("%.2lf ", ind);  // so printa se for diferente de 0
                pivo_col++;
            } else if(!equal(A->at[i][j], 0)){
                hv_var = 1;
                get_key(ht, dp->data[j], free_var);
                printf("%+.2lf%s ", -A->at[i][j], free_var); //mostra as variaveis livres
            }
        }
        if(hv_pivot){
            if(!hv_var) printf("%.2lf ", ind);  // pode ser que nao tenha variaveis livres. nesse caso, printa o zero
            putchar('\n');
        }
            
    }
}

Consistency print_solution(cmatrix_double* A, hash_table* ht){
    Consistency discussion = Analisys_solution(A, dp->len);
    switch (discussion)
    {
    case SPI:
        printf("Sistema com infinitas soluções (SPI)\n");
        print_SPI_solution(A, ht);
        break;
    case SI:
        printf("Sistema sem solução (SI)\n");
        break;
    case SPD:
        for(int i = 0; i < dp->len; i++){
            char name[SIZE_KEY + 1] = {0};
            get_key(ht, dp->data[i], name);
            printf("%s = %.2lf\n", name, A->at[i][A->cols - 1]);
        }
        break;
    }
    return discussion;
}
