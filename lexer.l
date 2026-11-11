/* ============================================================
 *  lexer.l  -  Analisador léxico da linguagem Traduzi (Flex)
 *
 *  Gerar:  flex -o lex.yy.c lexer.l
 *
 *  Modo integrado:  inclui "parser.tab.h" (gerado pelo Bison).
 *  Modo isolado  :  compile com -DLEXER_STANDALONE para testar o
 *                   Flex sozinho (usa teste_lexer/tokens_standalone.h).
 * ============================================================ */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef LEXER_STANDALONE
#include "tokens_standalone.h"
#else
#include "parser.tab.h"
#endif

/* ---- Informações de posição (usadas pelo Bison e nas mensagens) ---- */
int lex_line   = 1;   /* linha do último token lido                    */
int lex_col    = 1;   /* coluna inicial do último token lido           */
int lex_errors = 0;   /* quantidade de erros léxicos encontrados       */

static int col = 1;            /* coluna corrente (próximo caractere)  */
static int last_tok = 0;       /* último token devolvido ao Bison      */
static int in_cond = 0;        /* 1 = dentro de uma condição -...-     */
static int com_line = 0;       /* onde começou o comentário de bloco   */
static int com_col  = 0;

/* Atualiza linha/coluna antes de executar cada ação */
#define YY_USER_ACTION                                   \
    {                                                    \
        lex_line = yylineno;                             \
        lex_col  = col;                                  \
        for (int _i = 0; _i < yyleng; _i++) {            \
            if (yytext[_i] == '\n') col = 1; else col++; \
        }                                                \
    }

/* Devolve o token ao Bison guardando qual foi o último */
#define RET(t) do { last_tok = (t); return (t); } while (0)

static void lex_error_at(int line, int column, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "Erro (Léxica) linha %d, coluna %d: ", line, column);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    lex_errors++;
}

#define LEX_ERROR(...) lex_error_at(lex_line, lex_col, __VA_ARGS__)
%}

%option noyywrap
%option yylineno
%option nounput
%option noinput

%x COMENTARIO

LETRA    [A-Za-z]
DIGITO   [0-9]
IDCHAR   [A-Za-z0-9_]
WS       [ \t\r\n]

%%

 /* ---------------- Espaços em branco ---------------- */
[ \t\r\n]+                      { /* ignora */ }

 /* ---------------- Comentários ---------------- */
"~>".*                          { /* comentário de uma linha */ }

"~{"                            { com_line = lex_line; com_col = lex_col; BEGIN(COMENTARIO); }
<COMENTARIO>"}~"                { BEGIN(INITIAL); }
<COMENTARIO>[^}\n]+             { /* ignora conteúdo */ }
<COMENTARIO>"}"                 { /* ignora */ }
<COMENTARIO>\n                  { /* ignora */ }
<COMENTARIO><<EOF>>             {
                                    lex_error_at(com_line, com_col,
                                        "comentário de várias linhas não foi fechado (esperado '}~').");
                                    BEGIN(INITIAL);
                                    yyterminate();
                                }
"}~"                            { LEX_ERROR("'}~' sem um '~{' correspondente."); }

 /* ---------------- Palavras reservadas ---------------- */
 /* Tipos */
"int"                           { RET(TOK_INT); }
"float"                         { RET(TOK_FLOAT); }
"string"                        { RET(TOK_STRING); }
"bool"                          { RET(TOK_BOOL); }
"void"                          { RET(TOK_VOID); }

 /* Valores booleanos */
"true"                          { RET(TOK_TRUE); }
"false"                         { RET(TOK_FALSE); }

 /* Condicional: in caso di ... succede ... altrimenti */
"in"                            { RET(TOK_IN); }
"caso"                          { RET(TOK_CASO); }
"di"                            { RET(TOK_DI); }
"succede"                       { RET(TOK_SUCCEDE); }
"altrimenti"                    { RET(TOK_ALTRIMENTI); }

 /* Repetição */
"mentre"                        { RET(TOK_MENTRE); }
"per"                           { RET(TOK_PER); }
"de"                            { RET(TOK_DE); }
"ate"                           { RET(TOK_ATE); }
"fermarsi"                      { RET(TOK_FERMARSI); }
"continuare"                    { RET(TOK_CONTINUARE); }

 /* Entrada e saída */
"input"                         { RET(TOK_INPUT); }
"stampa"                        { RET(TOK_STAMPA); }

 /* Ações */
"acao"                          { RET(TOK_ACAO); }
"ritorna"                       { RET(TOK_RITORNA); }
"Traduzi"                       { RET(TOK_TRADUZI); }

 /* Variáveis e conversões */
"mutabile"                      { RET(TOK_MUTABILE); }
"intero"                        { RET(TOK_INTERO); }
"reale"                         { RET(TOK_REALE); }

 /* Operadores lógicos */
"and"                           { RET(TOK_AND); }
"or"                            { RET(TOK_OR); }
"not"                           { RET(TOK_NOT); }

 /* ---------------- Números ---------------- */
{DIGITO}+"."{DIGITO}+           { yylval.fval = atof(yytext); RET(TOK_FLOAT_LIT); }

{DIGITO}+                       { yylval.ival = atoi(yytext); RET(TOK_INT_LIT); }

 /* Formas inválidas: "5." e ".5" */
{DIGITO}+"."                    {
                                    LEX_ERROR("número real inválido '%s': faltam dígitos depois do ponto.", yytext);
                                    yylval.fval = atof(yytext); RET(TOK_FLOAT_LIT);
                                }
"."{DIGITO}+                    {
                                    LEX_ERROR("número real inválido '%s': faltam dígitos antes do ponto.", yytext);
                                    yylval.fval = atof(yytext); RET(TOK_FLOAT_LIT);
                                }

 /* Identificador começando por número, ex.: 2idade */
{DIGITO}+{IDCHAR}*              {
                                    LEX_ERROR("Identificador não pode começar com número ('%s').", yytext);
                                    yylval.sval = strdup(yytext); RET(TOK_IDENT);
                                }

 /* ---------------- Identificadores ---------------- */
({LETRA}|_){IDCHAR}*            { yylval.sval = strdup(yytext); RET(TOK_IDENT); }

 /* ---------------- Textos: :conteudo: ---------------- */
:[^:\n]*:                       {
                                    /* remove os dois-pontos das pontas */
                                    yytext[yyleng - 1] = '\0';
                                    yylval.sval = strdup(yytext + 1);
                                    RET(TOK_TEXTO_LIT);
                                }
:[^:\n]*                        {
                                    LEX_ERROR("texto sem delimitador de fechamento ':'.");
                                    yylval.sval = strdup(yytext + 1);
                                    RET(TOK_TEXTO_LIT);
                                }

 /* ---------------- Hífen (regra do hífen, seção 3 da especificação) ----------------
  *  1) '-' logo depois de 'di' ou 'mentre'      -> abre a condição
  *  2) '-' logo antes de 'succede' ou de '['    -> fecha a condição
  *  3) qualquer outro '-'                       -> operador aritmético
  */
"-"/{WS}*("succede"[^A-Za-z0-9_]|"[")  {
                                    if (in_cond) { in_cond = 0; RET(TOK_COND_CLOSE); }
                                    RET(TOK_MINUS);
                                }
"-"                             {
                                    if (last_tok == TOK_DI || last_tok == TOK_MENTRE) {
                                        in_cond = 1;
                                        RET(TOK_COND_OPEN);
                                    }
                                    RET(TOK_MINUS);
                                }

 /* ---------------- Operadores ---------------- */
">="                            { RET(TOK_GE); }
"<="                            { RET(TOK_LE); }
"=="                            { RET(TOK_EQ); }
"!="                            { RET(TOK_NE); }
">"                             { RET(TOK_GT); }
"<"                             { RET(TOK_LT); }
"="                             { RET(TOK_ASSIGN); }
"+"                             { RET(TOK_PLUS); }
"*"                             { RET(TOK_STAR); }
"/"                             { RET(TOK_SLASH); }

 /* ---------------- Delimitadores ---------------- */
"("                             { RET(TOK_LPAREN); }
")"                             { RET(TOK_RPAREN); }
"["                             { RET(TOK_LBRACKET); }
"]"                             { RET(TOK_RBRACKET); }
";"                             { RET(TOK_SEMI); }
","                             { RET(TOK_COMMA); }

 /* ---------------- Erros ---------------- */
[\x80-\xFF]+                    {
                                    LEX_ERROR("caractere não permitido fora de textos ('%s'). "
                                              "Identificadores aceitam apenas letras sem acento, números e '_'.", yytext);
                                }
.                               { LEX_ERROR("caractere inválido '%s'.", yytext); }

%%
