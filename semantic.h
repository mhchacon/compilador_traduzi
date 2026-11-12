#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "ast.h"

// Retorna 1 se o programa for válido, 0 se houver erro semântico
int check_semantics(ASTNode* root);

#endif