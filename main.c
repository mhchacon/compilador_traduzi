#include "parser.tab.h"
#include <stdlib.h>

extern FILE *yyin;
int yylex_destroy(void);

int main(int argc, char **argv) {
    int resultado;
    if (argc > 2) {
        fprintf(stderr, "Uso: %s [arquivo.traduzi]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        yyin = fopen(argv[1], "rb");
        if (!yyin) { perror(argv[1]); return EXIT_FAILURE; }
    }
    resultado = yyparse();
    if (argc == 2) fclose(yyin);
    yylex_destroy();
    if (!resultado && !erros_sintaticos && !erros_lexicos) ast_imprimir(ast_raiz, stdout, 0);
    ast_liberar(ast_raiz);
    ast_raiz = NULL;
    return resultado || erros_sintaticos || erros_lexicos ? EXIT_FAILURE : EXIT_SUCCESS;
}
