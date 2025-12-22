#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdio.h>

#ifndef _VECTOR_INT_
    #define _VECTOR_INT_
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME int
    #define GENERICS_TYPE int
    #include "genericVector.h"
#endif

#ifndef _VECTOR_CHAR_
    #define _VECTOR_CHAR_
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME char
    #define GENERICS_TYPE char
    #include "genericVector.h"
#endif

#ifndef _VECTOR_UINT_
    #define _VECTOR_UINT_
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME uint
    #define GENERICS_TYPE unsigned
    #include "genericVector.h"
#endif

#ifndef _VECTOR_FLOAT_
    #define _VECTOR_FLOAT_
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME float
    #define GENERICS_TYPE float    
    #include "genericVector.h"
#endif

#ifndef _VECTOR_DOUBLE_
    #define _VECTOR_DOUBLE_   
    #undef GENERICS_NAME
    #undef GENERICS_TYPE
    #define GENERICS_NAME double
    #define GENERICS_TYPE double
    #include "genericVector.h"
#endif

#undef VECTOR_TYPE

#endif