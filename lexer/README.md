Espaco para integrar o analisador lexico do integrante responsavel.

Adicione aqui o arquivo `lexer.l`. O lexer de teste foi removido.

Esta pasta fica reservada ao integrante 1. Nao ha um lexer provisório para
substituir sua implementacao. O `Makefile` ja gera `lex.yy.c` a partir desse
arquivo e o compila junto com o parser, a AST e o programa principal.

Contrato com o parser

- Incluir `parser.tab.h`, gerado por `make` a partir de `parser.y`.
- Fornecer `int yylex(void)`, `FILE *yyin`, `int yylex_destroy(void)` e `int erros_lexicos`, inicializado com zero.
- Retornar os tokens declarados em `parser.y` e zero no fim do arquivo.
- Retornar caracteres para `+ - * / > < = ( ) [ ] , ;` e os tokens nomeados para operadores compostos.
- Para `IDENT`, `INT_LIT`, `FLOAT_LIT` e `TEXTO_LIT`, preencher `yylval.texto` com memoria alocada dinamicamente, por exemplo usando `ast_texto`. O parser assume a propriedade dessa memoria. Textos devem vir sem os dois-pontos delimitadores. Numeros devem preservar sua representacao textual.
- Preencher `yylloc.first_line`, `first_column`, `last_line` e `last_column` em cada token, incluindo o fim do arquivo, com contagem iniciando em 1.
- Retornar `FIM_CONDICAO` para o hifen que fecha uma condicao antes de `succede` ou `[`, ignorando espacos e comentarios intermediarios. Os demais hifens retornam `'-'`.
- Reconhecer `in`, `caso` e `di` como tres tokens separados e `Traduzi` como o token reservado `TRADUZI`.
- Ignorar comentarios `~>` de uma linha e `~{ ... }~` de varias linhas.
- Em erro lexico, incrementar `erros_lexicos`, emitir diagnostico com linha e coluna e retornar `INVALIDO`.
- Usar `%option noyywrap never-interactive` ou fornecer as funcoes e configuracoes equivalentes. O executavel integrado ja possui `main` em `main.c`; o lexer nao deve definir outro.

Mapa de tokens para a integracao (os nomes oficiais estao em `parser.y`):

| Texto reconhecido | Token retornado |
| --- | --- |
| `acao`, `Traduzi`, `ritorna`, `void`, `mutabile` | `ACAO`, `TRADUZI`, `RITORNA`, `VOID`, `MUTABILE` |
| `int`, `float`, `string`, `bool` | `INT`, `FLOAT`, `STRING`, `BOOL` |
| `true`, `false` | `TRUE`, `FALSE` |
| `in`, `caso`, `di`, `succede`, `altrimenti` | `IN`, `CASO`, `DI`, `SUCCEDE`, `ALTRIMENTI` |
| `mentre`, `per`, `de`, `ate` | `MENTRE`, `PER`, `DE`, `ATE` |
| `fermarsi`, `continuare`, `input`, `stampa` | `FERMARSI`, `CONTINUARE`, `INPUT`, `STAMPA` |
| `intero`, `reale`, `and`, `or`, `not` | `INTERO`, `REALE`, `AND`, `OR`, `NOT` |
| `>=`, `<=`, `==`, `!=` | `MAIOR_IGUAL`, `MENOR_IGUAL`, `IGUAL`, `DIFERENTE` |
| Identificador, inteiro, real, texto entre dois-pontos | `IDENT`, `INT_LIT`, `FLOAT_LIT`, `TEXTO_LIT` |

Por exemplo, em `mentre -x - 1 > 0- []`, o primeiro hifen abre a
condicao, o segundo e subtracao (ambos retornam `'-'`) e o ultimo retorna
`FIM_CONDICAO`. Esse acordo precisa ser respeitado pelo lexer para que a
gramatica atual reconheca as condicoes.

O modo de teste isolado do integrante 1 pode imprimir tokens, mas o `yylex`
integrado precisa retorna-los ao Bison. Se houver um `main` para testes lexicos,
ele deve ficar em um programa separado do executavel `traduzi`.

Depois de adicionar o lexer:

```sh
make traduzi
./traduzi exemplos/completo.traduzi
make test
```

Os testes existentes verificam o conjunto lexer e parser e so podem ser executados depois da integracao.
