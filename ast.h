
#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdio.h>

/* Estrutura AST original */
typedef enum {
    AST_PROGRAMA, AST_ACAO, AST_PARAMETROS, AST_PARAMETRO, AST_BLOCO,
    AST_DECLARACAO, AST_ATRIBUICAO, AST_ENTRADA, AST_SAIDA, AST_CONDICIONAL,
    AST_ENQUANTO, AST_PARA, AST_PARAR, AST_CONTINUAR, AST_RETORNO,
    AST_CHAMADA, AST_ARGUMENTOS, AST_CONVERSAO, AST_OPERACAO,
    AST_IDENTIFICADOR, AST_INTEIRO, AST_REAL, AST_TEXTO, AST_BOOLEANO
} ASTTipo;

typedef struct AST {
    ASTTipo tipo;
    char *valor;
    char *tipo_dado;
    int mutavel;
    int linha;
    int coluna;
    size_t quantidade;
    struct AST **filhos;
} AST;

extern AST *ast_raiz;
extern int erros_sintaticos;
extern int erros_lexicos;

AST *ast_criar(ASTTipo tipo, const char *valor, int linha, int coluna);
AST *ast_adicionar(AST *pai, AST *filho);
AST *ast_operacao(const char *operador, AST *esquerda, AST *direita, int linha, int coluna);
char *ast_texto(const char *texto);
void ast_imprimir(const AST *no, FILE *saida, int nivel);
void ast_liberar(AST *no);

/* Estruturas utilizadas pela análise semântica */
typedef enum {
    T_INT, T_FLOAT, T_STRING, T_BOOL, T_VOID, T_UNKNOWN
} DataType;

typedef enum {
    NODE_PROGRAM, NODE_ACTION, NODE_BLOCK, NODE_VAR_DECL,
    NODE_ASSIGN, NODE_IF, NODE_WHILE, NODE_FOR, NODE_PRINT,
    NODE_INPUT, NODE_BINOP, NODE_ID, NODE_LITERAL, NODE_RETURN,
    NODE_BREAK, NODE_CONTINUE, NODE_CALL
} NodeType;

typedef struct ASTNode {
    NodeType type;
    DataType data_type;
    char* name;
    char* string_value;
    int int_value;
    float float_value;
    int is_mutable;

    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* next;
} ASTNode;

#endif
