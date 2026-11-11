/* lexer_test.c - imprime os tokens reconhecidos (teste isolado do Flex).
 * Uso: ./lexer_test arquivo.trz */
#include <stdio.h>
#include "tokens_standalone.h"

YYSTYPE yylval;
extern FILE *yyin;
extern int yylex(void);
extern int lex_line, lex_col, lex_errors;

static const char *nome(int t)
{
    switch (t) {
    case TOK_INT: return "TOK_INT";           case TOK_FLOAT: return "TOK_FLOAT";
    case TOK_STRING: return "TOK_STRING";     case TOK_BOOL: return "TOK_BOOL";
    case TOK_VOID: return "TOK_VOID";         case TOK_TRUE: return "TOK_TRUE";
    case TOK_FALSE: return "TOK_FALSE";       case TOK_IN: return "TOK_IN";
    case TOK_CASO: return "TOK_CASO";         case TOK_DI: return "TOK_DI";
    case TOK_SUCCEDE: return "TOK_SUCCEDE";   case TOK_ALTRIMENTI: return "TOK_ALTRIMENTI";
    case TOK_MENTRE: return "TOK_MENTRE";     case TOK_PER: return "TOK_PER";
    case TOK_DE: return "TOK_DE";             case TOK_ATE: return "TOK_ATE";
    case TOK_FERMARSI: return "TOK_FERMARSI"; case TOK_CONTINUARE: return "TOK_CONTINUARE";
    case TOK_INPUT: return "TOK_INPUT";       case TOK_STAMPA: return "TOK_STAMPA";
    case TOK_ACAO: return "TOK_ACAO";         case TOK_RITORNA: return "TOK_RITORNA";
    case TOK_TRADUZI: return "TOK_TRADUZI";   case TOK_MUTABILE: return "TOK_MUTABILE";
    case TOK_INTERO: return "TOK_INTERO";     case TOK_REALE: return "TOK_REALE";
    case TOK_AND: return "TOK_AND";           case TOK_OR: return "TOK_OR";
    case TOK_NOT: return "TOK_NOT";
    case TOK_INT_LIT: return "TOK_INT_LIT";   case TOK_FLOAT_LIT: return "TOK_FLOAT_LIT";
    case TOK_TEXTO_LIT: return "TOK_TEXTO_LIT"; case TOK_IDENT: return "TOK_IDENT";
    case TOK_PLUS: return "TOK_PLUS";         case TOK_MINUS: return "TOK_MINUS";
    case TOK_STAR: return "TOK_STAR";         case TOK_SLASH: return "TOK_SLASH";
    case TOK_GT: return "TOK_GT";             case TOK_LT: return "TOK_LT";
    case TOK_GE: return "TOK_GE";             case TOK_LE: return "TOK_LE";
    case TOK_EQ: return "TOK_EQ";             case TOK_NE: return "TOK_NE";
    case TOK_ASSIGN: return "TOK_ASSIGN";
    case TOK_LPAREN: return "TOK_LPAREN";     case TOK_RPAREN: return "TOK_RPAREN";
    case TOK_LBRACKET: return "TOK_LBRACKET"; case TOK_RBRACKET: return "TOK_RBRACKET";
    case TOK_SEMI: return "TOK_SEMI";         case TOK_COMMA: return "TOK_COMMA";
    case TOK_COND_OPEN: return "TOK_COND_OPEN"; case TOK_COND_CLOSE: return "TOK_COND_CLOSE";
    default: return "?";
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "Uso: %s arquivo.trz\n", argv[0]); return 1; }
    yyin = fopen(argv[1], "r");
    if (!yyin) { perror(argv[1]); return 1; }

    int t;
    while ((t = yylex()) != 0) {
        printf("%3d:%-3d %-16s", lex_line, lex_col, nome(t));
        if (t == TOK_INT_LIT)        printf(" %d", yylval.ival);
        else if (t == TOK_FLOAT_LIT) printf(" %g", yylval.fval);
        else if (t == TOK_IDENT)     printf(" %s", yylval.sval);
        else if (t == TOK_TEXTO_LIT) printf(" \"%s\"", yylval.sval);
        printf("\n");
    }
    printf("\n%d erro(s) léxico(s).\n", lex_errors);
    fclose(yyin);
    return lex_errors ? 1 : 0;
}
