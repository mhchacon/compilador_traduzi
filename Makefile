CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
BISON = bison
FLEX = flex
LEXER = lexer/lexer.l

.SUFFIXES:
.DEFAULT_GOAL := all

all: traduzi

parser: parser.tab.c parser.tab.h

parser.tab.c: parser.y ast.h
	$(BISON) -Wall -Werror=conflicts-sr -Werror=conflicts-rr -d -o parser.tab.c parser.y

parser.tab.h: parser.tab.c

lex.yy.c: $(LEXER) parser.tab.h
	$(FLEX) -o lex.yy.c $(LEXER)

$(LEXER):
	@echo "Lexer pendente: adicione o arquivo em $(LEXER). Consulte lexer/README.md."
	@exit 1

traduzi: parser.tab.c lex.yy.c ast.c ast.h semantic.c semantic.h symbol_table.c symbol_table.h main.c
	$(CC) $(CFLAGS) -o traduzi parser.tab.c lex.yy.c ast.c semantic.c symbol_table.c main.c

test: traduzi
	node tests/test.cjs

parser_test: parser.tab.c parser.tab.h lex.yy.c ast.c ast.h tests/parser_main.c
	$(CC) $(CFLAGS) -I. -o parser_test parser.tab.c lex.yy.c ast.c tests/parser_main.c

test-parser: parser_test
	node tests/parser.cjs

# ---- Testes isolados do analisador lexico (integrante 1) ----
# Cada lexer/testes/X.trz tem uma saida esperada em lexer/testes/X.esperado
# (tokens com linha:coluna e mensagens de erro). Arquivos valido_*.trz devem
# terminar sem erro lexico; invalido_*.trz devem acusar erro lexico.
lexer_test: parser.tab.c parser.tab.h lex.yy.c ast.c lexer/lexer_test.c
	$(CC) $(CFLAGS) -I. -o lexer_test parser.tab.c lex.yy.c ast.c lexer/lexer_test.c

test-lexer: lexer_test
	@falhas=0; tmp=lexer/testes/.saida.tmp; \
	for f in lexer/testes/valido_*.trz lexer/testes/invalido_*.trz; do \
		esp="$${f%.trz}.esperado"; \
		case "$$f" in lexer/testes/invalido_*) rc_ok=1 ;; *) rc_ok=0 ;; esac; \
		rc=0; ./lexer_test "$$f" > "$$tmp" 2>&1 || rc=$$?; \
		if [ "$$rc" != "$$rc_ok" ]; then \
			if [ "$$rc_ok" = 0 ]; then echo "FALHOU  $$f (deveria ser aceito)"; else echo "FALHOU  $$f (deveria dar erro lexico)"; fi; \
			falhas=1; \
		elif [ ! -f "$$esp" ]; then \
			echo "FALHOU  $$f (falta $$esp; gere com: make lexer-esperados)"; falhas=1; \
		elif diff -u --strip-trailing-cr "$$esp" "$$tmp" > /dev/null; then \
			echo "OK      $$f"; \
		else \
			echo "FALHOU  $$f (saida diferente de $$esp)"; diff -u --strip-trailing-cr "$$esp" "$$tmp" | head -20; falhas=1; \
		fi; \
	done; \
	rm -f "$$tmp"; \
	exit $$falhas

# Regenera as saidas esperadas. Use SO depois de conferir que a mudanca nos
# tokens ou nas mensagens e intencional.
lexer-esperados: lexer_test
	@for f in lexer/testes/valido_*.trz lexer/testes/invalido_*.trz; do \
		./lexer_test "$$f" > "$${f%.trz}.esperado" 2>&1 || true; \
		echo "gerado $${f%.trz}.esperado"; \
	done

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h traduzi traduzi.exe lexer_test lexer_test.exe parser_test parser_test.exe

.PHONY: all parser test test-parser test-lexer lexer-esperados clean
