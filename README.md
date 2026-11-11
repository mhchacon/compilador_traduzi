Entrega do integrante 2: analisador sintatico da Traduzi com Bison e AST em C11.

Estado da integracao: o analisador lexico do integrante 1 ainda esta pendente.
Nao existe arquivo Flex ativo nesta entrega. O local reservado para recebe-lo e
`lexer/lexer.l`; as regras lexicas devem ser desenvolvidas pelo integrante 1.
Sem esse arquivo, a compilacao do executavel e os testes integrados ficam
indisponiveis. Isso, por si so, nao demonstra um defeito em `ast.c` ou na gramatica.

Arquivos desta etapa:

- `parser.y`: gramatica, precedencia, construcao da AST e diagnosticos sintaticos.
- `ast.h` e `ast.c`: estrutura, criacao, impressao e liberacao da AST.
- `main.c`: execucao do parser e exibicao da AST para conferir a analise sintatica.
- `Makefile`: geracao do parser e futura compilacao com o lexer.
- `exemplos/` e `tests/`: entradas e verificacoes da analise sintatica.
- `lexer/README.md`: contrato de integracao com o integrante 1, sem implementar o lexer.

Nao ha tabela de simbolos, verificacao semantica, inferencia de tipos ou geracao de codigo Python/C nesta entrega. Os campos `tipo_dado` e `mutavel` da AST apenas registram o que foi escrito na declaracao; nao verificam tipos nem imutabilidade. Nos de conversao, retorno, parada e continuacao representam a sintaxe e ficam disponiveis para o integrante 3 analisar.

Requisitos para gerar o parser: Bison 3.8 ou superior e Make. A compilacao integrada tambem requer o lexer do outro integrante, Flex e GCC. Os testes utilizam Node.js.

```sh
make
make traduzi
./traduzi exemplos/completo.traduzi
make test
```

`make` gera apenas o parser e o cabecalho de tokens, sem depender do lexer. Os demais comandos ficam disponiveis depois que o outro integrante adicionar `lexer/lexer.l`, conforme `lexer/README.md`. Para outro caminho, use `make traduzi LEXER=caminho/arquivo.l`.

O programa integrado tambem aceita codigo pela entrada padrao. A AST e impressa apenas quando nao existem erros lexicos ou sintaticos. O codigo de saida e zero para sucesso e diferente de zero para erro.

`parser.y` define a gramatica e constroi os nos. O lexer sera integrado pelo outro integrante em `lexer/lexer.l`. `ast.h` e `ast.c` disponibilizam a estrutura, a impressao e a liberacao da arvore. `ast_raiz` pode ser percorrida pelos modulos de analise semantica e geracao de codigo antes de ser liberada.

Os filhos preservam a ordem do codigo. Acoes possuem parametros e bloco; declaracoes possuem um inicializador opcional; condicionais possuem condicao, bloco verdadeiro e alternativa opcional; enquanto possui condicao e bloco; para possui limite inicial, limite final e bloco. Operacoes binarias possuem operando esquerdo e direito; unarias possuem um operando. Chamadas possuem uma lista de argumentos. Tipos declarados, mutabilidade, nomes, literais e localizacoes sao preservados nos nos.

A precedencia segue os niveis da EBNF: sinal negativo, multiplicacao e divisao, soma e subtracao, relacionais, igualdade, not, and e or. Comparacoes do mesmo nivel nao podem ser encadeadas. Erros sao recuperados em ponto e virgula ou fechamento de bloco e impedem a exibicao da AST.

A existencia e unicidade da acao Traduzi, sua assinatura, verificacao de tipos, escopos, retornos, imutabilidade e uso de fermarsi e continuare dentro de lacos pertencem a etapa semantica, conforme a documentacao. Esta implementacao cobre a etapa sintatica e a construcao da AST.
