/* ============================================================
 *  lexer/lexer.l  -  Analisador lexico da linguagem Traduzi (Flex)
 *
 *  Segue o contrato descrito em lexer/README.md e os tokens declarados
 *  em parser.y. Gerado pelo Makefile:  flex -o lex.yy.c lexer/lexer.l
 * ============================================================ */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.tab.h"   /* tokens, yylval e yylloc gerados pelo Bison (inclui ast.h) */

int erros_lexicos = 0;    /* declarado como extern em ast.h */

/* Posicao do proximo caractere a ser lido (contagem a partir de 1).
 * Colunas contam caracteres: bytes de continuacao UTF-8 nao contam. */
static int linha = 1, coluna = 1;
static int com_linha = 0, com_coluna = 0;   /* onde comecou um comentario de bloco */

/* Executado antes de toda acao: preenche yylloc com o inicio e o fim do lexema
 * (last_column e a coluna logo apos o ultimo caractere). */
#define YY_USER_ACTION atualizar_posicao();

static void atualizar_posicao(void)
{
    int i;
    yylloc.first_line = linha;
    yylloc.first_column = coluna;
    for (i = 0; i < yyleng; i++) {
        unsigned char c = (unsigned char)yytext[i];
        if (c == '\n') { linha++; coluna = 1; }
        else if ((c & 0xC0) != 0x80) coluna++;
    }
    yylloc.last_line = linha;
    yylloc.last_column = coluna;
}

static char *copiar(const char *s, size_t n)
{
    char *p = malloc(n + 1);
    if (!p) { fputs("Memoria insuficiente.\n", stderr); exit(EXIT_FAILURE); }
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

/* Formato esperado pelos testes: Erro lexico linha L, coluna C: mensagem */
static void erro_lexico(int l, int c, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "Erro lexico linha %d, coluna %d: ", l, c);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    erros_lexicos++;
}

/* Registra o erro no lexema atual e devolve INVALIDO ao parser. */
#define INVALIDO_COM(...) \
    do { erro_lexico(yylloc.first_line, yylloc.first_column, __VA_ARGS__); return INVALIDO; } while (0)

static void posicao_fim_de_arquivo(void)
{
    yylloc.first_line = yylloc.last_line = linha;
    yylloc.first_column = yylloc.last_column = coluna;
}
%}

%option noyywrap never-interactive nounput noinput

%x COMENTARIO

DIGITO    [0-9]
IDCHAR    [A-Za-z0-9_]
LETRA_    [A-Za-z_]
WS        [ \t\r\n]

/* Comentarios (usados tambem para olhar adiante na regra do hifen) */
COM_LINHA   "~>"[^\n]*\n
COM_BLOCO   "~{"([^}]|"}"+[^}~])*"}"+"~"

/* O que pode aparecer depois do '-' que FECHA uma condicao: espacos e
 * comentarios, e entao 'succede' (condicional) ou '[' (mentre). */
FECHA_DEPOIS  ({WS}|{COM_LINHA}|{COM_BLOCO})*("succede"[^A-Za-z0-9_]|"[")

%%

 /* ---------------- BOM UTF-8 e espacos em branco ---------------- */
"\xEF\xBB\xBF"                  { coluna = 1; }
{WS}+                           { /* ignora */ }

 /* ---------------- Comentarios ---------------- */
"~>".*                          { /* comentario de uma linha */ }
"~{"                            { com_linha = yylloc.first_line; com_coluna = yylloc.first_column; BEGIN(COMENTARIO); }
<COMENTARIO>"}~"                { BEGIN(INITIAL); }
<COMENTARIO>[^}\n]+             { /* conteudo do comentario */ }
<COMENTARIO>"}"|\n              { /* conteudo do comentario */ }
<COMENTARIO><<EOF>>             {
                                    BEGIN(INITIAL);
                                    yylloc.first_line = com_linha;
                                    yylloc.first_column = com_coluna;
                                    yylloc.last_line = linha;
                                    yylloc.last_column = coluna;
                                    erro_lexico(com_linha, com_coluna,
                                        "comentario de varias linhas nao foi fechado (esperado '}~').");
                                    return INVALIDO;
                                }
"}~"                            { INVALIDO_COM("'}~' sem um '~{' correspondente."); }

 /* ---------------- Palavras reservadas ---------------- */
"int"                           { return INT; }
"float"                         { return FLOAT; }
"string"                        { return STRING; }
"bool"                          { return BOOL; }
"void"                          { return VOID; }
"true"                          { return TRUE; }
"false"                         { return FALSE; }
"in"                            { return IN; }
"caso"                          { return CASO; }
"di"                            { return DI; }
"succede"                       { return SUCCEDE; }
"altrimenti"                    { return ALTRIMENTI; }
"mentre"                        { return MENTRE; }
"per"                           { return PER; }
"de"                            { return DE; }
"ate"                           { return ATE; }
"fermarsi"                      { return FERMARSI; }
"continuare"                    { return CONTINUARE; }
"input"                         { return INPUT; }
"stampa"                        { return STAMPA; }
"acao"                          { return ACAO; }
"ritorna"                       { return RITORNA; }
"Traduzi"                       { return TRADUZI; }
"mutabile"                      { return MUTABILE; }
"intero"                        { return INTERO; }
"reale"                         { return REALE; }
"and"                           { return AND; }
"or"                            { return OR; }
"not"                           { return NOT; }

 /* ---------------- Numeros (o texto original e preservado) ---------------- */
{DIGITO}+"."{DIGITO}+           { yylval.texto = copiar(yytext, (size_t)yyleng); return FLOAT_LIT; }
{DIGITO}+                       { yylval.texto = copiar(yytext, (size_t)yyleng); return INT_LIT; }

{DIGITO}+"."                    { INVALIDO_COM("numero real invalido '%s': faltam digitos depois do ponto.", yytext); }
"."{DIGITO}+                    { INVALIDO_COM("numero real invalido '%s': faltam digitos antes do ponto.", yytext); }
{DIGITO}+"."{DIGITO}+{LETRA_}{IDCHAR}*  { INVALIDO_COM("numero real mal formado '%s'.", yytext); }
{DIGITO}+{LETRA_}{IDCHAR}*      { INVALIDO_COM("identificador nao pode comecar com numero ('%s').", yytext); }

 /* ---------------- Identificadores ---------------- */
{LETRA_}{IDCHAR}*               { yylval.texto = copiar(yytext, (size_t)yyleng); return IDENT; }

 /* ---------------- Textos: :conteudo: (sem os dois-pontos) ---------------- */
":"[^:\n]*":"                   { yylval.texto = copiar(yytext + 1, (size_t)yyleng - 2); return TEXTO_LIT; }
":"[^:\n]*                      { INVALIDO_COM("texto sem delimitador de fechamento ':' na mesma linha."); }

 /* ---------------- Hifen ----------------
  * O '-' que fecha a condicao (logo antes de 'succede' ou '[', ignorando
  * espacos e comentarios) devolve FIM_CONDICAO. Todos os outros '-' devolvem '-'. */
"-"/{FECHA_DEPOIS}              { return FIM_CONDICAO; }
"-"                             { return '-'; }

 /* ---------------- Operadores e delimitadores ---------------- */
">="                            { return MAIOR_IGUAL; }
"<="                            { return MENOR_IGUAL; }
"=="                            { return IGUAL; }
"!="                            { return DIFERENTE; }
[+*/<>=()\[\],;]                { return yytext[0]; }

 /* ---------------- Erros lexicos ---------------- */
[\x80-\xFF]+                    { INVALIDO_COM("caractere nao permitido fora de textos ('%s'). Identificadores aceitam apenas letras sem acento, numeros e '_'.", yytext); }
"!"                             { INVALIDO_COM("caractere '!' inesperado. Use 'not' para negacao ou '!=' para diferente."); }
"&"                             { INVALIDO_COM("caractere '&' invalido. Use 'and'."); }
"|"                             { INVALIDO_COM("caractere '|' invalido. Use 'or'."); }
["']                            { INVALIDO_COM("caractere %c invalido. Textos usam dois-pontos, como em :texto:.", yytext[0]); }
[{}]                            { INVALIDO_COM("caractere '%c' invalido. Blocos usam '[' e ']'.", yytext[0]); }
"~"                             { INVALIDO_COM("simbolo '~' isolado. Comentarios usam '~>' (uma linha) ou '~{ ... }~' (varias linhas)."); }
.                               { INVALIDO_COM("caractere invalido '%s'.", yytext); }

<INITIAL><<EOF>>                { posicao_fim_de_arquivo(); yyterminate(); }

%%
