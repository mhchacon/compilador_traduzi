#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "ast.h"

typedef struct Symbol {
    char name[100];
    DataType type;
    int is_mutable;
    int is_initialized;
    int is_action;          // 1 se for uma ação (função), 0 se variável
    struct Symbol* next;
} Symbol;

typedef struct Scope {
    Symbol* symbols;
    struct Scope* parent;
} Scope;

void init_symbol_table();
void enter_scope();
void exit_scope();
int add_symbol(const char* name, DataType type, int is_mutable, int is_action);
Symbol* lookup_symbol(const char* name);
Symbol* lookup_symbol_current_scope(const char* name);
void free_symbol_table();

#endif