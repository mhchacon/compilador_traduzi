const assert = require('node:assert/strict');
const { spawnSync } = require('node:child_process');
const path = require('node:path');
const exe = path.resolve(process.platform === 'win32' ? 'parser_test.exe' : 'parser_test');
const run = input => {
    const r = spawnSync(exe, [], { input, encoding: 'utf8' });
    if (r.error) throw r.error;
    r.stdout = r.stdout.replace(/\r\n/g, '\n');
    return r;
};
const wrap = s => `acao Traduzi() void [\n${s}\n]`;
const validos = [
    'acao Traduzi() void []',
    'acao soma(int a, float b) ritorna float [ ritorna a+b; ] acao Traduzi() void [ soma(1, 2.0); ]',
    wrap('mutabile int x; x = 3; input(x); stampa(:oi:);'),
    wrap('in caso di -true- succede [] altrimenti in caso di -false- succede [] altrimenti []'),
    wrap('mentre -x - 1 > -2- [ per i de -1 ate 4 [ continuare; fermarsi; ] ]'),
    wrap('float x = reale(intero(2.5)); bool b = not 1 < 2 and true or false;'),
    // Sao sintaticamente validos; as restricoes abaixo pertencem a semantica.
    wrap('x = :texto:; desconhecida(true, 2); fermarsi; ritorna false;')
];
for (const s of validos) assert.equal(run(s).status, 0, s);
const invalidos = [
    '', 'acao Traduzi( void []', 'acao f(int) void []',
    wrap('int x = ;'), wrap('int x = 1'), wrap('x 2;'),
    wrap('stampa(1, 2);'), wrap('f(1,);'), wrap('int x = 1 < 2 < 3;'),
    wrap('bool b = true == false == true;'), wrap('mentre true []'),
    wrap('in caso di -true- []'), wrap('altrimenti []'),
    wrap('per i de 1 3 []'), wrap('acao interna() void []'),
    'acao Traduzi() void [ stampa(1);'
];
for (const s of invalidos) {
    const r = run(s);
    assert.notEqual(r.status, 0, s);
    assert.equal(r.stdout, '', s);
    assert.match(r.stderr, /Erro sintatico linha \d+, coluna \d+:/, s);
}
const expr = s => run(wrap(`stampa(${s});`)).stdout;
assert.match(expr('10 + 5 * 2'), /operacao \+[^\n]*\n {10}inteiro 10[^\n]*\n {10}operacao \*/);
assert.match(expr('(10 + 5) * 2'), /operacao \*[^\n]*\n {10}operacao \+/);
assert.match(expr('10 - 5 - 2'), /operacao -[^\n]*\n {10}operacao -/);
assert.match(expr('20 / 5 / 2'), /operacao \/[^\n]*\n {10}operacao \//);
assert.match(expr('not true and false or true'), /operacao or[^\n]*\n {10}operacao and[^\n]*\n {12}operacao not/);
const location = run(wrap('mutabile int x = 42;')).stdout;
assert.match(location, /declaracao x tipo=int mutabile \(2:10\)/);
assert.match(location, /inteiro 42 \(2:18\)/);
const recovered = run(wrap('int x = ; stampa(1); int y = ; stampa(2);'));
assert.notEqual(recovered.status, 0);
assert.equal(recovered.stdout, '');
assert.equal((recovered.stderr.match(/Erro sintatico/g) || []).length, 2);
console.log(`${validos.length + invalidos.length + 8} testes de sintaxe/AST passaram.`);
