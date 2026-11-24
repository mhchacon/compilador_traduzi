# Compilador Traduzi

Compilador da linguagem **Traduzi**, implementado em C com Flex e Bison.
O projeto executa quatro etapas:

1. análise léxica;
2. análise sintática;
3. construção e análise da AST;
4. geração opcional de código Python.

O programa só imprime a AST ou gera Python quando o código-fonte passa pela
análise léxica, sintática e semântica.

## Requisitos

Instale e deixe disponíveis no `PATH`:

- GCC;
- Make;
- Flex;
- Bison;
- Node.js, usado pelos testes.

No Windows, recomenda-se o [MSYS2](https://www.msys2.org/) com o ambiente
MinGW. Os comandos `gcc`, `make`, `flex`, `bison` e `node` devem funcionar no
terminal escolhido.

## Compilação

Na raiz do projeto:

```sh
make traduzi
```

O alvo:

1. gera `parser.tab.c` e `parser.tab.h` a partir de `parser.y`;
2. gera `lex.yy.c` a partir de `lexer/lexer.l`;
3. compila o tradutor com a AST, a análise semântica, a tabela de símbolos e
   o gerador Python.

Para gerar somente o parser:

```sh
make parser
```

Os arquivos `parser.tab.c`, `parser.tab.h` e `lex.yy.c` são gerados. Altere
`parser.y` ou `lexer/lexer.l`, nunca os arquivos gerados diretamente.

## Uso

### Imprimir a AST

Linux/MSYS2:

```sh
./traduzi exemplos/completo.traduzi
```

Windows PowerShell:

```powershell
.\traduzi.exe exemplos\completo.traduzi
```

Sem arquivo, o código é lido da entrada padrão:

```sh
echo "acao Traduzi() void []" | ./traduzi
```

```powershell
'acao Traduzi() void []' | .\traduzi.exe
```

### Gerar Python

Use `--python` para enviar o programa traduzido para a saída padrão:

```powershell
.\traduzi.exe --python exemplos\funcoes.traduzi > saida.py
python saida.py
```

Também é possível usar a entrada padrão:

```sh
echo "acao Traduzi() void [ stampa(:Ola:); ]" | ./traduzi --python
```

O formato da linha de comando é:

```text
traduzi [--python] [arquivo.traduzi]
```

Com `--python`, o gerador produz funções Python, chamadas, expressões,
condicionais, laços, entrada, saída, retornos, conversões e o bloco:

```python
if __name__ == '__main__':
    Traduzi()
```

Declarações sem inicializador são traduzidas para uma variável com valor
`None`. Entradas são convertidas para `int`, `float` ou `bool` quando o tipo
da variável é conhecido; textos permanecem como `str`.

## Sintaxe da linguagem

Um programa contém uma ou mais ações. `Traduzi` é a ação principal e deve ser
`void`:

```traduzi
acao soma(int a, int b) ritorna int [
    ritorna a + b;
]

acao Traduzi() void [
    int resultado = soma(2, 3);
    stampa(resultado);
]
```

Tipos disponíveis:

```text
int       inteiros
float     números reais
string    textos entre dois-pontos
bool      true ou false
void      retorno de ações sem valor
```

Declarações e atribuições:

```traduzi
int idade;
mutabile int contador = 0;
float media = 7.5;
string nome = :Mariana:;
bool ativo = true;

idade = 18;
input(nome);
stampa(nome);
```

Uma variável sem `mutabile` pode receber uma atribuição posterior caso ainda
não tenha sido inicializada. Depois de inicializada, ela não pode ser alterada.

Condicionais:

```traduzi
in caso di -idade >= 18- succede [
    stampa(:Maior:);
] altrimenti in caso di -idade >= 12- succede [
    stampa(:Adolescente:);
] altrimenti [
    stampa(:Crianca:);
]
```

Laços:

```traduzi
mentre -contador < 3- [
    contador = contador + 1;
    continuare;
]

per i de -1 ate 3 [
    stampa(i);
    fermarsi;
]
```

`ate` é inclusivo no laço `per`. `fermarsi` e `continuare` só podem ser usados
dentro de um laço.

Retornos, chamadas e conversões:

```traduzi
acao dobro(int valor) ritorna int [
    ritorna valor + valor;
]

acao Traduzi() void [
    int valor = dobro(4);
    float real = reale(valor);
    int inteiro = intero(real);
    ritorna;
]
```

Operadores, da menor para a maior precedência:

```text
or
and
not
== !=
> < >= <=
+ -
* /
- unário
primários e expressões entre parênteses
```

Operadores binários associam à esquerda. `not` e o menos unário associam à
direita. Comparações não podem ser encadeadas sem parênteses.

Comentários:

```traduzi
~> comentário até o fim da linha
~{ comentário de várias linhas }~
```

Textos usam dois-pontos, por exemplo `:Olá, mundo:`. Identificadores aceitam
letras sem acento, números, `_` e não podem começar por número.

## Análise semântica

A tabela de símbolos mantém escopos aninhados e registra:

- nome;
- tipo;
- mutabilidade;
- inicialização;
- se o símbolo representa uma ação.

A análise semântica verifica:

- existência e unicidade da ação `Traduzi`;
- declarações duplicadas no mesmo escopo;
- uso de variáveis declaradas e inicializadas;
- atribuições a variáveis imutáveis;
- compatibilidade de tipos;
- tipos e quantidade de argumentos de chamadas;
- tipos de condições;
- limites inteiros de `per`;
- tipo de retorno;
- uso de `fermarsi` e `continuare` dentro de laços.

Erros são enviados para `stderr` no formato:

```text
Erro lexico linha L, coluna C: mensagem
Erro sintatico linha L, coluna C: mensagem
Erro semantico linha L, coluna C: mensagem
```

O processo termina com código diferente de zero quando ocorre qualquer erro.

## AST

O parser entrega `AST *ast_raiz`. Cada nó preserva:

- `tipo`;
- `valor`;
- `tipo_dado`;
- `mutavel`;
- `linha` e `coluna`;
- vetor ordenado de `filhos`.

A ordem dos filhos é:

| Nó | Filhos |
| --- | --- |
| Programa | ações |
| Ação | parâmetros, bloco |
| Declaração | inicializador opcional |
| Atribuição, saída e conversão | expressão |
| Condicional | condição, bloco verdadeiro, alternativa opcional |
| Enquanto | condição, bloco |
| Para | início, fim, bloco |
| Retorno | expressão opcional |
| Chamada | lista de argumentos |
| Operação | operandos; unárias têm um filho |

`ast_adicionar` transfere a propriedade do filho para o pai. `ast_liberar`
libera recursivamente filhos, textos e tipos. A AST não deve ser utilizada
quando o parser ou o lexer registrou erros.

## Testes

Compile antes de executar os testes:

```sh
make traduzi
```

Suíte integrada:

```sh
make test
```

Somente parser, precedência, posições e recuperação de erros:

```sh
make test-parser
```

Somente lexer:

```sh
make test-lexer
```

Os testes isolados do lexer usam os arquivos em `lexer/testes/`. Para
regenerar as saídas esperadas, depois de conferir que a alteração é
intencional:

```sh
make lexer-esperados
```

Para remover executáveis e arquivos gerados:

```sh
make clean
```

## Estrutura do projeto

- [`lexer/lexer.l`](lexer/lexer.l): analisador léxico;
- [`lexer/README.md`](lexer/README.md): contrato dos tokens e posições;
- [`parser.y`](parser.y): gramática Bison e construção da AST;
- [`gramatica.ebnf`](gramatica.ebnf): especificação formal da gramática;
- [`ast.c`](ast.c) e [`ast.h`](ast.h): estrutura e gerenciamento da AST;
- [`semantic.c`](semantic.c) e [`semantic.h`](semantic.h): análise semântica;
- [`symbol_table.c`](symbol_table.c) e [`symbol_table.h`](symbol_table.h):
  tabela de símbolos;
- [`codegen_python.c`](codegen_python.c) e
  [`codegen_python.h`](codegen_python.h): geração de Python;
- [`main.c`](main.c): entrada do programa e seleção entre AST e Python;
- [`Makefile`](Makefile): geração, compilação, testes e limpeza;
- [`exemplos/`](exemplos/): programas válidos e casos de erro.

## Limpeza e regeneração

Quando `parser.y` ou `lexer/lexer.l` for alterado, regenere e compile com:

```sh
make clean
make traduzi
```

Não versionar executáveis nem arquivos temporários gerados localmente.
