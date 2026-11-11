CC = gcc
CFLAGS = -Wall -DLEXER_STANDALONE -I.

lexer_test: lexer.l lexer_test.c tokens_standalone.h
	flex -o lex.yy.c lexer.l
	$(CC) $(CFLAGS) lex.yy.c lexer_test.c -o lexer_test

testes: lexer_test
	@set -e; \
	for f in valido_*.trz; do \
		echo "=== $$f ==="; \
		./lexer_test "$$f"; \
	done
	@echo "=== Testando arquivo invalido ==="
	@if ./lexer_test invalido_lexico.trz; then \
		echo "ERRO: o arquivo invalido foi aceito."; exit 1; \
	else \
		echo "OK: os erros lexicos foram identificados."; \
	fi

clean:
	rm -f lex.yy.c lexer_test lexer_test.exe

.PHONY: testes clean