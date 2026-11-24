#include "codegen_python.h"
#include <stdio.h>
#include <string.h>

static const AST *current_action;

static const char *find_type(const AST *node, const char *name)
{
    size_t i;
    if (!node)
        return NULL;
    if ((node->tipo == AST_DECLARACAO || node->tipo == AST_PARAMETRO) &&
        node->valor && strcmp(node->valor, name) == 0)
        return node->tipo_dado;
    for (i = 0; i < node->quantidade; ++i) {
        const char *type = find_type(node->filhos[i], name);
        if (type)
            return type;
    }
    return NULL;
}

static void emit_indent(FILE *output, int level)
{
    int i;
    for (i = 0; i < level; ++i)
        fputs("    ", output);
}

static void emit_string(FILE *output, const char *value)
{
    const unsigned char *cursor = (const unsigned char *)value;
    fputc('"', output);
    while (*cursor) {
        switch (*cursor) {
        case '\\': fputs("\\\\", output); break;
        case '"': fputs("\\\"", output); break;
        case '\n': fputs("\\n", output); break;
        case '\r': fputs("\\r", output); break;
        case '\t': fputs("\\t", output); break;
        default: fputc(*cursor, output); break;
        }
        ++cursor;
    }
    fputc('"', output);
}

static void emit_expression(const AST *node, FILE *output)
{
    size_t i;
    if (!node)
        return;

    switch (node->tipo) {
    case AST_IDENTIFICADOR:
    case AST_CHAMADA:
    case AST_CONVERSAO:
        if (node->tipo == AST_IDENTIFICADOR)
            fputs(node->valor, output);
        else if (node->tipo == AST_CHAMADA) {
            fputs(node->valor, output);
            fputc('(', output);
            if (node->quantidade && node->filhos[0]) {
                const AST *arguments = node->filhos[0];
                for (i = 0; i < arguments->quantidade; ++i) {
                    if (i) fputs(", ", output);
                    emit_expression(arguments->filhos[i], output);
                }
            }
            fputc(')', output);
        } else {
            fputs(strcmp(node->valor, "intero") == 0 ? "int(" : "float(", output);
            if (node->quantidade)
                emit_expression(node->filhos[0], output);
            fputc(')', output);
        }
        break;
    case AST_INTEIRO:
    case AST_REAL:
        fputs(node->valor, output);
        break;
    case AST_TEXTO:
        emit_string(output, node->valor ? node->valor : "");
        break;
    case AST_BOOLEANO:
        fputs(strcmp(node->valor, "true") == 0 ? "True" : "False", output);
        break;
    case AST_OPERACAO:
        if (strcmp(node->valor, "not") == 0) {
            fputs("(not ", output);
            emit_expression(node->filhos[0], output);
            fputc(')', output);
        } else if (node->quantidade == 1) {
            fputc('(', output);
            fputs(node->valor, output);
            emit_expression(node->filhos[0], output);
            fputc(')', output);
        } else {
            fputc('(', output);
            emit_expression(node->filhos[0], output);
            fprintf(output, " %s ", node->valor);
            emit_expression(node->filhos[1], output);
            fputc(')', output);
        }
        break;
    default:
        break;
    }
}

static void emit_statement(const AST *node, FILE *output, int level);

static void emit_block(const AST *node, FILE *output, int level)
{
    size_t i;
    if (!node || node->quantidade == 0) {
        emit_indent(output, level);
        fputs("pass\n", output);
        return;
    }
    for (i = 0; i < node->quantidade; ++i)
        emit_statement(node->filhos[i], output, level);
}

static void emit_statement(const AST *node, FILE *output, int level)
{
    if (!node)
        return;
    switch (node->tipo) {
    case AST_DECLARACAO:
        emit_indent(output, level);
        fputs(node->valor, output);
        fputs(" = ", output);
        if (node->quantidade)
            emit_expression(node->filhos[0], output);
        else
            fputs("None", output);
        fputc('\n', output);
        break;
    case AST_ATRIBUICAO:
        emit_indent(output, level);
        fprintf(output, "%s = ", node->valor);
        emit_expression(node->filhos[0], output);
        fputc('\n', output);
        break;
    case AST_ENTRADA:
    {
        const char *type = find_type(current_action, node->valor);
        emit_indent(output, level);
        fprintf(output, "%s = ", node->valor);
        if (type && strcmp(type, "int") == 0)
            fputs("int(input())", output);
        else if (type && strcmp(type, "float") == 0)
            fputs("float(input())", output);
        else if (type && strcmp(type, "bool") == 0)
            fputs("input().lower() == 'true'", output);
        else
            fputs("input()", output);
        fputc('\n', output);
        break;
    }
    case AST_SAIDA:
        emit_indent(output, level);
        fputs("print(", output);
        emit_expression(node->filhos[0], output);
        fputs(")\n", output);
        break;
    case AST_CONDICIONAL:
        emit_indent(output, level);
        fputs("if ", output);
        emit_expression(node->filhos[0], output);
        fputs(":\n", output);
        emit_block(node->filhos[1], output, level + 1);
        if (node->quantidade > 2 && node->filhos[2]) {
            emit_indent(output, level);
            fputs("else:\n", output);
            if (node->filhos[2]->tipo == AST_CONDICIONAL) {
                emit_statement(node->filhos[2], output, level + 1);
            } else {
                emit_block(node->filhos[2], output, level + 1);
            }
        }
        break;
    case AST_ENQUANTO:
        emit_indent(output, level);
        fputs("while ", output);
        emit_expression(node->filhos[0], output);
        fputs(":\n", output);
        emit_block(node->filhos[1], output, level + 1);
        break;
    case AST_PARA:
        emit_indent(output, level);
        fprintf(output, "for %s in range(", node->valor);
        emit_expression(node->filhos[0], output);
        fputs(", ", output);
        emit_expression(node->filhos[1], output);
        fputs(" + 1):\n", output);
        emit_block(node->filhos[2], output, level + 1);
        break;
    case AST_PARAR:
        emit_indent(output, level);
        fputs("break\n", output);
        break;
    case AST_CONTINUAR:
        emit_indent(output, level);
        fputs("continue\n", output);
        break;
    case AST_RETORNO:
        emit_indent(output, level);
        fputs("return", output);
        if (node->quantidade) {
            fputc(' ', output);
            emit_expression(node->filhos[0], output);
        }
        fputc('\n', output);
        break;
    case AST_CHAMADA:
        emit_indent(output, level);
        emit_expression(node, output);
        fputc('\n', output);
        break;
    default:
        break;
    }
}

void generate_python(const AST *root, FILE *output)
{
    size_t i;
    if (!root || !output)
        return;
    for (i = 0; i < root->quantidade; ++i) {
        const AST *action = root->filhos[i];
        const AST *parameters = action->filhos[0];
        const AST *block = action->filhos[1];
        size_t parameter;

        current_action = action;
        fprintf(output, "def %s(", action->valor);
        for (parameter = 0; parameters && parameter < parameters->quantidade; ++parameter) {
            if (parameter) fputs(", ", output);
            fputs(parameters->filhos[parameter]->valor, output);
        }
        fputs("):\n", output);
        emit_block(block, output, 1);
        fputc('\n', output);
    }
    fputs("if __name__ == '__main__':\n", output);
    fputs("    Traduzi()\n", output);
}
