# 🔢 Solve Linear System

Uma calculadora em **C** que resolve sistemas lineares e classifica sua solução de três formas:

- ✅ **Solução única** — exibe os valores de cada variável
- ♾️ **Infinitas soluções** — exibe o formato paramétrico da solução
- ❌ **Sem solução** — informa que o sistema é insolúvel

---

## 📁 Estrutura do Projeto

```
Solve-Linear-System/
├── src/
│   └── main.c              # Código-fonte principal
├── include/
│   ├── genericsBase.h      # Biblioteca base para tipos genéricos
│   ├── generic_matrix.h    # Manipulação de matrizes com tipo genérico
│   ├── generic_vector.h    # TAD vetor dinâmico genérico (sort, pushback, len, etc.)
│   └── hash.h              # Hash table perfeita para coeficientes, colunas e variáveis
├── build.sh                # Script de compilação
├── in.txt                  # Exemplo de entrada
└── main                    # Binário compilado (gerado após build)
```

---

## 🛠️ Compilação

### Manual (gcc)

```bash
gcc -std=c11 src/main.c -Iinclude -o main
```

### Via script

```bash
bash build.sh
```

---

## ▶️ Uso

O programa lê o sistema linear pela entrada padrão (`stdin`). Para usar o arquivo de exemplo:

```bash
./main < in.txt
```

Ou, se o binário estiver em `src/`:

```bash
./src/main < in.txt
```

---

## 📥 Formato de Entrada

Cada linha representa uma equação do sistema. As variáveis podem ser:

- **Letras simples do alfabeto latino**: `a`, `b`, `c`, ..., `z`
- **Variáveis indexadas** de 0 a 99: `c0`, `c1`, `c11`, ..., `c99`

**Regras importantes:**

- As variáveis **não precisam aparecer na mesma ordem** em cada equação
- A cada nova equação, **novas variáveis podem surgir** ou não
- Os coeficientes são números reais

### Exemplo (`in.txt`)

```
2a + 3b - c = 5
a - b + 2c = 3
3a + b + c = 7
```

---

## 📤 Formato de Saída

O programa classifica e exibe a solução de acordo com o tipo do sistema:

**Sistema com solução única:**
```
a = 1.00
b = 2.00
c = 3.00
```

**Sistema com infinitas soluções:**
```
Infinitas soluções:
a = 2 - b
c = 1 + b
b = livre
```

**Sistema sem solução:**
```
Sistema insolúvel.
```

---

## 📚 Dependências (include/)

| Arquivo             | Descrição                                                              |
|---------------------|------------------------------------------------------------------------|
| `genericsBase.h`    | Biblioteca base que define os tipos genéricos usados pelo projeto      |
| `generic_matrix.h`  | Manipulação de matrizes com suporte a tipo genérico                    |
| `generic_vector.h`  | TAD vetor dinâmico genérico com métodos como `sort`, `pushback`, `len` |
| `hash.h`            | Hash table perfeita para armazenar coeficientes, colunas e variáveis  |

Todas as dependências são headers próprios do projeto — **não é necessário instalar bibliotecas externas**

---

## 👤 Autor

Desenvolvido por [glauc73](https://github.com/glauc73).
