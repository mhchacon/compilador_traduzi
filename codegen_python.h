#ifndef CODEGEN_PYTHON_H
#define CODEGEN_PYTHON_H

#include "ast.h"
#include <stdio.h>

void generate_python(const AST *root, FILE *output);

#endif
