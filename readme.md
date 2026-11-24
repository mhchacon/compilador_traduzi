# Compilador Traduzi

## Entrega Bison e AST (Pessoa 2)

A gramática executável está em `parser.y`; a especificação formal está em
[`gramatica.ebnf`](gramatica.ebnf). Edite `parser.y` e gere os arquivos
`parser.tab.c` e `parser.tab.h` com `make parser`, em vez de editar os gerados.
O Bison usa `%expect 0`, e a compilação trata conflitos como erros.

Precedência, da menor para a maior: `or`, `and`, `not`, `== !=`,
`> < >= <=`, `+ -`, `* /`, menos unário e primários. Operadores binários
repetíveis associam à esquerda; `not` e menos unário associam à direita.
Comparações do mesmo nível não encadeiam sem parênteses. A precedência é
expressa pelos níveis da gramática, sem depender de resolução de conflitos.
O contrato de tokens, valores e localizações está em `lexer/README.md`.

### Contrato da AST com as etapas seguintes

O parser entrega `AST *ast_raiz`. Cada nó preserva `tipo`, `valor`,
`tipo_dado`, `mutavel`, `linha`, `coluna` e um vetor ordenado `filhos`.
Nomes, literais e operadores ficam em `valor`; declarações, parâmetros e
ações guardam seu tipo em `tipo_dado`. As posições começam em 1.

| Nó | Ordem dos filhos |
| --- | --- |
| Programa | Ações na ordem do código |
| Ação | Lista de parâmetros, bloco |
| Parâmetros / argumentos / bloco | Elementos na ordem do código |
| Declaração | Inicializador opcional |
| Atribuição / saída / conversão | Expressão |
| Condicional | Condição, bloco verdadeiro, alternativa opcional (bloco ou condicional) |
| Enquanto | Condição, bloco |
| Para | Início, fim, bloco; nome do contador em `valor` |
| Retorno | Expressão opcional |
| Chamada | Lista de argumentos (também quando vazia) |
| Operação | Operando esquerdo e direito; unárias têm um filho |
| Entrada / parâmetro / identificador / literal / parar / continuar | Sem filhos |

`ast_criar` copia o texto recebido. As ações do parser liberam os valores
textuais do lexer após copiá-los; os tipos alocados são transferidos ao nó.
`ast_adicionar` transfere a propriedade do filho ao pai e ignora filhos nulos.
`ast_liberar` libera recursivamente filhos, textos e tipos. Os `%destructor`
liberam símbolos descartados durante erros. O chamador deve liberar a raiz
mesmo após erros recuperáveis e impedir o uso da AST se houver erros.

O parser informa linha/coluna e tokens esperados com `parse.error detailed`
e LAC. A recuperação sincroniza em `;` e `]` para detectar mais de um erro.
Restrições como variável declarada, tipos e uso de `fermarsi` em laços são
verificadas pela análise semântica, que já recebe esta mesma `AST`.

### Testes e evidência de árvore

`make test-parser` compila um executável separado com o Flex real, o Bison
e a AST, sem a análise semântica. São 31 verificações de sintaxe, precedência,
associatividade, posições e recuperação. `make test` executa os 35 testes
integrados. Ambos passaram na validação desta entrega com GCC e Bison do MSYS2.

Por exemplo, `acao Traduzi() void [ int x = 10 + 5 * 2; ]` produz:

```text
programa (1:1)
  acao Traduzi tipo=void (1:1)
    parametros (1:14)
    bloco (1:21)
      declaracao x tipo=int (1:23)
        operacao + (1:34)
          inteiro 10 (1:31)
          operacao * (1:38)
            inteiro 5 (1:36)
            inteiro 2 (1:40)
```

Pendência para a Pessoa 3: `codegen_python.c` ainda recebe a estrutura
legada `ASTNode`, não a `AST` produzida pelo parser, e não participa do
executável principal. É necessário adaptar o gerador ao contrato acima e
integrá-lo para concluir a tradução Python.

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
