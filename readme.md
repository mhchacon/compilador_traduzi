Análise Semântica e Geração de Código: Linguagem Traduzi

Este documento explica os componentes e a lógica por trás da implementação em C da Etapa 3 (Análise Semântica) e Etapa 4 (Geração de Código Python) do compilador da linguagem Traduzi.

O fluxo do compilador após a Análise Sintática (feita pelo Bison) baseia-se em percorrer a Árvore Sintática Abstrata (AST).

1. A Estrutura Base: Árvore Sintática Abstrata (AST)

O arquivo ast.h define a estrutura fundamental que conecta o analisador sintático às próximas fases.

NodeType e DataType: Ajudam a classificar o que é cada nó (um laço, uma condicional, uma declaração) e o tipo de dado que ele carrega (inteiro, float, bool).

Campos de Dados: O nó guarda valores específicos (int_value, string_value, name) dependendo do seu tipo.

Ponteiros de Navegação (left, right, next): Permitem montar a árvore. O next é especialmente útil para encadear listas de comandos dentro de um bloco ou listas de parâmetros, enquanto left e right guardam as expressões ou condições de um comando.

2. A Tabela de Símbolos (symbol_table.h / .c)

A tabela de símbolos atua como a "memória" do compilador durante a análise semântica. Ela registra todas as variáveis e ações declaradas.

Gerenciamento de Escopo: Implementado como uma pilha (usando ponteiros parent). Cada vez que o código entra em um bloco [...] (seja de uma ação, condicional ou laço), enter_scope() é chamado. Ao sair, exit_scope() limpa as variáveis daquele bloco, garantindo que variáveis locais não vazem para o escopo externo.

Registro de Propriedades: A estrutura Symbol guarda o nome, o tipo, se a variável já recebeu valor (is_initialized) e se foi declarada com a palavra-chave mutabile (is_mutable).

Busca (lookup_symbol): Procura uma variável no escopo atual e, se não achar, sobe para os escopos pais até chegar ao global.

3. A Análise Semântica (semantic.h / .c)

O papel do analisador semântico é garantir que o programa faça sentido lógico e respeite as regras da Traduzi descritas no documento de especificação, mesmo que a sintaxe esteja correta.

Ele percorre a AST recursivamente (analyze_node) executando as seguintes validações:

Ação Principal Obrigatória: Ao analisar a raiz do programa, verifica se existe uma ação chamada Traduzi declarada sem parâmetros e com retorno void.

Declaração e Inicialização: Impede o uso de variáveis não declaradas ou o uso antes de receberem um valor inicial.

Checagem de Tipos: Verifica se operações matemáticas não estão misturando textos com números, e se as condições do in caso di e mentre são do tipo booleano (T_BOOL).

Regra de Imutabilidade: Bloqueia reatribuições (nó NODE_ASSIGN) a variáveis que não possuem a flag mutabile.

Controle de Fluxo: Usa um contador (in_loop) para garantir que os comandos fermarsi (break) e continuare (continue) só sejam chamados de dentro de um laço de repetição.

Se qualquer regra for violada, a função report_error exibe a mensagem de diagnóstico, incrementa o contador de erros e impede a geração de código.

4. O Gerador de Código Python (codegen_python.c)

Se a função check_semantics retornar sucesso (zero erros), o compilador chama a geração de código. Python foi escolhido como o primeiro destino.

O gerador faz uma nova travessia na AST validada:

Tradução Direta de Comandos: Nós como NODE_PRINT viram print(), NODE_WHILE viram while, e assim por diante.

Controle de Indentação: Ao contrário de C ou da própria Traduzi (que usa []), blocos em Python dependem de espaços. A variável global indent_level aumenta ao entrar em um bloco e diminui ao sair. A função print_indent() escreve os espaços necessários antes de cada linha.

Laço per: Traduzido para um for do Python usando a função range(). Como a linguagem Traduzi define limites inclusivos, o gerador adiciona + 1 ao limite superior do range.

Chamada Principal: Ao final do arquivo Python gerado, o código insere automaticamente a chamada condicional if __name__ == '__main__': Traduzi(), que é o padrão Python para iniciar a execução.

5. Integração

O fluxo de vida do seu programa main.c será:

Lex (Análise Léxica): Converte texto em Tokens.

Bison (Análise Sintática): Consome Tokens, valida a gramática e constrói a variável global root_ast.

Módulo Semântico: Avalia check_semantics(root_ast).

Módulo Gerador: Se aprovado, executa generate_python(root_ast) imprimindo o código final na tela (ou em um arquivo .py).