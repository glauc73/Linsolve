#include <stdio.h>
#include "generic_matrix/Matrix.h"
#include "generic_vector/vector.h"
#include "generic_stack/stack.h"
#include "hash_term.h"
#include "mergesort.h"
#include <ctype.h>

#define INPUT_BUFFER_SIZE 1024
#define MAX(X, Y) X > Y ? X : Y
#define MIN(X, Y) X < Y ? X : Y

vector_int* dp;

typedef enum{
    SI = -1,
    SPI,
    SPD
}Consistency;

void skipspace(char** ptr){
    while(**ptr == ' ') (*ptr)++;
}

void clearbuf(){
    int c;
    while(c = getchar() != '\n' && c != EOF);
}

int extract_unknown(char* ptr, char* unknown){
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
void put_row_matrix(hash_table* ht, matrix_double* A, size_t row, vector_int* dvez, double independent){
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
int parse_equation(hash_table *ht, char* ptr, matrix_double* A,  size_t row){
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

Consistency Analisys_solution(matrix_double* A, int quant_var){
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

void print_SPI_solution(matrix_double* A, hash_table* ht){
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

Consistency print_solution(matrix_double* A, hash_table* ht){
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

void input_data(char* str){
    fgets(str, INPUT_BUFFER_SIZE, stdin);
    *(str + strcspn(str, "\n")) = '\0';
}

int main(){
    size_t m, n;
    char input[INPUT_BUFFER_SIZE];
    double ind;
    hash_table table = new_hash_table();
    dp = new_vector_int(0); //guarda os hashs das variaveis distintas da matriz inteira

    printf("informe o numero de equações e de variaveis\n");
    scanf("%zu %zu", &m, &n);
    clearbuf();
    printf("informe o sistema:\n\n");
    
    matrix_double* A = new_matrix_double(m, n + 1);
    
    for(int i = 0; i < m; i++){
        input_data(input);
        if(parse_equation(&table, input, A, i) == 0) break;
    }
    
    dp->sort(dp, NULL);

    A->rref(A);
    print_solution(A, &table);

    delete_hash_table(&table);
    A->free(&A);
    dp->free(&dp);
    return 0;
}