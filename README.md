## Linsolve

Uma calculadora de sistemas lineares escrita em C, do zero — sem bibliotecas externas.

Você escreve as equações, o programa resolve e te diz o que aconteceu: se existe uma única solução, ele mostra os valores; se existem infinitas, ele mostra como elas são parametrizadas; e se não existe nenhuma, ele avisa.

Mas ele vai além disso. Com outra flag, em vez de resolver o sistema ele **enumera todas as soluções básicas** — e, se você pedir, só as **básicas viáveis**. Isso é o ferramental de programação linear: as soluções básicas viáveis são exatamente os vértices da região factível, e é entre elas que mora o ótimo de um PPL.

A outra parte interessante é o parser. Você escreve as equações do jeito que quiser, misturando a ordem das variáveis e introduzindo variáveis novas no meio do sistema, e ele se vira.

---

## Compilando

```bash
gcc -std=c11 src/linsolve.c -Iinclude -o linsolve
```

Ou use o script que já está no repositório:

```bash
bash build.sh
```

---

## Argumentos de linha de comando

O programa **exige** exatamente uma flag. Chamado sem nenhuma — ou com algo que ele não reconhece — ele imprime as opções disponíveis e encerra.

| Flag   | O que faz                                                                   |
|--------|------------------------------------------------------------------------------|
| `-s`   | Resolve o sistema. Reduz a matriz aumentada à forma escalonada reduzida (RREF) e classifica a solução. |
| `-sb`  | Gera todas as **soluções básicas** do sistema.                               |
| `-sbv` | Gera apenas as **soluções básicas viáveis** (as de componentes não-negativas). |

```bash
./linsolve -s   < in.txt
./linsolve -sb  < in.txt
./linsolve -sbv < in.txt
```

---

## A entrada

A entrada tem duas partes, nesta ordem:

1. **Uma linha com dois números**: a quantidade de equações (`m`) e a quantidade de variáveis (`n`).
2. **`m` linhas**, uma equação por linha.

Rodando interativamente o programa te guia:

```
$ ./linsolve -s
informe o numero de equações e de variaveis
3 3
informe o sistema:

2x + 3y - z = 5
x - y + 2z = 3
3x + y + z = 7
```

Por isso o `in.txt` precisa começar com as dimensões:

```
3 3
2x + 3y - z = 5
x - y + 2z = 3
3x + y + z = 7
```

### Nomes de variáveis

| Forma               | Exemplos                      |
|---------------------|-------------------------------|
| Letras soltas       | `a`, `b`, `x`, `y`, `z`       |
| Indexadas, 0 a 99   | `c0`, `c1`, `c11`, ..., `c99` |

### Você não precisa ser organizado

Isto aqui é um sistema perfeitamente válido:

```
3 3
2x + 3y - z = 5
-z + 4x = 1
y = 3
```

Repare: a segunda equação inverteu a ordem dos termos e pulou o `y` inteiro; a terceira tem uma variável só. Tudo bem. O programa monta a matriz de coeficientes **por nome de variável, não por posição** — cada termo é encaixado na coluna certa independentemente de onde foi escrito, e variáveis ausentes entram com coeficiente zero.

Variáveis novas também podem aparecer só lá no fim:

```
3 4
a + b = 10
a - b = 2
c0 + c1 = 7
```

---

## Os três desfechos de `-s`

**Solução única** — tantas equações independentes quanto variáveis:

```
2 2
2x + 3y = 8
x -  y = 1
```

Cada variável recebe um valor e acabou.

**Infinitas soluções** — sobram variáveis livres:

```
2 3
x + y + z = 6
2x + 2y + 2z = 12
```

A segunda equação é só a primeira multiplicada por 2 — não acrescenta informação nenhuma. O resultado sai em forma paramétrica: algumas variáveis ficam livres, as outras são escritas em função delas.

**Sem solução** — as equações se contradizem:

```
2 2
x + y = 5
x + y = 9
```

Não existe par de números que satisfaça as duas. O programa detecta a inconsistência e avisa.

---

## O que `-sb` e `-sbv` devolvem

Ambos imprimem quantas soluções foram encontradas, listam cada uma como um vetor, e fecham repetindo o total.

Uma solução básica se obtém escolhendo um conjunto de colunas para formar uma base, zerando as demais variáveis e resolvendo o que sobra. O programa percorre essas combinações e coleta as que dão certo. Com `-sbv`, ele descarta as que têm alguma componente negativa — sobram os vértices da região factível.

Se nenhuma sobreviver ao filtro, ele diz que o sistema não tem soluções básicas viáveis.

---

## Como o código está organizado

O `linsolve.c` fica em `src/`. Todo o resto são headers próprios em `include/`:

| Módulo                        | Papel                                                                 |
|-------------------------------|-----------------------------------------------------------------------|
| `genericsBase.h`              | A base de tudo: a maquinaria de tipos genéricos que os outros módulos usam, para que as estruturas não fiquem presas a um tipo específico. |
| `generic_vector/vector.h`     | TAD de vetor dinâmico genérico — `pushback`, `len`, `sort` e companhia. |
| `generic_matrix/cMatrix.h`    | Matrizes genéricas, com operações como `rref`. É aqui que a matriz aumentada vive e é reduzida. |
| `hash.h`                      | Hash table perfeita que mapeia nome de variável para coluna da matriz. |
| `parse.h`                     | Lê uma linha de texto e a transforma em uma linha da matriz aumentada. |
| `consistency.h`               | Classifica o sistema após a redução e imprime a solução.              |
| `basicSolutions.h`            | Enumera as soluções básicas e filtra as viáveis.                      |

A hash table é o que permite escrever as equações fora de ordem: quando o parser encontra `z`, consulta a tabela e descobre em que coluna aquele termo deve ser somado — criando a coluna na hora, se for a primeira vez que aquela variável aparece.

Nada disso depende de biblioteca externa. Um GCC com suporte a C11 e você está pronto.

---

## Requisitos

- GCC com `-std=c11`
- Linux ou macOS (no Windows, via WSL)
