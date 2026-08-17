# ARPIA VM

Toolchain completa para a **VM ARPIA**: uma máquina virtual de 32 bits (inspirada na arquitetura 8086, mas com registradores de 32 bits), um **montador assembly** (`asm`) e um **compilador de um subconjunto de C** (`c`) que gera código para essa VM.

```
programa.c --[c.exe]--> programa.asm --[asm.exe]--> programa.bin --[vm.exe]--> execução
```

A definição formal dos mnemônicos, formato binário e registradores está em `doc/VM ARPIA.xlsx` (abas `Description`, `Instruction Set` e `Register`). Este documento resume o que é necessário para usar e estender o montador e o compilador.

## Sumário

- [Estrutura do repositório](#estrutura-do-repositório)
- [Compilando as ferramentas](#compilando-as-ferramentas)
- [Uso das ferramentas](#uso-das-ferramentas)
- [A VM ARPIA](#a-vm-arpia)
- [O montador (asm)](#o-montador-asm)
- [O compilador C](#o-compilador-c)
- [Funções builtin](#funções-builtin)
- [Limitações conhecidas](#limitações-conhecidas)

## Estrutura do repositório

```
src/
  asm/            fonte do montador (asm.l / asm.y, flex + bison)
  c/              fonte do compilador C (c.l / c.y, flex + bison)
  vm-x86/         VM para desktop (vm.c, syscall.h) — usada em Windows/Linux/Mac
  vm-iot/         VM para microcontrolador (arpia.h com as constantes da ISA,
                  vm-iot.ino, syscall.h) — variante para Arduino/IoT
  Makefile        build de asm.exe / c.exe / vm.exe
bin/              executáveis gerados (asm.exe, c.exe, vm.exe)
samples/          exemplos em assembly (.asm) e em C (.c), com seus .bin
doc/VM ARPIA.xlsx especificação completa da ISA (mnemônicos, encoding, registradores)
```

`src/vm-iot/arpia.h` é a fonte única das constantes da ISA (opcodes, registradores, modos de
endereçamento, códigos de syscall) e é incluída tanto pelo montador quanto pelas duas variantes
de VM.

## Compilando as ferramentas

Requer **flex** e **bison** (no Windows, os binários `win_flex.exe` / `win_bison.exe` do pacote
[winflexbison](https://github.com/lexxmark/winflexbison) funcionam) e um `gcc` compatível com
MinGW.

```
cd src
make asm     # gera bin/asm.exe
make c       # gera bin/c.exe
make vm      # gera bin/vm.exe
```

> **Nota:** o alvo `vm` do `Makefile` ainda aponta para `./vm/vm.c`, que foi substituído por
> `./vm-x86/vm.c` (a VM para desktop) — esse alvo do Makefile está desatualizado. Para recompilar
> a VM manualmente:
> ```
> cd src/vm-x86
> gcc -o ../../bin/vm.exe vm.c
> ```

Se `bison`/`flex` não estiverem no `PATH`, gere os arquivos manualmente antes do `gcc`:

```
cd src/c   # ou src/asm
win_bison -d c.y      # gera c.tab.c / c.tab.h
win_flex c.l          # gera lex.yy.c
gcc -o ../../bin/c.exe c.tab.c
```

## Uso das ferramentas

Todas aceitam arquivo de entrada posicional e `-o <arquivo>` para a saída; sem esses argumentos,
usam stdin/stdout.

```
bin/c.exe   -o programa.asm  programa.c     # compila C -> assembly ARPIA
bin/asm.exe -o programa.bin  programa.asm   # monta assembly -> binário
bin/vm.exe  programa.bin                    # executa o binário
bin/vm.exe  -d programa.bin                 # executa em modo debug (passo a passo)
bin/vm.exe  -v                              # versão
```

## A VM ARPIA

### Registradores

| Registrador | Tamanho | Descrição |
|---|---|---|
| `AH`, `AL` | 8 bits | metades alta/baixa de `AW` |
| `AW` | 16 bits | metade baixa de `AX` |
| `AX` | 32 bits | acumulador principal |
| `BH`, `BL`, `BW`, `BX` | 8/8/16/32 bits | igual a AX, papel de registrador secundário |
| `CX`, `DX`, `SX`, `SP` | 32 bits | registradores gerais / ponteiro de pilha |
| `AF`, `BF`, `CF`, `DF` | 32 bits | registradores de **ponto flutuante** (float) |
| `IP` | 32 bits | instruction pointer |
| `FLAGS` | — | ver flags abaixo |

Não existe instrução `FLOAT` implementada na VM: valores `float` são sempre manipulados através
dos registradores `AF/BF/CF/DF`, e as instruções aritméticas (`ADD`, `SUB`, `MUL`, `DIV`, `CMP`,
`MOV`, ...) tratam esses registradores como float automaticamente.

Flags (bit a bit em `FLAGS`): `ZR` (zero), `CY` (carry), `OV` (overflow), `GT` (maior), `EQ`
(igual), `STR` (modo string), `WDOG` (watchdog — não implementado), `ADDR` (modo de
endereçamento corrente), `INT` (interrupções habilitadas).

### Modos de endereçamento

- **Imediato** — valor literal ou label (`mov ax,10`).
- **Registrador** — `mov ax,bx`.
- **Direto** — `mov ax,[label]` / `mov ax,(1234)`, acessa o conteúdo de um endereço de memória.
- **Indireto** — `mov ax,(bx)`, acessa o conteúdo do endereço contido em um registrador.

`SM20` seleciona endereçamento direto de 20 bits (endereço embutido parcialmente no próprio
mnemônico); `SM32` seleciona 32 bits completos (4 bytes após o mnemônico). Valores imediatos
sempre usam 32 bits.

### Formato do binário gerado

`printcode()` (em `asm.y`) emite: nome do programa, tabela `MAP` (opcional) e o dump em hexa do
código+dados (seção `.code` seguida da `.data`), terminado por um CRC16 simples.

## O montador (asm)

Fonte: `src/asm/asm.l` (léxico) + `src/asm/asm.y` (gramática, bison).

### Estrutura de um programa assembly

```asm
.name "nome_do_programa"

.code
  <instruções>

.data
  <declarações de dados>

.end
```

### Grupos de mnemônicos

O opcode de 16 bits embute o mnemônico e os modos de endereçamento dos operandos. Os mnemônicos
são agrupados por "forma" de instrução (`MNEM_A` .. `MNEM_F`):

| Grupo | Mnemônicos | Operandos |
|---|---|---|
| A | `ADD SUB MUL DIV CMP MOV AND OR XOR SYS IN OUT` | `<dest>, <origem>` — qualquer combinação de registrador/imediato/direto/indireto |
| B | `SHL SHR ROL ROR` | `<dest_reg\|direto>, <reg\|imediato>` |
| C | `CMPA MOVA SETINT` | `<reg\|imediato>, <reg\|imediato>` |
| D | `INC DEC PUSH POP NOT` | um operando (registrador/imediato/direto/indireto) |
| E | `LOOP LOOPNZ JGT JGE JLT JLE JZ JNZ JC JNC JMP CALL WDC INT DELAY` | um operando (label/registrador) |
| F | `PUSHA POPA CLI STI RET IRET BIN STR RST SM20 SM32 EWD DWD RWD NOP` | sem operandos |

### Dados (seção `.data`)

`DB` (byte), `DW` (word/16 bits), `DD` (double word/32 bits), `FL` (float/32 bits, IEEE-754 como
inteiro), listas separadas por vírgula, strings entre aspas, `?` para posição indefinida
(reservada, zerada) e `<n> DUP(?)` para reservar `n` posições indefinidas.

```asm
.data
msg:      db "Hello, world!",13,10,0
contador: dw 0
buffer:   db dup(64) ?
```

### `SYS` e códigos de syscall

`SYS <code>,<param>` invoca uma rotina do "sistema operacional" da VM. `<code>` pode ser um
número literal ou uma das palavras-chave reconhecidas pelo léxico do montador:

| Código | Palavra-chave | Efeito |
|---|---|---|
| 0 | `PRINT` | imprime a string apontada por `<param>` (endereço) |
| 1 | `PRINTS` | define a máscara/formato usado pelo `PRINTF` seguinte |
| 2 | `PRINTF` | imprime usando a máscara definida por `PRINTS`, com `<param>` como valor |
| 3 | `DEBUG` | como `PRINT`, mas só imprime se a VM estiver em modo debug |
| 4 | `HALT` | encerra a execução da VM |
| 5 | — | lê um caractere de stdin (`getchar()`) e grava o valor lido em `AX` |
| 6 | — | configura `<param>` como pino de entrada (`inputpin`) — ainda não implementado na VM |
| 7 | — | configura `<param>` como pino de saída (`outputpin`) — ainda não implementado na VM |
| 8 | — | configura `<param>` como pino de entrada com pull-up (`pulluppin`) — ainda não implementado na VM |

`SYSCALL` **não** é um mnemônico separado do montador — o `SYS` cobre todos os códigos acima e
qualquer código numérico adicional que a VM venha a implementar.

## O compilador C

Fonte: `src/c/c.l` (léxico) + `src/c/c.y` (gramática, bison). Compila um subconjunto simplificado
de C diretamente para assembly ARPIA (nenhuma biblioteca padrão é usada — tudo é gerado via
[funções builtin](#funções-builtin) que mapeiam para instruções/syscalls da VM).

### Tipos suportados

| Tipo | Tamanho | Observação |
|---|---|---|
| `char` | 8 bits | |
| `int` | 16 bits | |
| `long` | 32 bits | |
| `float` | 32 bits | manipulado via registradores `AF/BF/CF/DF` |
| `void` | — | apenas como tipo de retorno de função |
| `struct <nome> { ... };` | soma dos campos | sem `union`, `enum`, `typedef` |
| `tipo nome[N];` | array de tamanho fixo, unidimensional | sem arrays multidimensionais |

Não existem ponteiros explícitos (`*`/`&` como operadores de tipo). Arrays e structs, quando
usados como **parâmetro de função**, são sempre passados por **referência** (endereço de 32
bits) de forma implícita — não é preciso (nem possível) declarar `struct Foo *p`.

### Declarações e escopo

- Globais: `tipo nome;`, `tipo nome[N];`, `tipo nome = <literal>;` (inteiro ou float).
- Locais: iguais às globais, mais `tipo nome = <expressão>;`.
- `struct Nome { campo; ... };` no escopo global, antes do uso.
- Protótipos: `tipo nome(parâmetros);` (sem corpo, terminado em `;`).
- Todo programa precisa de uma função `main` (o runtime gera `call F_main` seguido de
  `sys halt,0`).

### Controle de fluxo e expressões

- `if` / `else`, `while`, `for (init; cond; incr)`, `break`, `continue`, `return [expr];`.
- Operadores aritméticos `+ - * / %`, bit a bit `& | ^ ~ << >>`, lógicos `&& || !`,
  relacionais `== != < > <= >=`, atribuições compostas `+= -= *= /= %= &= |= ^= <<= >>=`,
  `++`/`--` (somente prefixados: `++x`, `--x`).
- Acesso a array `arr[i]`, a campo de struct `s.campo`, chamada de função `f(a, b)`.
- Comentários `/* ... */` e `// ...`.

### Convenção de chamada de função

- Argumentos são avaliados da esquerda para a direita e empilhados (`push`); a função chamada
  desempilha na ordem inversa para os parâmetros declarados.
- O valor de retorno é deixado em `AX` (ou `AF` para `float`) antes do `ret`.
- **Não há pilha de ativação (stack frame):** cada variável local recebe um endereço de memória
  fixo (rótulo `V<n>`) gerado em tempo de compilação, do mesmo jeito que uma variável global.
  Isso significa que **chamadas recursivas não são suportadas** — uma chamada recursiva
  sobrescreve as variáveis locais da chamada em andamento.

Exemplo (`samples/fatorial.c`):

```c
long fatorial(int n) {
    long r;
    r = 1;
    while (n > 1) {
        r = r * n;
        n = n - 1;
    }
    return r;
}

void main() {
    int i;
    print("Calculando fatorial\n");
    i = 1;
    while (i <= 13) {
        printf("Fatorial(%d)", i);
        printf("=%d\n", fatorial(i));
        i = i + 1;
    }
    halt(0);
}
```

Structs e arrays como parâmetro (`samples/estruturas.c`):

```c
struct Point { int x; int y; };

void movepoint(struct Point p, int dx, int dy) {   /* p é passado por referência */
    p.x = p.x + dx;
    p.y = p.y + dy;
}

void soma_array(int arr[], int n, int resultado[]) {
    int i; int total;
    total = 0; i = 0;
    while (i < n) { total = total + arr[i]; i = i + 1; }
    resultado[0] = total;                            /* "retorno" via parâmetro de saída */
}
```

## Funções builtin

Como não há biblioteca padrão, toda interação com o "sistema operacional" da VM (E/S, término do
programa, leitura de teclado) é feita por meio de funções builtin reconhecidas diretamente pelo
compilador (não precisam — e não podem — ser redeclaradas pelo usuário).

| Assinatura | Código gerado | Retorno |
|---|---|---|
| `void print(<endereço/string>);` | `sys print,<param>` (código 0) | — |
| `void debug(<endereço/string>);` | `sys debug,<param>` (código 3) — só imprime em modo debug | — |
| `void halt(int <código>);` | `sys halt,<código>` (código 4) — encerra a VM | — |
| `void printf(<string de formato>, <valor>);` | `sys prints,<formato>` seguido de `sys printf,<valor>` | — |
| `int input(int <porta>);` | `in <reg>,<porta>` | valor lido da porta |
| `void output(int <porta>, int <valor>);` | `out <valor>,<porta>` | — |
| `void delay(int <ms>);` | `delay <reg>` | — |
| `int getch(int <timeout>);` | `sys 5,<reg>` | caractere lido de stdin (`getchar()`), valor devolvido pela VM em `AX` |
| `int syscall(int <number>, int <param>);` | `sys <number>,<param>` | valor devolvido pela VM em `AX` |
| `void inputpin(<pin>);` | `sys 6,<reg>` | — |
| `void outputpin(<pin>);` | `sys 7,<reg>` | — |
| `void pulluppin(<pin>);` | `sys 8,<reg>` | — |

Notas:

- `printf` aceita **exatamente dois** argumentos (string de formato + um único valor/ponteiro) —
  não é variádico como o `printf` real; para imprimir múltiplos valores são necessárias várias
  chamadas (ver exemplo do `fatorial.c` acima).
- `getch` e `syscall` funcionam da mesma forma: o(s) argumento(s) são desempilhados para
  registradores, a instrução `sys` é emitida, e o valor que a rotina de syscall da VM grava em
  `AX` (via `setregval(AX, ...)`) é adotado diretamente como o valor de retorno da função — não
  há nenhum `pop` adicional após o `sys`.
- `syscall(number, param)` permite invocar **qualquer** código de syscall (inclusive futuros,
  ainda não cobertos por uma builtin dedicada) sem precisar estender o compilador.
- `inputpin`/`outputpin`/`pulluppin` seguem o mesmo padrão de `getch` (o argumento é desempilhado
  para um registrador e a instrução `sys` é emitida com o código fixo 6/7/8), mas a rotina de
  syscall da VM ainda **não trata** esses códigos (nenhuma das duas variantes, `vm-x86` nem
  `vm-iot`, tem um `case` para 6/7/8 em `syscall.h`) — o assembly gerado é correto, porém a VM
  atual simplesmente ignora a chamada (nenhum efeito, nenhum erro).

## Limitações conhecidas

- **Sem recursão real** — variáveis locais não vivem em uma pilha de ativação (ver
  [Convenção de chamada de função](#convenção-de-chamada-de-função)).
- **Sem ponteiros** — apenas arrays e structs, sempre passados por referência quando são
  parâmetro de função; não há operadores `*`/`&` de ponteiro nem aritmética de ponteiros.
- **Sem pré-processador** — não há `#include`, `#define`, macros ou compilação condicional.
- **Sem `typedef`, `enum`, `union`** — apenas `struct`.
- **Arrays apenas unidimensionais** e de tamanho fixo (sem `arr[N][M]`, sem alocação dinâmica).
- **`printf` com um único valor por chamada** (sem varargs); conversão de tipo (int/float/string)
  é decidida em tempo de execução pela máscara de formato (`%d`, `%f`/`%g`/`%e`, `%s`).
- **Escopo simples de duas camadas** (global/local) — blocos aninhados não criam escopos novos;
  redeclarar um nome dentro da mesma função é erro, mesmo em blocos `{ }` diferentes.
- **Structs/arrays não podem ser retornados por valor** — o padrão usado é passar um parâmetro de
  "saída" e escrever o resultado nele (ver `soma_array` em `samples/estruturas.c`).
- **Watchdog (`WDC`/`EWD`/`DWD`/`RWD`) não implementado** — presente na ISA para compatibilidade
  futura com a variante IoT, mas ignorado pela linguagem C e pela VM desktop.

