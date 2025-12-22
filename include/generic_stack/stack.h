#ifndef _STACK_H_
#define _STACK_H_

#include <stdio.h>

static inline void lowMemory(){
    printf("Memória insuficiente!\n");
}

#ifndef _STACK_INT_
    #define _STACK_INT_
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME int
    #define GENERICS_TYPE int
    #include "genericStack.h"
#endif

#ifndef _STACK_CHAR_
    #define _STACK_CHAR_
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME char
    #define GENERICS_TYPE char
    #include "genericStack.h"
#endif

#ifndef _STACK_UINT_
    #define _STACK_UINT_
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME uint
    #define GENERICS_TYPE unsigned
    #include "genericStack.h"
#endif

#ifndef _STACK_FLOAT_
    #define _STACK_FLOAT_
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME float
    #define GENERICS_TYPE float    
    #include "genericStack.h"
#endif

#ifndef _STACK_DOUBLE_
    #define _STACK_DOUBLE_   
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME double
    #define GENERICS_TYPE double
    #include "genericStack.h"
#endif

#undef STACK_TYPE

#endif