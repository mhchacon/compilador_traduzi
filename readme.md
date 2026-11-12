# Compilador Traduzi

Compilador da linguagem **Traduzi**, implementado em C. O programa executa
as etapas de análise léxica, sintática e semântica e imprime a árvore sintática
abstrata (AST) quando o código é válido.

## Pré-requisitos

Instale e deixe disponíveis no `PATH`:

- GCC;
- Make;
- Flex;
- Bison;
- Node.js (necessário para executar os testes).

No Windows, uma opção é usar o [MSYS2](https://www.msys2.org/) com o ambiente
MinGW. Os comandos abaixo devem ser executados em um terminal que tenha
`gcc`, `make`, `flex`, `bison` e `node` disponíveis.

## Como compilar

Na raiz do projeto, execute:

```sh
make traduzi
```

Esse comando:

1. gera `parser.tab.c` e `parser.tab.h` a partir de `parser.y` usando o
   Bison;
2. gera `lex.yy.c` a partir de `lexer/lexer.l` usando o Flex;
3. compila os arquivos C, incluindo a análise semântica e a tabela de
   símbolos, e cria o executável `traduzi` (ou `traduzi.exe` no Windows).

Para gerar apenas os arquivos do parser, use:

```sh
make parser
```

## Como executar

### Traduzir um arquivo

Passe um arquivo `.traduzi` como argumento:

```sh
./traduzi exemplos/completo.traduzi
```

No Windows, o equivalente é:

```powershell
.\traduzi.exe exemplos\completo.traduzi
```

Se o programa for válido nas três análises, a AST será impressa no terminal.
Em caso de erro léxico, sintático ou semântico, o diagnóstico será enviado
para a saída de erro e o processo terminará com falha.

### Ler o código pela entrada padrão

Também é possível executar o tradutor sem informar um arquivo e fornecer o
código pelo terminal:

```sh
echo "acao Traduzi() void []" | ./traduzi
```

No PowerShell:

```powershell
'acao Traduzi() void []' | .\traduzi.exe
```

O executável aceita no máximo um arquivo de entrada:

```text
Uso: traduzi [arquivo.traduzi]
```

### Exemplos adicionais

Exemplos válidos:

```sh
./traduzi exemplos/basico.traduzi
./traduzi exemplos/funcoes.traduzi
./traduzi exemplos/lacos.traduzi
```

Exemplos que devem produzir erro semântico:

```sh
./traduzi exemplos/erro_tipo.traduzi
./traduzi exemplos/erro_variavel.traduzi
./traduzi exemplos/erro_laco.traduzi
```

## Testes

Depois de compilar o executável, execute a suíte de testes do parser e do
lexer:

```sh
make test
```

Para executar somente os testes isolados do analisador léxico:

```sh
make test-lexer
```

Os testes isolados usam os arquivos em `lexer/testes/`. Para remover os
arquivos gerados e os executáveis:

```sh
make clean
```

## Estrutura e fluxo

1. O **Flex** converte o código-fonte em tokens.
2. O **Bison** valida a gramática e constrói a AST.
3. A análise semântica valida declarações, tipos, escopos, atribuições,
   chamadas, retornos e comandos de laço.
4. O programa principal imprime a AST somente quando não há erros.

Arquivos principais:

- `parser.y`: gramática e regras de construção da AST;
- `lexer/lexer.l`: analisador léxico;
- `ast.c` e `ast.h`: estruturas e operações da AST;
- `semantic.c` e `semantic.h`: análise semântica;
- `symbol_table.c` e `symbol_table.h`: tabela de símbolos e escopos;
- `main.c`: entrada do programa e tratamento do arquivo de entrada;
- `Makefile`: regras de geração, compilação, testes e limpeza;
- `exemplos/completo.traduzi`: exemplo completo de programa Traduzi.
