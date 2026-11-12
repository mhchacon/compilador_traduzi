#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "ast.h"

/* Retorna 1 se o programa for semanticamente valido, 0 caso contrario. */
int check_semantics(const AST *root);

#endif