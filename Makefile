CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
BISON = bison
FLEX = flex
LEXER = lexer/lexer.l

.SUFFIXES:
.DEFAULT_GOAL := all

all: parser

parser: parser.tab.c parser.tab.h

parser.tab.c: parser.y ast.h
	$(BISON) -Wall -Werror=conflicts-sr -Werror=conflicts-rr -d -o parser.tab.c parser.y

parser.tab.h: parser.tab.c

lex.yy.c: $(LEXER) parser.tab.h
	$(FLEX) -o lex.yy.c $(LEXER)

$(LEXER):
	@echo "Lexer pendente: adicione o arquivo em $(LEXER). Consulte lexer/README.md."
	@exit 1

traduzi: parser.tab.c lex.yy.c ast.c ast.h main.c
	$(CC) $(CFLAGS) -o traduzi parser.tab.c lex.yy.c ast.c main.c

test: traduzi
	node tests/test.cjs

# ---- Testes isolados do analisador lexico (integrante 1) ----
lexer_test: parser.tab.c parser.tab.h lex.yy.c ast.c lexer/lexer_test.c
	$(CC) $(CFLAGS) -I. -o lexer_test parser.tab.c lex.yy.c ast.c lexer/lexer_test.c

test-lexer: lexer_test
	@set -e; \
	for f in lexer/testes/valido_*.trz; do \
		if ./lexer_test "$$f" > /dev/null; then echo "OK      $$f"; \
		else echo "FALHOU  $$f (deveria ser aceito)"; exit 1; fi; \
	done; \
	for f in lexer/testes/invalido_*.trz; do \
		if ./lexer_test "$$f" > /dev/null 2>&1; then echo "FALHOU  $$f (deveria dar erro lexico)"; exit 1; \
		else echo "OK      $$f (erro lexico detectado)"; fi; \
	done

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h traduzi traduzi.exe lexer_test lexer_test.exe

.PHONY: all parser test test-lexer clean
