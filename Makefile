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

.PHONY: all parser test
