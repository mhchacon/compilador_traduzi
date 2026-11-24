/* Executavel de teste: usa o Flex real e a AST, sem validacao semantica. */
#include "parser.tab.h"
#include <stdlib.h>

int yylex_destroy(void);

int main(void)
{
    int resultado = yyparse();
    int falhou = resultado || erros_sintaticos || erros_lexicos;
    if (!falhou) ast_imprimir(ast_raiz, stdout, 0);
    ast_liberar(ast_raiz);
    ast_raiz = NULL;
    yylex_destroy();
    return falhou ? EXIT_FAILURE : EXIT_SUCCESS;
}
