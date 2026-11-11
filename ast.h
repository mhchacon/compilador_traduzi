#ifndef AST_H
#define AST_H

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
    DataType data_type;       // Preenchido na análise semântica
    char* name;               // Para identificadores, nomes de ações, etc.
    char* string_value;       // Para strings literais
    int int_value;
    float float_value;
    int is_mutable;           // Flag para variáveis 'mutabile'
    
    struct ASTNode* left;     // Filhos da árvore (esquerda/direita)
    struct ASTNode* right;
    struct ASTNode* next;     // Para listas (comandos num bloco, parâmetros)
} ASTNode;

#endif