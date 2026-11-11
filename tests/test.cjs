const assert = require('node:assert/strict');
const { spawnSync } = require('node:child_process');
const path = require('node:path');
const exe = path.resolve(process.platform === 'win32' ? 'traduzi.exe' : 'traduzi');
const run = source => {
    const result = spawnSync(exe, [], { input: source, encoding: 'utf8' });
    if (result.error) throw result.error;
    return result;
};
const wrap = body => `acao Traduzi() void [ ${body} ]`;
const validos = [
    '', 'int x = 10 + 5 * 2; stampa(x);',
    'int x = (10 + 5) * 2;',
    'bool x = not 1 < 2 and true or false;',
    'in caso di -a - b > -2- succede [] altrimenti []',
    'mentre --x < 2- []',
    'mentre -true- ~> fechamento\n []',
    'in caso di -true- ~{ fechamento }~ succede []',
    'per i de -1 ate f(2, 3) [ continuare; fermarsi; ]',
    'string nome; input(nome); stampa(:Ola, mundo:);',
    'float x = reale(intero(2.5)); f(); ritorna;'
];
for (const body of validos) {
    const result = run(wrap(body));
    assert.equal(result.status, 0, `${body}\n${result.stderr}`);
}
const invalidos = [
    'int x = 10 + ;', 'stampa((2);', 'mentre -true- [',
    'in caso di -true succede []', 'int x = 1 < 2 < 3;',
    'bool x = true == false == true;', 'int x = 2',
    'stampa(:a:, :b:);', 'int 2idade = 2;', 'float x = .5;',
    'float x = 5.;', 'stampa(:aberto);', '@;', '~{ aberto',
    'acao outra() void []', 'Traduzi();'
];
for (const body of invalidos) {
    const result = run(wrap(body));
    assert.notEqual(result.status, 0, body);
    assert.equal(result.stdout, '', body);
    assert.match(result.stderr, /Erro (sintatico|lexico) linha \d+, coluna \d+:/);
}
const precedence = run(wrap('int x = 10 + 5 * 2;')).stdout;
assert.match(precedence, /operacao \+[\s\S]*inteiro 10[\s\S]*operacao \*[\s\S]*inteiro 5[\s\S]*inteiro 2/);
assert.match(run(wrap('int x = 10 - 5 - 2;')).stdout, /operacao -[^\n]*\n {8}operacao -/);
const recovery = run(wrap('int x = ; stampa(1); int y = ;'));
assert.ok((recovery.stderr.match(/Erro sintatico/g) || []).length >= 2);
const complete = spawnSync(exe, ['exemplos/completo.traduzi'], { encoding: 'utf8' });
assert.equal(complete.status, 0, complete.stderr);
console.log(`${validos.length + invalidos.length + 4} testes passaram.`);
