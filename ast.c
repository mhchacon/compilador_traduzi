#include "ast.h"
#include <stdlib.h>
#include <string.h>

static void *alocar(size_t tamanho)
{
    void *memoria = malloc(tamanho);
    if (!memoria)
    {
        fputs("Memoria insuficiente.\n", stderr);
        exit(EXIT_FAILURE);
    }
    return memoria;
}

char *ast_texto(const char *texto)
{
    size_t tamanho = strlen(texto) + 1;
    char *copia = alocar(tamanho);
    memcpy(copia, texto, tamanho);
    return copia;
}

AST *ast_criar(ASTTipo tipo, const char *valor, int linha, int coluna)
{
    AST *no = alocar(sizeof *no);
    *no = (AST){0};
    no->tipo = tipo;
    no->valor = valor ? ast_texto(valor) : NULL;
    no->linha = linha;
    no->coluna = coluna;
    return no;
}

AST *ast_adicionar(AST *pai, AST *filho)
{
    AST **filhos;
    if (!filho)
        return pai;
    filhos = realloc(pai->filhos, (pai->quantidade + 1) * sizeof *filhos);
    if (!filhos)
    {
        fputs("Memoria insuficiente.\n", stderr);
        exit(EXIT_FAILURE);
    }
    pai->filhos = filhos;
    pai->filhos[pai->quantidade++] = filho;
    return pai;
}

AST *ast_operacao(const char *operador, AST *esquerda, AST *direita, int linha, int coluna)
{
    AST *no = ast_criar(AST_OPERACAO, operador, linha, coluna);
    ast_adicionar(no, esquerda);
    return ast_adicionar(no, direita);
}

void ast_imprimir(const AST *no, FILE *saida, int nivel)
{
    static const char *nomes[] = {
        "programa", "acao", "parametros", "parametro", "bloco", "declaracao",
        "atribuicao", "input", "stampa", "condicional", "mentre", "per",
        "fermarsi", "continuare", "ritorna", "chamada", "argumentos",
        "conversao", "operacao", "identificador", "inteiro", "real", "texto", "booleano"};
    size_t i;
    if (!no)
        return;
    fprintf(saida, "%*s%s", nivel * 2, "", nomes[no->tipo]);
    if (no->valor)
        fprintf(saida, " %s", no->valor);
    if (no->tipo_dado)
        fprintf(saida, " tipo=%s", no->tipo_dado);
    if (no->mutavel)
        fputs(" mutabile", saida);
    fprintf(saida, " (%d:%d)\n", no->linha, no->coluna);
    for (i = 0; i < no->quantidade; ++i)
        ast_imprimir(no->filhos[i], saida, nivel + 1);
}

void ast_liberar(AST *no)
{
    size_t i;
    if (!no)
        return;
    for (i = 0; i < no->quantidade; ++i)
        ast_liberar(no->filhos[i]);
    free(no->filhos);
    free(no->valor);
    free(no->tipo_dado);
    free(no);
}
