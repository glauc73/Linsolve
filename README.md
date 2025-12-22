**src**  contém o codigo fonte do programa

**include** tem as dependencias necessárias, que são:

**genericsBase.h** : biblioteca base para tipos genericos;
**generic_matrix** : uma biblioteca de manipulação de matrizes com métodos e tipo generico;
**generic_vector** : implementação de um TAD vetor dinâmico generico com métodos como sort, pushback, len, etc;
**hash.h** : implementação de uma hash table perfeita para armazenar coeficientes, colunas e nomes de variaveis

O arquivo deve ser compilado como:
**gcc -std=c11 src/main.c -Iinclude -o main**

**in.txt** é um exemplo de entrada que deve ser executado da seguinte maneira:
**./src/main < in.txt**
