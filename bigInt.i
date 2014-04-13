%module bigInt
%{
/* Includes the header in the wrapper code */
#include "bigInt.h"
%}

/* Parse the header file to generate wrappers */
%include "bigInt.h"
