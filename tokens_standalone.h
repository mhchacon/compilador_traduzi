/* tokens_standalone.h
 * Usado SOMENTE para testar o Flex sem o Bison (-DLEXER_STANDALONE).
 * No projeto integrado, estes tokens vêm de parser.tab.h (gerado pelo Bison). */
#ifndef TOKENS_STANDALONE_H
#define TOKENS_STANDALONE_H

enum yytokentype {
    TOK_INT = 258, TOK_FLOAT, TOK_STRING, TOK_BOOL, TOK_VOID,
    TOK_TRUE, TOK_FALSE,
    TOK_IN, TOK_CASO, TOK_DI, TOK_SUCCEDE, TOK_ALTRIMENTI,
    TOK_MENTRE, TOK_PER, TOK_DE, TOK_ATE, TOK_FERMARSI, TOK_CONTINUARE,
    TOK_INPUT, TOK_STAMPA,
    TOK_ACAO, TOK_RITORNA, TOK_TRADUZI,
    TOK_MUTABILE, TOK_INTERO, TOK_REALE,
    TOK_AND, TOK_OR, TOK_NOT,
    TOK_INT_LIT, TOK_FLOAT_LIT, TOK_TEXTO_LIT, TOK_IDENT,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH,
    TOK_GT, TOK_LT, TOK_GE, TOK_LE, TOK_EQ, TOK_NE, TOK_ASSIGN,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACKET, TOK_RBRACKET, TOK_SEMI, TOK_COMMA,
    TOK_COND_OPEN, TOK_COND_CLOSE
};

typedef union {
    int    ival;
    double fval;
    char  *sval;
} YYSTYPE;

extern YYSTYPE yylval;

#endif
