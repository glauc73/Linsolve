#include "genericsBase.h"

#ifndef GENERICS_TYPE
#define GENERICS_TYPE int
#endif

#ifndef GENERICS_NAME
#define GENERICS_NAME int
#endif

#ifndef GROWN_STACK
#define GROWN_STACK 1.5
#endif

#define DEFAULT_SIZE_STACK 256
#define STACK_TYPE CONCAT(stack_, GENERICS_NAME)

typedef struct STACK_TYPE{
    int index_top;
    GENERICS_TYPE *data;
    size_t size;
    
    void (*pop)(struct STACK_TYPE*);
    void (*push)(struct STACK_TYPE*, GENERICS_TYPE);
    int (*empty)(struct STACK_TYPE*);
    GENERICS_TYPE (*top)(struct STACK_TYPE*);
    void (*free)(struct STACK_TYPE**);
}STACK_TYPE;

#if !defined(__INTELLISENSE__) && !defined(__clangd__)

static inline void CONCAT(STACK_TYPE, _push)(STACK_TYPE *stk, GENERICS_TYPE value){
    if(stk->index_top >= stk->size - 1){
        size_t resize = GROWN_STACK * (stk->size + 1); 
        GENERICS_TYPE* temp = realloc(stk->data, resize * sizeof(GENERICS_TYPE));
        if(temp) {
            stk->size = resize;
            stk->data = temp;
        } else {
            lowMemory();            
            return;
        }
    }
    stk->data[++stk->index_top] = value;
}

static inline int CONCAT(STACK_TYPE, _empty)(STACK_TYPE* stk){
    return (stk->index_top == -1);
}

static inline void CONCAT(STACK_TYPE, _pop)(STACK_TYPE* stk){
    if(stk->empty(stk)){
        printf("Empty Stack!\n");
        return;
    }
    stk->index_top--;
}

static inline GENERICS_TYPE CONCAT(STACK_TYPE, _top)(STACK_TYPE* stk){
    if(stk->empty(stk)){
        printf("Empty Stack!\n");
        exit(EXIT_FAILURE);
    }
    return stk->data[stk->index_top];
}

static inline void CONCAT(delete_, STACK_TYPE)(STACK_TYPE** stk){
    if(stk == NULL || *stk == NULL) return;

    free((*stk)->data);
    (*stk)->data = NULL;

    free(*stk);
    *stk = NULL;
}

#endif

static inline STACK_TYPE* CONCAT(new_, STACK_TYPE)(void){
    STACK_TYPE* stk = malloc(sizeof(STACK_TYPE));
    if(!stk) {
        lowMemory();
        return NULL;
    }

    stk->data = malloc(DEFAULT_SIZE_STACK * sizeof(GENERICS_TYPE));

    if(!stk->data){
        lowMemory();
        free(stk);
        return NULL;
    }
    stk->index_top = -1;
    stk->size = DEFAULT_SIZE_STACK; 

    stk->push = CONCAT(STACK_TYPE, _push);
    stk->empty = CONCAT(STACK_TYPE, _empty);
    stk->top = CONCAT(STACK_TYPE, _top);
    stk->pop = CONCAT(STACK_TYPE, _pop);
    stk->free = CONCAT(delete_, STACK_TYPE);

    return stk;
}