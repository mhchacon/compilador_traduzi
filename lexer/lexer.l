/* ====================================================================
 *  lexer/lexer.l  -  Analisador lexico da linguagem Traduzi (Flex)
 *
 *  O que faz:
 *    Le o texto do programa e o transforma em uma sequencia de tokens,
 *    que o Bison (parser.y) consome. Tambem ignora espacos e comentarios
 *    e avisa quando encontra algo que nao pertence a linguagem.
 *
 *  Como e gerado (ver Makefile):
 *    flex -o lex.yy.c lexer/lexer.l      (lex.yy.c NAO vai para o Git)
 *
 *  Organizacao deste arquivo (3 partes separadas por %%, como todo .l):
 *    1. Definicoes  : codigo C auxiliar, opcoes do Flex e nomes de padroes
 *    2. Regras      : "padrao  { acao }", na ordem em que sao testadas
 *    3. (vazia)     : o main fica em main.c; o lexer nao define main
 *
 *  Duas regras do Flex que explicam a ordem das regras abaixo:
 *    - vence o MAIOR casamento: ">=" e um token so, "international" e um
 *      identificador e nao "int" + "ernational";
 *    - em empate, vence a regra que aparece PRIMEIRO: por isso as palavras
 *      reservadas vem antes da regra de identificador.
 *
 *  Contrato com o Bison (detalhes em lexer/README.md):
 *    - nomes dos tokens: os declarados em parser.y (parser.tab.h)
 *    - IDENT, INT_LIT, FLOAT_LIT, TEXTO_LIT: yylval.texto recebe uma copia
 *      alocada com malloc; o PARSER passa a ser o dono (ele libera)
 *    - yylloc (linha/coluna de inicio e fim) preenchido em todo token
 *    - erro lexico: mensagem em stderr, erros_lexicos++ e retorno INVALIDO
 * ==================================================================== */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.tab.h"   /* tokens, yylval e yylloc gerados pelo Bison (inclui ast.h) */

/* ---- Estado do analisador ------------------------------------------ */

int erros_lexicos = 0;    /* declarado como extern em ast.h; main.c consulta */

/* Posicao do PROXIMO caractere a ser lido (contagem a partir de 1).
 * Colunas contam caracteres: bytes de continuacao UTF-8 nao contam,
 * entao um 'a' com til ocupa uma coluna, como no editor. */
static int linha = 1, coluna = 1;
static int com_linha = 0, com_coluna = 0;   /* onde comecou um comentario de bloco */

/* ---- Posicao de cada token (yylloc) --------------------------------- */

/* O Flex executa YY_USER_ACTION antes da acao de TODA regra. Aqui ela
 * guarda o inicio do lexema em yylloc e avanca linha/coluna ate o fim dele.
 * last_column e a coluna logo APOS o ultimo caractere (fim exclusivo). */
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

/* No fim do arquivo nao ha lexema: inicio e fim sao o mesmo ponto. */
static void posicao_fim_de_arquivo(void)
{
    yylloc.first_line = yylloc.last_line = linha;
    yylloc.first_column = yylloc.last_column = coluna;
}

/* ---- Valores semanticos --------------------------------------------- */

/* Copia n bytes de s para memoria alocada (o parser libera com free). */
static char *copiar(const char *s, size_t n)
{
    char *p = malloc(n + 1);
    if (!p) { fputs("Memoria insuficiente.\n", stderr); exit(EXIT_FAILURE); }
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

/* ---- Erros lexicos --------------------------------------------------- */

/* Formato exigido pelos testes do grupo: Erro lexico linha L, coluna C: mensagem */
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

/* Caractere ASCII que nao pertence a linguagem: mensagem com dica quando e
 * um engano comum de quem vem de outras linguagens. */
static void erro_caractere_ascii(void)
{
    unsigned char c = (unsigned char)yytext[0];
    if (c >= 32 && c < 127)
        erro_lexico(yylloc.first_line, yylloc.first_column, "caractere invalido '%c'.", c);
    else
        erro_lexico(yylloc.first_line, yylloc.first_column, "caractere de controle invalido (codigo 0x%02X).", c);
}

/* Caractere UTF-8 (2 a 4 bytes) fora de um texto: so e aceito dentro de
 * :...:. Mostra o codigo Unicode e, para os casos que mais aparecem ao
 * copiar codigo de PDF/Word, explica o que trocar. */
static void erro_caractere_utf8(void)
{
    unsigned long cp;
    int i;
    unsigned char c = (unsigned char)yytext[0];
    const char *dica = NULL;

    cp = (yyleng == 2) ? (c & 0x1F) : (yyleng == 3) ? (c & 0x0F) : (c & 0x07);
    for (i = 1; i < yyleng; i++) cp = (cp << 6) | ((unsigned char)yytext[i] & 0x3F);

    switch (cp) {
        case 0x00A0: dica = "espaco nao separavel; troque por um espaco comum"; break;
        case 0x2013: case 0x2014: case 0x2212:
                     dica = "parece um hifen; use o hifen comum '-'"; break;
        case 0x2018: case 0x2019: case 0x201C: case 0x201D:
                     dica = "aspas tipograficas; textos usam dois-pontos, como em :texto:"; break;
        default: break;
    }
    if (dica)
        erro_lexico(yylloc.first_line, yylloc.first_column,
                    "caractere nao permitido '%s' (U+%04lX): %s.", yytext, cp, dica);
    else
        erro_lexico(yylloc.first_line, yylloc.first_column,
                    "caractere nao permitido fora de textos '%s' (U+%04lX). Identificadores "
                    "aceitam apenas letras sem acento, numeros e '_'.", yytext, cp);
}
%}

%option noyywrap never-interactive nounput noinput nodefault

%x COMENTARIO

/* ---- Nomes de padroes ------------------------------------------------ */

DIGITO    [0-9]
LETRA_    [A-Za-z_]
IDCHAR    [A-Za-z0-9_]
WS        [ \t\r\n]

/* Comentarios (tambem usados no "olhar adiante" da regra do hifen) */
COM_LINHA   "~>"[^\n]*\n
COM_BLOCO   "~{"([^}]|"}"+[^}~])*"}"+"~"

/* O que pode vir depois do '-' que FECHA uma condicao: espacos e comentarios
 * e, entao, 'succede' (condicional) ou '[' (mentre). 'succede' so conta se
 * nao vier colado em letra/numero ('a -succede_x' e uma subtracao). */
FECHA_DEPOIS  ({WS}|{COM_LINHA}|{COM_BLOCO})*("succede"[^A-Za-z0-9_]|"[")

%%

 /* ================= 1. BOM UTF-8 E ESPACOS EM BRANCO =================
  * Alguns editores do Windows gravam o BOM no inicio do arquivo; ele e
  * ignorado e nao conta como coluna. Espacos, tabs e quebras de linha
  * (inclusive CRLF) separam tokens e sao descartados. */
"\xEF\xBB\xBF"                  { coluna = 1; }
{WS}+                           { /* ignora */ }

 /* ================= 2. COMENTARIOS =================
  * ~> vai ate o fim da linha. ~{ ... }~ pode ocupar varias linhas e nao
  * aninha. O estado exclusivo COMENTARIO engole tudo ate achar }~. */
"~>".*                          { /* comentario de uma linha */ }
"~{"                            { com_linha = yylloc.first_line; com_coluna = yylloc.first_column; BEGIN(COMENTARIO); }
<COMENTARIO>"}~"                { BEGIN(INITIAL); }
<COMENTARIO>[^}\n]+             { /* conteudo do comentario */ }
<COMENTARIO>"}"|\n              { /* conteudo do comentario */ }
<COMENTARIO><<EOF>>             {
                                    /* arquivo acabou com o comentario aberto: o erro
                                     * aponta onde o comentario COMECOU */
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

 /* ================= 3. PALAVRAS RESERVADAS =================
  * Ficam ANTES da regra de identificador (desempate pela ordem).
  * Diferenciam maiusculas de minusculas: 'Traduzi' e reservada, 'traduzi' nao.
  * "in caso di" sao tres palavras reservadas separadas. */
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

 /* ================= 4. NUMEROS =================
  * O texto original e preservado em yylval.texto. Numero negativo nao existe
  * como literal: '-5' e o operador unario '-' seguido de 5. */
{DIGITO}+"."{DIGITO}+           { yylval.texto = copiar(yytext, (size_t)yyleng); return FLOAT_LIT; }
{DIGITO}+                       { yylval.texto = copiar(yytext, (size_t)yyleng); return INT_LIT; }

 /* Formas invalidas (o maior casamento faz estas regras vencerem as de cima
  * quando o numero esta mal formado) */
{DIGITO}+"."                    { INVALIDO_COM("numero real invalido '%s': faltam digitos depois do ponto.", yytext); }
"."{DIGITO}+                    { INVALIDO_COM("numero real invalido '%s': faltam digitos antes do ponto.", yytext); }
{DIGITO}+"."{DIGITO}+{LETRA_}{IDCHAR}*  { INVALIDO_COM("numero real mal formado '%s'.", yytext); }
{DIGITO}+{LETRA_}{IDCHAR}*      { INVALIDO_COM("identificador nao pode comecar com numero ('%s').", yytext); }

 /* ================= 5. IDENTIFICADORES =================
  * (letra | _) (letra | digito | _)*  -  sem acentos; 'idade', 'Idade' e
  * 'IDADE' sao tres identificadores diferentes. */
{LETRA_}{IDCHAR}*               { yylval.texto = copiar(yytext, (size_t)yyleng); return IDENT; }

 /* ================= 6. TEXTOS =================
  * :conteudo:  (sem ':' e sem quebra de linha dentro). O conteudo vai para o
  * parser SEM os dois-pontos; o gerador de codigo recoloca as aspas. */
":"[^:\r\n]*":"                 { yylval.texto = copiar(yytext + 1, (size_t)yyleng - 2); return TEXTO_LIT; }
":"[^:\r\n]*                    { INVALIDO_COM("texto sem delimitador de fechamento ':' na mesma linha."); }

 /* ================= 7. HIFEN =================
  * O '-' tem tres usos (secao 3 da definicao da linguagem):
  *   1) logo depois de 'di' ou 'mentre'      abre a condicao
  *   2) logo antes de 'succede' ou de '['    fecha a condicao
  *   3) qualquer outro                       subtracao ou sinal negativo
  * O lexer so precisa reconhecer o caso 2, olhando adiante (contexto a
  * direita, a barra '/'): devolve FIM_CONDICAO. Os demais devolvem '-' e o
  * Bison decide pela posicao na gramatica (condicao: '-' expressao FIM_CONDICAO). */
"-"/{FECHA_DEPOIS}              { return FIM_CONDICAO; }
"-"                             { return '-'; }

 /* ================= 8. OPERADORES E DELIMITADORES =================
  * Compostos viram tokens nomeados; os de um caractere voltam como o
  * proprio caractere (como o parser.y os usa). */
">="                            { return MAIOR_IGUAL; }
"<="                            { return MENOR_IGUAL; }
"=="                            { return IGUAL; }
"!="                            { return DIFERENTE; }
[+*/<>=()\[\],;]                { return yytext[0]; }

 /* ================= 9. ERROS LEXICOS =================
  * Tudo que sobra nao pertence a linguagem. O lexer avisa (linha e coluna),
  * devolve INVALIDO e CONTINUA lendo, para achar mais de um erro por vez. */
"!"                             { INVALIDO_COM("caractere '!' inesperado. Use 'not' para negacao ou '!=' para diferente."); }
"&"                             { INVALIDO_COM("caractere '&' invalido. Use 'and'."); }
"|"                             { INVALIDO_COM("caractere '|' invalido. Use 'or'."); }
["']                            { INVALIDO_COM("caractere %c invalido. Textos usam dois-pontos, como em :texto:.", yytext[0]); }
[{}]                            { INVALIDO_COM("caractere '%c' invalido. Blocos usam '[' e ']'.", yytext[0]); }
"~"                             { INVALIDO_COM("simbolo '~' isolado. Comentarios usam '~>' (uma linha) ou '~{ ... }~' (varias linhas)."); }
[\xC2-\xDF][\x80-\xBF]          { erro_caractere_utf8(); return INVALIDO; }
[\xE0-\xEF][\x80-\xBF][\x80-\xBF]               { erro_caractere_utf8(); return INVALIDO; }
[\xF0-\xF4][\x80-\xBF][\x80-\xBF][\x80-\xBF]    { erro_caractere_utf8(); return INVALIDO; }
[\x80-\xFF]                     { INVALIDO_COM("byte invalido 0x%02X (o arquivo deve estar em UTF-8).", (unsigned char)yytext[0]); }
.                               { erro_caractere_ascii(); return INVALIDO; }

<INITIAL><<EOF>>                { posicao_fim_de_arquivo(); yyterminate(); }

%%
