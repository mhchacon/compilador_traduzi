#include <stdio.h>
#include <string.h>
#include "semantic.h"
#include "symbol_table.h"

int semantic_errors = 0;
int in_loop = 0; // Para checar fermarsi/continuare
DataType current_action_return_type = T_VOID;

void report_error(const char* msg, const char* detail) {
    printf("Erro (Semântica): %s %s\n", msg, detail ? detail : "");
    semantic_errors++;
}

void analyze_node(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            init_symbol_table();
            analyze_node(node->left); // Analisa todas as ações
            
            // Verifica a existência da ação obrigatória Traduzi
            Symbol* main_action = lookup_symbol("Traduzi");
            if (!main_action || !main_action->is_action || main_action->type != T_VOID) {
                report_error("Ação principal 'Traduzi' do tipo void não encontrada.", NULL);
            }
            break;

        case NODE_ACTION:
            if (!add_symbol(node->name, node->data_type, 0, 1)) {
                report_error("Ação já declarada: ", node->name);
            }
            current_action_return_type = node->data_type;
            enter_scope();
            // Analisa parâmetros (node->left) e bloco (node->right)
            analyze_node(node->left);
            analyze_node(node->right);
            exit_scope();
            break;

        case NODE_VAR_DECL:
            if (!add_symbol(node->name, node->data_type, node->is_mutable, 0)) {
                report_error("Variável já declarada no escopo atual: ", node->name);
            }
            Symbol* sym = lookup_symbol_current_scope(node->name);
            if (node->left) { // Se possui valor inicial (atribuição)
                analyze_node(node->left);
                if (node->left->data_type != node->data_type && 
                   !(node->data_type == T_FLOAT && node->left->data_type == T_INT)) {
                    report_error("Tipo incompatível na inicialização de: ", node->name);
                }
                sym->is_initialized = 1;
            }
            break;

        case NODE_ASSIGN:
            analyze_node(node->right);
            Symbol* var_sym = lookup_symbol(node->name);
            if (!var_sym) {
                report_error("Variável não declarada: ", node->name);
            } else {
                if (!var_sym->is_mutable && var_sym->is_initialized) {
                    report_error("Variável é imutável e já foi inicializada: ", node->name);
                }
                if (var_sym->type != node->right->data_type && 
                   !(var_sym->type == T_FLOAT && node->right->data_type == T_INT)) {
                    report_error("Atribuição com tipo incompatível para: ", node->name);
                }
                var_sym->is_initialized = 1;
            }
            break;

        case NODE_ID:
            var_sym = lookup_symbol(node->name);
            if (!var_sym) {
                report_error("Variável não declarada: ", node->name);
                node->data_type = T_UNKNOWN;
            } else {
                node->data_type = var_sym->type;
            }
            break;

        case NODE_WHILE:
        case NODE_FOR:
            in_loop++;
            enter_scope();
            analyze_node(node->left); // Condição
            if (node->type == NODE_WHILE && node->left && node->left->data_type != T_BOOL) {
                report_error("A condição do 'mentre' deve ser do tipo bool.", NULL);
            }
            analyze_node(node->right); // Bloco
            exit_scope();
            in_loop--;
            break;

        case NODE_IF:
            analyze_node(node->left); // Condição
            if (node->left && node->left->data_type != T_BOOL) {
                report_error("A condição do 'in caso di' deve ser bool.", NULL);
            }
            enter_scope();
            analyze_node(node->right); // Bloco sucede
            exit_scope();
            if (node->next) { // Bloco altrimenti (se existir)
                enter_scope();
                analyze_node(node->next);
                exit_scope();
            }
            break;

        case NODE_BREAK:
        case NODE_CONTINUE:
            if (in_loop == 0) {
                report_error("'fermarsi' ou 'continuare' usado fora de um laço.", NULL);
            }
            break;
            
        case NODE_BINOP:
            analyze_node(node->left);
            analyze_node(node->right);
            // Lógica simplificada de tipos: int + float = float, etc.
            if (node->left->data_type == T_FLOAT || node->right->data_type == T_FLOAT) {
                node->data_type = T_FLOAT;
            } else {
                node->data_type = T_INT;
            }
            // Adicionar checagem de divisão por constante zero
            break;

        // Omitidos NODE_PRINT, NODE_INPUT para concisão, siga o mesmo padrão
    }
    
    // Analisa irmãos (próximos comandos em um bloco ou ações subsequentes)
    if (node->type != NODE_IF) { // IF lida com o next de forma diferente (altrimenti)
        analyze_node(node->next);
    }
}

int check_semantics(ASTNode* root) {
    semantic_errors = 0;
    analyze_node(root);
    return semantic_errors == 0;
}