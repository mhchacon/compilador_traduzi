# Analisador léxico da Traduzi (Flex)

O analisador léxico lê o texto de um programa Traduzi e o transforma em uma
sequência de **tokens**, que o Bison (`parser.y`) consome. Ele também ignora
espaços e comentários e avisa, com linha e coluna, quando encontra algo que não
pertence à linguagem.

```
arquivo .traduzi ──► Flex (lexer/lexer.l) ──tokens──► Bison (parser.y) ──► AST ──► semântica
```

## Arquivos desta pasta

| Arquivo | Para que serve |
| --- | --- |
| `lexer.l` | Especificação do Flex (o arquivo que se mantém). O `lex.yy.c` é gerado a partir dele e **não** vai para o Git. |
| `lexer_test.c` | Programa de teste isolado: imprime um token por linha. Não faz parte do executável `traduzi` (que usa o `main.c`). |
| `testes/valido_*.trz` | Programas que o lexer deve aceitar sem erro léxico. |
| `testes/invalido_*.trz` | Programas com erros léxicos que o lexer deve acusar. |
| `testes/*.esperado` | Saída esperada de cada teste (tokens com posição e mensagens de erro). |

## Como compilar e testar

Na raiz do projeto:

```sh
make traduzi          # gera lex.yy.c a partir de lexer/lexer.l e compila tudo
make test-lexer       # roda os testes do lexer e compara com os arquivos .esperado
make lexer_test       # só compila o programa de teste isolado
```

Para ver os tokens de qualquer programa:

```sh
./lexer_test exemplos/completo.traduzi
echo 'int x = 10 - 5;' | ./lexer_test          # também lê da entrada padrão
```

Cada linha da saída é `linha:coluna-linha:coluna  TOKEN  "texto"` (início e fim
do token; o fim é a coluna logo após o último caractere). Erros léxicos saem em
`stderr`; o programa retorna `0` sem erro léxico, `1` com erro e `2` se o arquivo
não abriu.

Se uma mudança nos tokens ou nas mensagens for **intencional**, regenere as
saídas esperadas com `make lexer-esperados` e confira o resultado com `git diff`.

## Tokens reconhecidos

Os nomes oficiais dos tokens são os declarados em `parser.y`.

### Palavras reservadas

Diferenciam maiúsculas de minúsculas (`Traduzi` é reservada, `traduzi` é um
identificador). Não podem ser usadas como identificadores.

| Categoria | Texto reconhecido | Token retornado |
| --- | --- | --- |
| Tipos | `int` `float` `string` `bool` `void` | `INT` `FLOAT` `STRING` `BOOL` `VOID` |
| Valores | `true` `false` | `TRUE` `FALSE` |
| Condicional | `in` `caso` `di` `succede` `altrimenti` | `IN` `CASO` `DI` `SUCCEDE` `ALTRIMENTI` |
| Repetição | `mentre` `per` `de` `ate` `fermarsi` `continuare` | `MENTRE` `PER` `DE` `ATE` `FERMARSI` `CONTINUARE` |
| Entrada e saída | `input` `stampa` | `INPUT` `STAMPA` |
| Ações | `acao` `ritorna` `Traduzi` | `ACAO` `RITORNA` `TRADUZI` |
| Variáveis e conversões | `mutabile` `intero` `reale` | `MUTABILE` `INTERO` `REALE` |
| Lógicos | `and` `or` `not` | `AND` `OR` `NOT` |

`in caso di` são **três** palavras reservadas separadas (`IN`, `CASO`, `DI`).

### Identificadores e literais

| Elemento | Forma | Token | Valor (`yylval.texto`) |
| --- | --- | --- | --- |
| Identificador | `(letra \| _) (letra \| dígito \| _)*`, sem acentos | `IDENT` | o nome |
| Inteiro | `[0-9]+` (ex.: `10`, `0`) | `INT_LIT` | o texto do número |
| Real | `[0-9]+.[0-9]+` (ex.: `3.14`, `0.5`) | `FLOAT_LIT` | o texto do número |
| Texto | `:` conteúdo `:` (sem `:` e sem quebra de linha dentro) | `TEXTO_LIT` | o conteúdo, **sem** os dois-pontos |
| Booleano | `true`, `false` | `TRUE`, `FALSE` | — |

- `idade`, `Idade` e `IDADE` são três identificadores diferentes.
- Não existe literal negativo: `-5` é o operador `-` seguido do inteiro `5`.
- Números preservam a representação original (o lexer não converte nem verifica a faixa do `int`).
- Textos aceitam espaços e caracteres acentuados.

### Operadores e delimitadores

| Símbolos | Retorno |
| --- | --- |
| `+` `*` `/` `<` `>` `=` `(` `)` `[` `]` `,` `;` | o próprio caractere (`'+'`, `'*'`, ...) |
| `>=` `<=` `==` `!=` | `MAIOR_IGUAL` `MENOR_IGUAL` `IGUAL` `DIFERENTE` |
| `-` | `'-'` (subtração, negativo unário ou hífen que **abre** a condição) |
| `-` que **fecha** a condição | `FIM_CONDICAO` |

Quando há mais de uma possibilidade vale o **token mais longo**: `>=` é um token
só, e não `>` seguido de `=`.

### Espaços e comentários (ignorados)

| Elemento | Forma |
| --- | --- |
| Espaços, tabs, quebras de linha (inclusive CRLF) | separam tokens e são descartados |
| Comentário de uma linha | `~>` até o fim da linha |
| Comentário de várias linhas | `~{ ... }~` (não aninha) |

O BOM UTF-8 no início do arquivo (gravado por alguns editores do Windows) é ignorado.

## A regra do hífen

O `-` tem três usos, e o lexer resolve o que consegue olhando adiante:

1. logo depois de `di` ou `mentre`: **abre** a condição;
2. logo antes de `succede` ou de `[` (ignorando espaços e comentários no meio): **fecha** a condição;
3. qualquer outro: subtração ou sinal negativo.

O lexer só precisa reconhecer o caso 2: devolve `FIM_CONDICAO`. Todos os outros
hifens voltam como `'-'`, e o Bison decide pela posição na gramática
(`condicao: '-' expressao FIM_CONDICAO`).

Exemplo: em `in caso di -a - b > 0- succede [`, o primeiro `-` abre (`'-'`), o
segundo é subtração (`'-'`) e o último fecha (`FIM_CONDICAO`).

## Erros léxicos

Em todo erro o lexer: (1) escreve em `stderr` no formato
`Erro lexico linha L, coluna C: mensagem`; (2) incrementa `erros_lexicos`;
(3) devolve o token `INVALIDO` ao parser e **continua lendo**, para achar vários
erros de uma vez. O `main.c` não imprime a AST quando `erros_lexicos > 0` e o
programa termina com falha: se há erro na tradução, nada é gerado.

| Situação | Exemplo | Mensagem (resumo) |
| --- | --- | --- |
| Identificador começando com número | `int 2idade = 20;` | identificador nao pode comecar com numero |
| Real sem dígitos depois do ponto | `5.` | numero real invalido: faltam digitos depois do ponto |
| Real sem dígitos antes do ponto | `.5` | numero real invalido: faltam digitos antes do ponto |
| Real colado em letras | `3.14abc` | numero real mal formado |
| Texto sem `:` final | `:texto sem fim` | texto sem delimitador de fechamento ':' na mesma linha |
| Comentário de bloco aberto | `~{ nunca fecha` | comentario de varias linhas nao foi fechado (aponta onde começou) |
| `}~` sem `~{` | `}~` | '}~' sem um '~{' correspondente |
| Caractere inválido | `@` | caractere invalido '@' |
| Operadores de outras linguagens | `!` `&` `\|` | dica: use `not`/`!=`, `and`, `or` |
| Aspas e chaves | `"` `'` `{` `}` | dica: textos usam `:`; blocos usam `[` `]` |
| `~` isolado | `~` | explica `~>` e `~{ ... }~` |
| Caractere não ASCII fora de texto | `ç`, `–` (travessão), NBSP, aspas “ ” | mostra o código Unicode (U+XXXX) e, nos casos comuns de copiar/colar, diz o que trocar |
| Byte que não é UTF-8 | — | byte invalido 0x.. (o arquivo deve estar em UTF-8) |

Observação: como o lexer devolve `INVALIDO`, o parser costuma acrescentar um erro
sintático logo em seguida (`unexpected INVALIDO`). É esperado.

## Interface com o Bison

O lexer faz `#include "parser.tab.h"` (gerado por `make` a partir de `parser.y`).
Não mude nome de token, tipo de valor ou campo da AST sem avisar quem depende
dele e atualizar os testes.

- Fornece `int yylex(void)`, `int erros_lexicos` (começa em 0) e usa `FILE *yyin`
  e `int yylex_destroy(void)` do Flex; **não** define `main`.
- Retorna `0` no fim do arquivo.
- `yylval.texto`: para `IDENT`, `INT_LIT`, `FLOAT_LIT` e `TEXTO_LIT`, uma cópia
  alocada com `malloc`. **O parser passa a ser o dono** da memória e a libera
  (`free` nas ações e `%destructor` no `parser.y`). `TRADUZI`, `TRUE` e `FALSE`
  não carregam valor.
- `yylloc`: `first_line`, `first_column`, `last_line` e `last_column`
  preenchidos em todo token, inclusive no fim do arquivo, contando a partir de 1.
  Colunas contam caracteres (não bytes), então acentos não desalinham.

## Como acrescentar ou mudar um token

1. Declare o token em `parser.y` (`%token`) e use-o na gramática.
2. Em `lexer.l`, acrescente a regra. Palavras reservadas ficam **antes** da regra
   de identificador (em empate vale a regra que aparece primeiro).
3. Acrescente o `case` correspondente na função `nome()` de `lexer_test.c`.
4. Crie ou ajuste um arquivo em `testes/`, rode `make lexer-esperados`, confira
   o `git diff` e rode `make test-lexer`.
5. Atualize a tabela de tokens deste arquivo.

## Testes

Todos passam com `make test-lexer`.

| Arquivo | O que cobre |
| --- | --- |
| `valido_calculo.trz` | declarações, expressões e precedência |
| `valido_acoes.trz` | ações com parâmetros, retorno e chamadas |
| `valido_condicional.trz` | `in caso di`, `succede`, `altrimenti` encadeado |
| `valido_lacos.trz` | `mentre` e `per`, `fermarsi` e `continuare`, sinal negativo unário |
| `valido_hifen.trz` | os três usos do hífen, inclusive com comentários antes do fechamento |
| `valido_comentarios_textos.trz` | comentários de uma e várias linhas, textos com espaços e acentos, reais |
| `valido_maiusculas.trz` | maiúsculas e minúsculas diferem (`idade`/`Idade`/`IDADE`, `Int`, `traduzi`) e identificador que começa com palavra reservada (`succede_x`, sem confundir com o hífen de fechamento) |
| `valido_colunas.trz` | colunas corretas depois de textos com acento |
| `valido_todos_tokens.trz` | todos os tokens da linguagem; também é um programa válido para o `traduzi` completo |
| `invalido_identificador.trz` | identificador começando com número |
| `invalido_reais.trz` | `.5`, `5.`, `3.14abc` |
| `invalido_texto.trz` | texto sem `:` de fechamento |
| `invalido_comentario.trz` | comentário de bloco não fechado |
| `invalido_caracteres.trz` | `@`, `!`, `&`, `\|`, aspas, chaves, acentos em identificador |
| `invalido_utf8.trz` | travessão, espaço não separável, aspas tipográficas, `ç` e emoji fora de texto |
| `invalido_lexico.trz` | mistura de vários erros e `}~` solto |

## Limitações e decisões

- Textos não podem conter `:` nem quebra de linha (regra da linguagem). Um texto
  sem fechamento consome até o fim da linha.
- Comentários de várias linhas não aninham.
- O lexer não verifica a faixa dos inteiros; quem converte o texto em número
  (semântica ou gerador de código) deve tratar valores grandes.
- O "olhar adiante" do hífen só ignora espaços e comentários entre o `-` e
  `succede`/`[`.
- O arquivo de entrada deve estar em UTF-8 (CRLF e BOM são aceitos).
