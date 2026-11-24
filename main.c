#include "parser.tab.h"
#include "codegen_python.h"
#include "semantic.h"
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;
int yylex_destroy(void);

int main(int argc, char **argv) {
    int resultado;
    int gerar_python = 0;
    const char *arquivo = NULL;
    if (argc > 1 && strcmp(argv[1], "--python") == 0) {
        gerar_python = 1;
        if (argc == 3)
            arquivo = argv[2];
        else if (argc > 3) {
            fprintf(stderr, "Uso: %s [--python] [arquivo.traduzi]\n", argv[0]);
            return EXIT_FAILURE;
        }
    } else if (argc == 2) {
        arquivo = argv[1];
    } else if (argc > 2) {
        fprintf(stderr, "Uso: %s [--python] [arquivo.traduzi]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (arquivo) {
        yyin = fopen(arquivo, "rb");
        if (!yyin) { perror(arquivo); return EXIT_FAILURE; }
    }
    resultado = yyparse();
    if (arquivo) fclose(yyin);
    yylex_destroy();
    if (!resultado && !erros_sintaticos && !erros_lexicos) {
        if (!check_semantics(ast_raiz)) {
            ast_liberar(ast_raiz);
            ast_raiz = NULL;
            return EXIT_FAILURE;
        }
        if (gerar_python)
            generate_python(ast_raiz, stdout);
        else
            ast_imprimir(ast_raiz, stdout, 0);
    }
    ast_liberar(ast_raiz);
    ast_raiz = NULL;
    return resultado || erros_sintaticos || erros_lexicos ? EXIT_FAILURE : EXIT_SUCCESS;
}
