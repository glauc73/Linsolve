#include "genericsBase.h"
#include <string.h>

#ifndef GENERICS_TYPE
#define GENERICS_TYPE int
#endif

#ifndef GENERICS_NAME
#define GENERICS_NAME int
#endif

#ifndef GROWUP_VECTOR
#define GROWUP_VECTOR 2
#endif

#define MIN_SIZE_VECTOR 64
#define VECTOR_TYPE CONCAT(vector_, GENERICS_NAME)
#define NEW_VECTOR CONCAT(new_, VECTOR_TYPE)

typedef struct VECTOR_TYPE{
    GENERICS_TYPE *data;
    size_t len;
    size_t capacity;
    void (*free)(struct VECTOR_TYPE**);
    int (*empty)(struct VECTOR_TYPE*);
    void (*push_back)(struct VECTOR_TYPE*, GENERICS_TYPE);
    void (*insert)(struct VECTOR_TYPE*, size_t, GENERICS_TYPE);
    void (*erase)(struct VECTOR_TYPE*, size_t);
    void (*clear)(struct VECTOR_TYPE*);
    void (*sort)(struct VECTOR_TYPE*, int (*)(const void*, const void*));
}VECTOR_TYPE;

VECTOR_TYPE* NEW_VECTOR(size_t n_size);
static inline void CONCAT(merge_, VECTOR_TYPE)(VECTOR_TYPE* vet, size_t start, size_t middle, size_t end, comp_function cmp);

#if !defined(__INTELLISENSE__) && !defined(__clangd__)


static inline void CONCAT(delete_, VECTOR_TYPE)(VECTOR_TYPE** vet){
    if(!vet || !*vet) return;
    free((*vet)->data);
    free(*vet);
    *vet = NULL;
}

static inline void CONCAT(pushback_, VECTOR_TYPE)(VECTOR_TYPE* vet, GENERICS_TYPE value){
    if(vet->len + 1 >= vet->capacity){
        GENERICS_TYPE* temp = (GENERICS_TYPE*)realloc(vet->data, GROWUP_VECTOR*vet->capacity*sizeof(GENERICS_TYPE));
        if(!temp){
            printf("Memoria insuficiente\n");
            exit(EXIT_FAILURE);
        }
        vet->data = temp;
        vet->capacity *= GROWUP_VECTOR;
    }
    vet->data[vet->len++] = value;
}

static inline int CONCAT(empty_, VECTOR_TYPE)(VECTOR_TYPE* vet){
    return (vet->len == 0);
}

static inline void CONCAT(clear_, VECTOR_TYPE)(VECTOR_TYPE* vet){
    vet->len = 0;
}

void CONCAT(insert_, VECTOR_TYPE)(VECTOR_TYPE* vet, size_t index, GENERICS_TYPE value){
    if(index >= vet->capacity) return;

    if(vet->len + 1 >= vet->capacity){
        GENERICS_TYPE* temp = (GENERICS_TYPE*)realloc(vet->data, GROWUP_VECTOR*vet->capacity*sizeof(GENERICS_TYPE));
        if(!temp){
            printf("Memoria insuficiente\n");
            exit(EXIT_FAILURE);
        }
        vet->data = temp;
        vet->capacity *= GROWUP_VECTOR;
    }
    GENERICS_TYPE* buffer = vet->data + index;
    size_t n = vet->len - index;

    if(n > 0) {
        memmove(buffer + 1, buffer, n * sizeof(GENERICS_TYPE));
    }

    vet->data[index] = value;
    vet->len++;
}

void CONCAT(erase_, VECTOR_TYPE)(VECTOR_TYPE* vet, size_t index){
    if(index >= vet->capacity) return;

    GENERICS_TYPE* buffer = vet->data + index + 1;
    size_t n = vet->len - index - 1;
    if(n > 0){
        memmove(buffer, buffer + 1, n * sizeof(GENERICS_TYPE));
    }
    vet->len--;
}

static inline void CONCAT(merge_, VECTOR_TYPE)(
        VECTOR_TYPE* vet,  
        size_t start, 
        size_t middle, 
        size_t end, 
        int (*cmp)(const void*, const void*)
    ){

    size_t n1 = middle - start + 1;
    size_t n2 = end - middle;
    size_t i = 0, j = 0, k = start;

    GENERICS_TYPE* left = (GENERICS_TYPE*)malloc(n1 * sizeof(GENERICS_TYPE));
    GENERICS_TYPE* right = (GENERICS_TYPE*)malloc(n2 * sizeof(GENERICS_TYPE));

    for(i = 0; i < n1; i++) left[i] = vet->data[start + i];
    for(j = 0; j < n2; j++) right[j] = vet->data[middle + 1 + j];

    i = 0, j = 0;
    while(i < n1 && j < n2){
        if(cmp){
            if(cmp(&left[i], &right[j]))
                vet->data[k++] = left[i++];
            else
                vet->data[k++] = right[j++];
        }else{
            if(left[i] <= right[j])
                vet->data[k++] = left[i++];
            else 
                vet->data[k++] = right[j++];
        }
    }

    while(i < n1) vet->data[k++] = left[i++];
    while(j < n2) vet->data[k++] = right[j++];
    
    free(left);
    free(right);
}

static inline void CONCAT(mergesort_, VECTOR_TYPE)(VECTOR_TYPE* vet, size_t start, size_t end, int (*cmp)(const void*, const void*)){
    if(start < end){
        size_t middle = (start + end)/2;
        CONCAT(mergesort_, VECTOR_TYPE)(vet, start, middle, cmp);
        CONCAT(mergesort_, VECTOR_TYPE)(vet, middle + 1, end, cmp);
        CONCAT(merge_, VECTOR_TYPE)(vet, start, middle, end, cmp);
    }
}

static inline void CONCAT(sort_, VECTOR_TYPE)(VECTOR_TYPE* vet, int (*cmp)(const void*, const void*)){
    if(vet->len > 1)
    CONCAT(mergesort_, VECTOR_TYPE)(vet, 0, vet->len - 1, cmp);
}

#endif


VECTOR_TYPE* NEW_VECTOR(size_t n_size){
    n_size = (n_size < MIN_SIZE_VECTOR) ? MIN_SIZE_VECTOR : n_size;

    VECTOR_TYPE* vet = (VECTOR_TYPE*)malloc(sizeof(*vet));
    vet->data = (GENERICS_TYPE*) malloc(n_size * sizeof(GENERICS_TYPE));
    if(!vet->data){
        printf("Memoria insuficiente!\n");
        return NULL;
    }

    vet->len = 0;
    vet->capacity = n_size;
    vet->free = CONCAT(delete_, VECTOR_TYPE);
    vet->push_back = CONCAT(pushback_, VECTOR_TYPE);
    vet->insert = CONCAT(insert_, VECTOR_TYPE);
    vet->erase = CONCAT(erase_, VECTOR_TYPE);
    vet->empty = CONCAT(empty_, VECTOR_TYPE);
    vet->clear = CONCAT(clear_, VECTOR_TYPE);
    vet->sort = CONCAT(sort_, VECTOR_TYPE);
    return vet;
}

