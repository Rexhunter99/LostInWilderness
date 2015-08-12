#ifndef TINYJS_MATHFUNCTIONS_H
#define TINYJS_MATHFUNCTIONS_H

#include "TinyJS.h"

/// Register useful math. functions with the TinyJS interpreter
extern void DEPRECATED("is deprecated - MathFunctions now registered by default") registerMathFunctions(JS::Engine * tinyJS);

#endif
