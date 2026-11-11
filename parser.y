%code requires {
#include "ast.h"
}
%{
#include "ast.h"
#include <stdlib.h>
int yylex(void);
void yyerror(const char *mensagem);
AST *ast_raiz = NULL;
int erros_sintaticos = 0;
#define NO(t, v, l) ast_criar(t, v, (l).first_line, (l).first_column)
#define OP(v, a, b, l) ast_operacao(v, a, b, (l).first_line, (l).first_column)
%}
%define parse.error detailed
%define parse.lac full
%locations
%expect 0
%union {
    char *texto;
    int booleano;
    AST *no;
}
%token ACAO TRADUZI RITORNA VOID MUTABILE
%token INT FLOAT STRING BOOL TRUE FALSE
%token IN CASO DI SUCCEDE ALTRIMENTI MENTRE PER DE ATE
%token FERMARSI CONTINUARE INPUT STAMPA INTERO REALE AND OR NOT
%token MAIOR_IGUAL MENOR_IGUAL IGUAL DIFERENTE FIM_CONDICAO INVALIDO
%token <texto> IDENT INT_LIT FLOAT_LIT TEXTO_LIT
%type <texto> tipo tipo_retorno nome_acao
%type <booleano> mutabilidade
%type <no> programa acoes acao parametros parametros_opcionais parametro
%type <no> bloco comandos comando declaracao atribuicao entrada saida retorno
%type <no> condicional alternativa enquanto para condicao chamada argumentos argumentos_opcionais
%type <no> expressao ou e nao igualdade relacional soma termo unario primario conversao
%destructor { free($$); } <texto>
%destructor { ast_liberar($$); } <no>
%start programa
%%
programa: acoes { ast_raiz = $1; $$ = NULL; };
acoes:
    acao { $$ = ast_adicionar(NO(AST_PROGRAMA, NULL, @1), $1); }
  | acoes acao { $$ = ast_adicionar($1, $2); }
;
acao: ACAO nome_acao '(' parametros_opcionais ')' tipo_retorno bloco {
    $$ = NO(AST_ACAO, $2, @1);
    free($2);
    $$->tipo_dado = $6;
    ast_adicionar($$, $4);
    ast_adicionar($$, $7);
};
nome_acao: IDENT { $$ = $1; } | TRADUZI { $$ = ast_texto("Traduzi"); };
tipo: INT { $$ = ast_texto("int"); } | FLOAT { $$ = ast_texto("float"); }
    | STRING { $$ = ast_texto("string"); } | BOOL { $$ = ast_texto("bool"); };
tipo_retorno: RITORNA tipo { $$ = $2; } | VOID { $$ = ast_texto("void"); };
parametros_opcionais: %empty { $$ = NO(AST_PARAMETROS, NULL, @$); } | parametros { $$ = $1; };
parametros: parametro { $$ = ast_adicionar(NO(AST_PARAMETROS, NULL, @1), $1); }
    | parametros ',' parametro { $$ = ast_adicionar($1, $3); };
parametro: tipo IDENT {
    $$ = NO(AST_PARAMETRO, $2, @1); free($2); $$->tipo_dado = $1;
};
bloco: '[' comandos ']' { $$ = $2; $$->linha = @1.first_line; $$->coluna = @1.first_column; }
    | '[' comandos error ']' { $$ = $2; yyerrok; };
comandos: %empty { $$ = NO(AST_BLOCO, NULL, @$); }
    | comandos comando { $$ = ast_adicionar($1, $2); }
    | comandos error ';' { $$ = $1; yyerrok; };
comando:
    declaracao { $$ = $1; } | atribuicao { $$ = $1; }
  | entrada { $$ = $1; } | saida { $$ = $1; }
  | condicional { $$ = $1; } | enquanto { $$ = $1; } | para { $$ = $1; }
  | FERMARSI ';' { $$ = NO(AST_PARAR, NULL, @1); }
  | CONTINUARE ';' { $$ = NO(AST_CONTINUAR, NULL, @1); }
  | retorno { $$ = $1; } | chamada ';' { $$ = $1; };
mutabilidade: %empty { $$ = 0; } | MUTABILE { $$ = 1; };
declaracao:
    mutabilidade tipo IDENT ';' {
        $$ = NO(AST_DECLARACAO, $3, @2); free($3); $$->tipo_dado = $2; $$->mutavel = $1;
    }
  | mutabilidade tipo IDENT '=' expressao ';' {
        $$ = NO(AST_DECLARACAO, $3, @2); free($3); $$->tipo_dado = $2; $$->mutavel = $1;
        ast_adicionar($$, $5);
    };
atribuicao: IDENT '=' expressao ';' {
    $$ = ast_adicionar(NO(AST_ATRIBUICAO, $1, @1), $3); free($1);
};
entrada: INPUT '(' IDENT ')' ';' { $$ = NO(AST_ENTRADA, $3, @1); free($3); };
saida: STAMPA '(' expressao ')' ';' { $$ = ast_adicionar(NO(AST_SAIDA, NULL, @1), $3); };
retorno: RITORNA ';' { $$ = NO(AST_RETORNO, NULL, @1); }
    | RITORNA expressao ';' { $$ = ast_adicionar(NO(AST_RETORNO, NULL, @1), $2); };
condicional: IN CASO DI condicao SUCCEDE bloco alternativa {
    $$ = NO(AST_CONDICIONAL, NULL, @1);
    ast_adicionar($$, $4); ast_adicionar($$, $6); ast_adicionar($$, $7);
};
alternativa: %empty { $$ = NULL; } | ALTRIMENTI bloco { $$ = $2; }
    | ALTRIMENTI condicional { $$ = $2; };
condicao: '-' expressao FIM_CONDICAO { $$ = $2; };
enquanto: MENTRE condicao bloco {
    $$ = ast_adicionar(NO(AST_ENQUANTO, NULL, @1), $2); ast_adicionar($$, $3);
};
para: PER IDENT DE expressao ATE expressao bloco {
    $$ = NO(AST_PARA, $2, @1); free($2);
    ast_adicionar($$, $4); ast_adicionar($$, $6); ast_adicionar($$, $7);
};
expressao: ou { $$ = $1; };
ou: e { $$ = $1; } | ou OR e { $$ = OP("or", $1, $3, @2); };
e: nao { $$ = $1; } | e AND nao { $$ = OP("and", $1, $3, @2); };
nao: igualdade { $$ = $1; } | NOT nao { $$ = OP("not", $2, NULL, @1); };
igualdade: relacional { $$ = $1; }
    | relacional IGUAL relacional { $$ = OP("==", $1, $3, @2); }
    | relacional DIFERENTE relacional { $$ = OP("!=", $1, $3, @2); };
relacional: soma { $$ = $1; }
    | soma '>' soma { $$ = OP(">", $1, $3, @2); }
    | soma '<' soma { $$ = OP("<", $1, $3, @2); }
    | soma MAIOR_IGUAL soma { $$ = OP(">=", $1, $3, @2); }
    | soma MENOR_IGUAL soma { $$ = OP("<=", $1, $3, @2); };
soma: termo { $$ = $1; }
    | soma '+' termo { $$ = OP("+", $1, $3, @2); }
    | soma '-' termo { $$ = OP("-", $1, $3, @2); };
termo: unario { $$ = $1; }
    | termo '*' unario { $$ = OP("*", $1, $3, @2); }
    | termo '/' unario { $$ = OP("/", $1, $3, @2); };
unario: primario { $$ = $1; } | '-' unario { $$ = OP("-", $2, NULL, @1); };
primario:
    INT_LIT { $$ = NO(AST_INTEIRO, $1, @1); free($1); }
  | FLOAT_LIT { $$ = NO(AST_REAL, $1, @1); free($1); }
  | TEXTO_LIT { $$ = NO(AST_TEXTO, $1, @1); free($1); }
  | TRUE { $$ = NO(AST_BOOLEANO, "true", @1); }
  | FALSE { $$ = NO(AST_BOOLEANO, "false", @1); }
  | IDENT { $$ = NO(AST_IDENTIFICADOR, $1, @1); free($1); }
  | chamada { $$ = $1; } | conversao { $$ = $1; }
  | '(' expressao ')' { $$ = $2; };
chamada: IDENT '(' argumentos_opcionais ')' {
    $$ = ast_adicionar(NO(AST_CHAMADA, $1, @1), $3); free($1);
};
argumentos_opcionais: %empty { $$ = NO(AST_ARGUMENTOS, NULL, @$); } | argumentos { $$ = $1; };
argumentos: expressao { $$ = ast_adicionar(NO(AST_ARGUMENTOS, NULL, @1), $1); }
    | argumentos ',' expressao { $$ = ast_adicionar($1, $3); };
conversao: INTERO '(' expressao ')' { $$ = ast_adicionar(NO(AST_CONVERSAO, "intero", @1), $3); }
    | REALE '(' expressao ')' { $$ = ast_adicionar(NO(AST_CONVERSAO, "reale", @1), $3); };
%%
void yyerror(const char *mensagem) {
    ++erros_sintaticos;
    fprintf(stderr, "Erro sintatico linha %d, coluna %d: %s\n", yylloc.first_line, yylloc.first_column, mensagem);
}
