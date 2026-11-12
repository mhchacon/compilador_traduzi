#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

Scope* current_scope = NULL;

void init_symbol_table() {
    current_scope = NULL;
    enter_scope(); // Escopo global (para ações)
}

void enter_scope() {
    Scope* new_scope = (Scope*)malloc(sizeof(Scope));
    new_scope->symbols = NULL;
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

void exit_scope() {
    if (!current_scope) return;
    Scope* old_scope = current_scope;
    current_scope = current_scope->parent;
    
    Symbol* curr = old_scope->symbols;
    while (curr) {
        Symbol* temp = curr;
        curr = curr->next;
        free(temp);
    }
    free(old_scope);
}

int add_symbol(const char* name, DataType type, int is_mutable, int is_action) {
    if (lookup_symbol_current_scope(name) != NULL) {
        return 0; // Erro: já declarado no escopo atual
    }
    Symbol* sym = (Symbol*)malloc(sizeof(Symbol));
    strncpy(sym->name, name, 99);
    sym->name[99] = '\0';
    sym->type = type;
    sym->is_mutable = is_mutable;
    sym->is_initialized = 0;
    sym->is_action = is_action;
    sym->next = current_scope->symbols;
    current_scope->symbols = sym;
    return 1;
}

Symbol* lookup_symbol_current_scope(const char* name) {
    if (!current_scope) return NULL;
    Symbol* curr = current_scope->symbols;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

Symbol* lookup_symbol(const char* name) {
    Scope* scope = current_scope;
    while (scope) {
        Symbol* curr = scope->symbols;
        while (curr) {
            if (strcmp(curr->name, name) == 0) return curr;
            curr = curr->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

void free_symbol_table() {
    while (current_scope) {
        exit_scope();
    }
}