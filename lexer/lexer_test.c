/*
 * lexer/lexer_test.c - Teste isolado do analisador lexico (NAO faz parte do traduzi).
 *
 * Imprime um token por linha:  linha:coluna-fim  TOKEN  [texto]
 * Retorna 1 se houve erro lexico. Compilado por "make lexer_test", junto com
 * parser.tab.c, ast.c e lex.yy.c (nao usa o main.c do executavel integrado).
 *
 * Uso: ./lexer_test arquivo.trz
 */
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>

extern FILE *yyin;
int yylex(void);
int yylex_destroy(void);

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
    if (argc != 2) {
        fprintf(stderr, "Uso: %s arquivo.trz\n", argv[0]);
        return 2;
    }
    yyin = fopen(argv[1], "rb");
    if (!yyin) { perror(argv[1]); return 2; }

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
            free(yylval.texto);
        }
        printf("\n");
    }
    printf("%3d:%-3d       EOF\n", yylloc.first_line, yylloc.first_column);
    fclose(yyin);
    yylex_destroy();
    if (erros_lexicos > 0) {
        printf("\n%d erro(s) lexico(s).\n", erros_lexicos);
        return 1;
    }
    return 0;
}
