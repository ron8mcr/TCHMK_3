%module bigInt
%{
/* Includes the header in the wrapper code */
#include "bigInt.h"
%}

%rename(_in) operator >>;
%rename(_out) operator <<;

/* Parse the header file to generate wrappers */
%include "bigInt.h"
