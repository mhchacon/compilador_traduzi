#include <stdio.h>
#include "ast.h"

int indent_level = 0;

void print_indent() {
    for (int i = 0; i < indent_level; i++) {
        printf("    ");
    }
}

void generate_python(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            generate_python(node->left);
            // Python precisa chamar a função principal no final do arquivo
            printf("\nif __name__ == '__main__':\n    Traduzi()\n");
            break;

        case NODE_ACTION:
            printf("\ndef %s():\n", node->name); // Adaptar para incluir parâmetros
            indent_level++;
            if (!node->right) {
                print_indent();
                printf("pass\n");
            } else {
                generate_python(node->right);
            }
            indent_level--;
            break;

        case NODE_VAR_DECL:
            if (node->left) { // Apenas gera código se houver atribuição
                print_indent();
                printf("%s = ", node->name);
                generate_python(node->left);
                printf("\n");
            }
            break;

        case NODE_ASSIGN:
            print_indent();
            printf("%s = ", node->name);
            generate_python(node->right);
            printf("\n");
            break;

        case NODE_PRINT:
            print_indent();
            printf("print(");
            generate_python(node->left);
            printf(")\n");
            break;

        case NODE_IF:
            print_indent();
            printf("if ");
            generate_python(node->left);
            printf(":\n");
            indent_level++;
            generate_python(node->right);
            indent_level--;
            if (node->next) { // altrimenti
                print_indent();
                printf("else:\n");
                indent_level++;
                generate_python(node->next);
                indent_level--;
            }
            break;

        case NODE_WHILE:
            print_indent();
            printf("while ");
            generate_python(node->left);
            printf(":\n");
            indent_level++;
            generate_python(node->right);
            indent_level--;
            break;
            
        case NODE_FOR:
            print_indent();
            // A estrutura de For precisaria dos nós para o identificador, início e fim.
            // Exemplo simplificado assumindo node->name, node->left (início) e node->right (fim)
            printf("for %s in range(", node->name);
            generate_python(node->left); // Início
            printf(", ");
            generate_python(node->right); // Fim
            printf(" + 1):\n"); // per é inclusivo
            indent_level++;
            // Assumindo que o corpo do laço esteja anexado após ou em um campo child
            // generate_python(node->body); 
            indent_level--;
            break;

        case NODE_BREAK:
            print_indent();
            printf("break\n");
            break;
            
        case NODE_CONTINUE:
            print_indent();
            printf("continue\n");
            break;

        case NODE_BINOP:
            generate_python(node->left);
            printf(" %s ", node->name); // node->name guardaria o operador (ex: "+", "*", "and")
            generate_python(node->right);
            break;

        case NODE_ID:
            printf("%s", node->name);
            break;

        case NODE_LITERAL:
            if (node->data_type == T_INT) printf("%d", node->int_value);
            else if (node->data_type == T_FLOAT) printf("%f", node->float_value);
            else if (node->data_type == T_STRING) printf("\"%s\"", node->string_value); // Python strings
            else if (node->data_type == T_BOOL) printf(node->int_value ? "True" : "False");
            break;
            
        // Demais nós omitidos para brevidade
    }
    
    if (node->type != NODE_IF && node->type != NODE_WHILE && node->type != NODE_FOR && node->type != NODE_ACTION) {
        generate_python(node->next); // Continua para o próximo comando no bloco
    }
}