#include <stdio.h>
#include <string.h>
#include "semantic.h"
#include "symbol_table.h"

static int semantic_errors;
static DataType current_return_type;
static int loop_depth;

static void report_error(const AST *node, const char *message)
{
    fprintf(stderr, "Erro semantico linha %d, coluna %d: %s\n",
            node ? node->linha : 0, node ? node->coluna : 0, message);
    ++semantic_errors;
}

static DataType type_from_name(const char *name)
{
    if (!name) return T_UNKNOWN;
    if (strcmp(name, "int") == 0) return T_INT;
    if (strcmp(name, "float") == 0) return T_FLOAT;
    if (strcmp(name, "string") == 0) return T_STRING;
    if (strcmp(name, "bool") == 0) return T_BOOL;
    if (strcmp(name, "void") == 0) return T_VOID;
    return T_UNKNOWN;
}

static int assignable(DataType target, DataType value)
{
    return target == value || (target == T_FLOAT && value == T_INT);
}

static DataType analyze_expression(const AST *node);
static void analyze_statement(const AST *node);

static void analyze_block(const AST *node)
{
    size_t i;
    enter_scope();
    for (i = 0; node && i < node->quantidade; ++i)
        analyze_statement(node->filhos[i]);
    exit_scope();
}

static DataType analyze_call(const AST *node)
{
    const Symbol *symbol = lookup_symbol(node->valor);
    const AST *arguments = node->quantidade ? node->filhos[0] : NULL;
    size_t i;

    if (!symbol || !symbol->is_action) {
        char message[160];
        snprintf(message, sizeof message, "acao '%s' nao foi declarada.", node->valor);
        report_error(node, message);
        return T_UNKNOWN;
    }
    for (i = 0; arguments && i < arguments->quantidade; ++i)
        analyze_expression(arguments->filhos[i]);
    return symbol->type;
}

static DataType analyze_expression(const AST *node)
{
    DataType left, right;
    const Symbol *symbol;

    if (!node) return T_UNKNOWN;
    switch (node->tipo) {
    case AST_INTEIRO: return T_INT;
    case AST_REAL: return T_FLOAT;
    case AST_TEXTO: return T_STRING;
    case AST_BOOLEANO: return T_BOOL;
    case AST_IDENTIFICADOR:
        symbol = lookup_symbol(node->valor);
        if (!symbol || symbol->is_action) {
            report_error(node, "identificador nao declarado.");
            return T_UNKNOWN;
        }
        return symbol->type;
    case AST_CHAMADA:
        return analyze_call(node);
    case AST_CONVERSAO:
        if (node->quantidade) analyze_expression(node->filhos[0]);
        return strcmp(node->valor, "intero") == 0 ? T_INT : T_FLOAT;
    case AST_OPERACAO:
        left = node->quantidade ? analyze_expression(node->filhos[0]) : T_UNKNOWN;
        right = node->quantidade > 1 ? analyze_expression(node->filhos[1]) : T_UNKNOWN;
        if (strcmp(node->valor, "-") == 0 && node->quantidade == 1) {
            if (left != T_INT && left != T_FLOAT)
                report_error(node, "operador unario exige operando numerico.");
            return left;
        }
        if (strcmp(node->valor, "and") == 0 || strcmp(node->valor, "or") == 0) {
            if (left != T_BOOL || right != T_BOOL)
                report_error(node, "operadores logicos exigem operandos bool.");
            return T_BOOL;
        }
        if (strcmp(node->valor, "not") == 0) {
            if (left != T_BOOL)
                report_error(node, "o operador not exige um operando bool.");
            return T_BOOL;
        }
        if (strcmp(node->valor, "<") == 0 || strcmp(node->valor, ">") == 0 ||
            strcmp(node->valor, "<=") == 0 || strcmp(node->valor, ">=") == 0 ||
            strcmp(node->valor, "==") == 0 || strcmp(node->valor, "!=") == 0) {
            if (!assignable(left, right) && !assignable(right, left))
                report_error(node, "operandos de comparacao possuem tipos incompativeis.");
            return T_BOOL;
        }
        if (left != T_INT && left != T_FLOAT)
            report_error(node, "operacao aritmetica exige operandos numericos.");
        if (right != T_INT && right != T_FLOAT)
            report_error(node, "operacao aritmetica exige operandos numericos.");
        return left == T_FLOAT || right == T_FLOAT ? T_FLOAT : T_INT;
    default:
        report_error(node, "expressao invalida.");
        return T_UNKNOWN;
    }
}

static void analyze_statement(const AST *node)
{
    Symbol *symbol;
    DataType declared_type, value_type;

    if (!node) return;
    switch (node->tipo) {
    case AST_DECLARACAO:
        declared_type = type_from_name(node->tipo_dado);
        if (!add_symbol(node->valor, declared_type, node->mutavel, 0)) {
            report_error(node, "variavel ja declarada neste escopo.");
        } else if (node->quantidade) {
            symbol = lookup_symbol_current_scope(node->valor);
            value_type = analyze_expression(node->filhos[0]);
            if (!assignable(declared_type, value_type))
                report_error(node, "tipo incompativel na inicializacao.");
            symbol->is_initialized = 1;
        }
        break;
    case AST_ATRIBUICAO:
        symbol = lookup_symbol(node->valor);
        if (!symbol || symbol->is_action) {
            report_error(node, "variavel atribuida nao foi declarada.");
        } else {
            value_type = analyze_expression(node->filhos[0]);
            if (!symbol->is_mutable && symbol->is_initialized)
                report_error(node, "variavel imutavel ja foi inicializada.");
            if (!assignable(symbol->type, value_type))
                report_error(node, "tipo incompativel na atribuicao.");
            symbol->is_initialized = 1;
        }
        break;
    case AST_ENTRADA:
        symbol = lookup_symbol(node->valor);
        if (!symbol || symbol->is_action)
            report_error(node, "variavel de entrada nao foi declarada.");
        else
            symbol->is_initialized = 1;
        break;
    case AST_SAIDA:
        if (node->quantidade) analyze_expression(node->filhos[0]);
        break;
    case AST_CONDICIONAL:
        if (analyze_expression(node->filhos[0]) != T_BOOL)
            report_error(node, "a condicao deve ser bool.");
        analyze_block(node->filhos[1]);
        if (node->quantidade > 2) {
            if (node->filhos[2]->tipo == AST_BLOCO)
                analyze_block(node->filhos[2]);
            else
                analyze_statement(node->filhos[2]);
        }
        break;
    case AST_ENQUANTO:
        if (analyze_expression(node->filhos[0]) != T_BOOL)
            report_error(node, "a condicao do enquanto deve ser bool.");
        ++loop_depth;
        analyze_block(node->filhos[1]);
        --loop_depth;
        break;
    case AST_PARA:
        if (analyze_expression(node->filhos[0]) != T_INT ||
            analyze_expression(node->filhos[1]) != T_INT)
            report_error(node, "os limites do para devem ser inteiros.");
        ++loop_depth;
        enter_scope();
        if (!add_symbol(node->valor, T_INT, 0, 0))
            report_error(node, "contador do para ja declarado.");
        {
            size_t i;
            for (i = 0; i < node->filhos[2]->quantidade; ++i)
                analyze_statement(node->filhos[2]->filhos[i]);
        }
        exit_scope();
        --loop_depth;
        break;
    case AST_PARAR:
    case AST_CONTINUAR:
        if (loop_depth == 0)
            report_error(node, "comando de loop usado fora de um laco.");
        break;
    case AST_RETORNO:
        value_type = node->quantidade ? analyze_expression(node->filhos[0]) : T_VOID;
        if (!assignable(current_return_type, value_type))
            report_error(node, "tipo de retorno incompativel com a acao.");
        break;
    case AST_CHAMADA:
        analyze_call(node);
        break;
    default:
        report_error(node, "comando invalido.");
        break;
    }
}

static void analyze_action(const AST *node)
{
    const AST *parameters = node->filhos[0];
    const AST *block = node->filhos[1];
    size_t i;

    current_return_type = type_from_name(node->tipo_dado);
    enter_scope();
    for (i = 0; parameters && i < parameters->quantidade; ++i) {
        const AST *parameter = parameters->filhos[i];
        if (!add_symbol(parameter->valor, type_from_name(parameter->tipo_dado), 0, 0))
            report_error(parameter, "parametro ja declarado.");
    }
    for (i = 0; block && i < block->quantidade; ++i)
        analyze_statement(block->filhos[i]);
    exit_scope();
}

int check_semantics(const AST *root)
{
    const Symbol *main_action;
    size_t i;

    semantic_errors = 0;
    loop_depth = 0;
    init_symbol_table();
    for (i = 0; root && i < root->quantidade; ++i) {
        const AST *action = root->filhos[i];
        if (!add_symbol(action->valor, type_from_name(action->tipo_dado), 0, 1))
            report_error(action, "acao ja declarada.");
    }
    main_action = lookup_symbol("Traduzi");
    if (!main_action || main_action->type != T_VOID)
        report_error(root, "a acao Traduzi deve existir e ser void.");
    for (i = 0; root && i < root->quantidade; ++i)
        analyze_action(root->filhos[i]);
    free_symbol_table();
    return semantic_errors == 0;
}
