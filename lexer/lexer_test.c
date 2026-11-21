/*
 * lexer/lexer_test.c - Teste isolado do analisador lexico (NAO faz parte do traduzi).
 *
 * Imprime um token por linha:  linha:coluna-linha:coluna  TOKEN  ["texto"]
 * Os erros lexicos saem em stderr (do proprio lexer). Retorna 0 se nao houve
 * erro lexico, 1 se houve e 2 se o arquivo nao pode ser aberto.
 *
 * Compilado por "make lexer_test", junto com parser.tab.c, ast.c e lex.yy.c.
 * Nao usa o main.c do executavel integrado (o executavel traduzi nao muda).
 *
 * Uso:  ./lexer_test arquivo.trz
 *       echo 'int x = 1;' | ./lexer_test        (le da entrada padrao)
 */
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>

extern FILE *yyin;
int yylex(void);
int yylex_destroy(void);

/* Nome do token, na mesma grafia do parser.y (caracteres simples saem entre aspas). */
static const char *nome(int t)
{
    switch (t) {
        case ACAO: return "ACAO";
        case TRADUZI: return "TRADUZI";
        case RITORNA: return "RITORNA";
        case VOID: return "VOID";
        case MUTABILE: return "MUTABILE";
        case INT: return "INT";
        case FLOAT: return "FLOAT";
        case STRING: return "STRING";
        case BOOL: return "BOOL";
        case TRUE: return "TRUE";
        case FALSE: return "FALSE";
        case IN: return "IN";
        case CASO: return "CASO";
        case DI: return "DI";
        case SUCCEDE: return "SUCCEDE";
        case ALTRIMENTI: return "ALTRIMENTI";
        case MENTRE: return "MENTRE";
        case PER: return "PER";
        case DE: return "DE";
        case ATE: return "ATE";
        case FERMARSI: return "FERMARSI";
        case CONTINUARE: return "CONTINUARE";
        case INPUT: return "INPUT";
        case STAMPA: return "STAMPA";
        case INTERO: return "INTERO";
        case REALE: return "REALE";
        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";
        case MAIOR_IGUAL: return "MAIOR_IGUAL";
        case MENOR_IGUAL: return "MENOR_IGUAL";
        case IGUAL: return "IGUAL";
        case DIFERENTE: return "DIFERENTE";
        case FIM_CONDICAO: return "FIM_CONDICAO";
        case INVALIDO: return "INVALIDO";
        case IDENT: return "IDENT";
        case INT_LIT: return "INT_LIT";
        case FLOAT_LIT: return "FLOAT_LIT";
        case TEXTO_LIT: return "TEXTO_LIT";
        default: return "?";
    }
}

int main(int argc, char **argv)
{
    int t;
    if (argc > 2) {
        fprintf(stderr, "Uso: %s [arquivo.trz]\n", argv[0]);
        return 2;
    }
    if (argc == 2) {
        yyin = fopen(argv[1], "rb");
        if (!yyin) { perror(argv[1]); return 2; }
    }

    /* stdout por linha: com "2>&1" os erros aparecem na ordem em que ocorrem,
     * entre os tokens (as saidas esperadas dos testes dependem disso). */
    setvbuf(stdout, NULL, _IOLBF, 0);

    while ((t = yylex()) != 0) {
        printf("%3d:%-3d-%3d:%-3d ", yylloc.first_line, yylloc.first_column,
               yylloc.last_line, yylloc.last_column);
        if (t < 256) {
            printf(" '%c'\n", t);
            continue;
        }
        printf(" %-12s", nome(t));
        if (t == IDENT || t == INT_LIT || t == FLOAT_LIT || t == TEXTO_LIT) {
            printf(" \"%s\"", yylval.texto);
            free(yylval.texto);   /* o lexer entrega a memoria; aqui quem consome libera */
        }
        printf("\n");
    }
    printf("%3d:%-3d       EOF\n", yylloc.first_line, yylloc.first_column);
    if (argc == 2) fclose(yyin);
    yylex_destroy();
    if (erros_lexicos > 0) {
        printf("\n%d erro(s) lexico(s).\n", erros_lexicos);
        return 1;
    }
    return 0;
}
