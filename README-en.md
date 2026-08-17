# ARPIA VM

*Read in other languages: [Português](README.md) · [Français](README-fr.md) · [Español](README-es.md)*

Complete toolchain for the **ARPIA VM**: a 32-bit virtual machine (inspired by the 8086
architecture, but with 32-bit registers), an **assembler** (`asm`) and a **simplified C compiler**
(`c`) that generates code for that VM.

```
program.c --[c.exe]--> program.asm --[asm.exe]--> program.bin --[vm.exe]--> execution
```

The formal definition of the mnemonics, binary format and registers lives in
`doc/VM ARPIA.xlsx` (`Description`, `Instruction Set` and `Register` tabs). This document
summarizes what you need to use and extend the assembler and the compiler.

## Table of contents

- [Repository layout](#repository-layout)
- [Building the tools](#building-the-tools)
- [Using the tools](#using-the-tools)
- [The ARPIA VM](#the-arpia-vm)
- [IoT VM (vm-iot.ino)](#iot-vm-vm-iotino)
- [The assembler (asm)](#the-assembler-asm)
- [The C compiler](#the-c-compiler)
- [Builtin functions](#builtin-functions)
- [Known limitations](#known-limitations)

## Repository layout

```
src/
  asm/            assembler source (asm.l / asm.y, flex + bison)
  c/              C compiler source (c.l / c.y, flex + bison)
  vm-x86/         desktop VM (vm.c, syscall.h) — used on Windows/Linux/Mac
  vm-iot/         microcontroller VM (arpia.h with the ISA constants,
                  vm-iot.ino, syscall.h) — Arduino/IoT variant, with a
                  serial minishell, WiFi and a web server (LittleFS)
  Makefile        builds asm.exe / c.exe / vm.exe
bin/              generated executables (asm.exe, c.exe, vm.exe)
samples/          examples in assembly (.asm) and C (.c), with their .bin
doc/VM ARPIA.xlsx complete ISA specification (mnemonics, encoding, registers)
```

`src/vm-iot/arpia.h` is the single source of truth for the ISA constants (opcodes, registers,
addressing modes, syscall codes) and is included by both the assembler and the two VM variants.

## Building the tools

Requires **flex** and **bison** (on Windows, the `win_flex.exe` / `win_bison.exe` binaries from
the [winflexbison](https://github.com/lexxmark/winflexbison) package work) and a MinGW-compatible
`gcc`.

```
cd src
make asm     # builds bin/asm.exe
make c       # builds bin/c.exe
make vm      # builds bin/vm.exe
```

> **Note:** the `vm` target in the `Makefile` still points to `./vm/vm.c`, which was replaced by
> `./vm-x86/vm.c` (the desktop VM) — that Makefile target is out of date. To rebuild the VM
> manually:
> ```
> cd src/vm-x86
> gcc -o ../../bin/vm.exe vm.c
> ```

If `bison`/`flex` are not on your `PATH`, generate the files manually before running `gcc`:

```
cd src/c   # or src/asm
win_bison -d c.y      # generates c.tab.c / c.tab.h
win_flex c.l          # generates lex.yy.c
gcc -o ../../bin/c.exe c.tab.c
```

## Using the tools

All three accept a positional input file and `-o <file>` for the output; without these
arguments they use stdin/stdout.

```
bin/c.exe   -o program.asm  program.c     # compile C -> ARPIA assembly
bin/asm.exe -o program.bin  program.asm   # assemble -> binary
bin/vm.exe  program.bin                   # run the binary
bin/vm.exe  -d program.bin                # run in debug mode (step by step)
bin/vm.exe  -v                            # version
```

## The ARPIA VM

### Registers

| Register | Size | Description |
|---|---|---|
| `AH`, `AL` | 8 bit | high/low halves of `AW` |
| `AW` | 16 bit | low half of `AX` |
| `AX` | 32 bit | main accumulator |
| `BH`, `BL`, `BW`, `BX` | 8/8/16/32 bit | same layout as AX, secondary register |
| `CX`, `DX`, `SX`, `SP` | 32 bit | general-purpose registers / stack pointer |
| `AF`, `BF`, `CF`, `DF` | 32 bit | **floating-point** registers |
| `IP` | 32 bit | instruction pointer |
| `FLAGS` | — | see flags below |

There is no `FLOAT` instruction implemented in the VM: `float` values are always handled through
the `AF/BF/CF/DF` registers, and the arithmetic instructions (`ADD`, `SUB`, `MUL`, `DIV`, `CMP`,
`MOV`, ...) treat those registers as floats automatically.

Flags (bits in `FLAGS`): `ZR` (zero), `CY` (carry), `OV` (overflow), `GT` (greater), `EQ`
(equal), `STR` (string mode), `WDOG` (watchdog — not implemented), `ADDR` (current addressing
mode), `INT` (interrupts enabled).

### Addressing modes

- **Immediate** — literal value or label (`mov ax,10`).
- **Register** — `mov ax,bx`.
- **Direct** — `mov ax,[label]` / `mov ax,(1234)`, accesses the contents of a memory address.
- **Indirect** — `mov ax,(bx)`, accesses the contents of the address held in a register.

`SM20` selects 20-bit direct addressing (the address is partially embedded in the mnemonic
itself); `SM32` selects full 32-bit addressing (4 bytes after the mnemonic). Immediate values
always use 32 bits.

### Generated binary format

`printcode()` (in `asm.y`) emits: the program name, the `MAP` table (optional), and a hex dump of
the code+data (`.code` section followed by `.data`), terminated by a simple CRC16.

## IoT VM (vm-iot.ino)

Source: `src/vm-iot/vm-iot.ino` (+ `arpia.h`, `vm-arpia.h`, `syscall.h`). An Arduino sketch for
ESP32/ESP8266 that runs the same ARPIA VM ISA, storing programs on a **LittleFS** filesystem
local to the board.

### Serial minishell

On boot, the sketch mounts LittleFS, connects to WiFi, and opens a command console over the
serial port (115200 bps):

| Command | Effect |
|---|---|
| `ls` / `dir` | lists the files on LittleFS (name and size) |
| `ver` / `version` | shows the firmware version |
| `help` / `?` | shows the command list |
| `format` | formats LittleFS |
| `rm` / `del <file>` | deletes a file |
| `cat` / `type <file>` | prints the file contents |
| `dump <file>` | prints a hex dump of the file |
| `cls` / `clear` | clears the terminal screen |
| `term` | toggles echoing the raw key codes received (debug) |
| `wifi` | Configures the WiFi SSID and password |
| `<file>` | loads and runs the file as an ARPIA program |

### WiFi and web UI

In `setup()`, the sketch tries to connect as a station to the network defined by `WIFI_SSID`/
`WIFI_PASSWORD` (constants at the top of `vm-iot.ino`, editable or overridable via a build flag);
if the connection fails within 15 seconds, it starts its own Access Point (`ArpiaVM`, password
`arpiavm123`) so the UI stays reachable even without a configured network. The resulting IP
address (from the local network or the AP) is printed over serial and shown by the `help`
command. The minishell's `wifi` command lets you change SSID/password at runtime, saving the
credentials to `/wifi.txt` on LittleFS (read on every boot, overriding `WIFI_SSID`/
`WIFI_PASSWORD`).

A built-in web server (`WebServer` on ESP32 / `ESP8266WebServer` on ESP8266) serves a page at
`http://<ip>/` that lists the files on LittleFS with:

- **Delete** for an existing file.
- **Upload** for a new file (multipart form, written straight to LittleFS).

The web server keeps responding even while the minishell is waiting for a command on the serial
port.

## The assembler (asm)

Source: `src/asm/asm.l` (lexer) + `src/asm/asm.y` (grammar, bison).

### Structure of an assembly program

```asm
.name "program_name"

.code
  <instructions>

.data
  <data declarations>

.end
```

### Mnemonic groups

The 16-bit opcode embeds the mnemonic and the addressing modes of the operands. Mnemonics are
grouped by instruction "shape" (`MNEM_A` .. `MNEM_F`):

| Group | Mnemonics | Operands |
|---|---|---|
| A | `ADD SUB MUL DIV CMP MOV AND OR XOR SYS IN OUT` | `<dest>, <src>` — any combination of register/immediate/direct/indirect |
| B | `SHL SHR ROL ROR` | `<dest_reg\|direct>, <reg\|immediate>` |
| C | `CMPA MOVA SETINT` | `<reg\|immediate>, <reg\|immediate>` |
| D | `INC DEC PUSH POP NOT` | one operand (register/immediate/direct/indirect) |
| E | `LOOP LOOPNZ JGT JGE JLT JLE JZ JNZ JC JNC JMP CALL WDC INT DELAY` | one operand (label/register) |
| F | `PUSHA POPA CLI STI RET IRET BIN STR RST SM20 SM32 EWD DWD RWD NOP` | no operands |

### Data (`.data` section)

`DB` (byte), `DW` (word/16 bit), `DD` (double word/32 bit), `FL` (float/32 bit, IEEE-754 as an
integer), comma-separated lists, quoted strings, `?` for an undefined (reserved, zeroed) slot,
and `<n> DUP(?)` to reserve `n` undefined slots.

```asm
.data
msg:      db "Hello, world!",13,10,0
counter:  dw 0
buffer:   db dup(64) ?
```

### `SYS` and syscall codes

`SYS <code>,<param>` invokes a routine of the VM's "operating system". `<code>` can be a literal
number or one of the keywords recognized by the assembler's lexer:

| Code | Keyword | Effect |
|---|---|---|
| 0 | `PRINT` | prints the string pointed to by `<param>` (address) |
| 1 | `PRINTS` | sets the format mask used by the following `PRINTF` |
| 2 | `PRINTF` | prints using the mask set by `PRINTS`, with `<param>` as the value |
| 3 | `DEBUG` | like `PRINT`, but only prints if the VM is in debug mode |
| 4 | `HALT` | stops VM execution |
| 5 | `GETCH` | reads a character from stdin (`getchar()`) and writes the value read into `AX` |
| 6 | `INPUTPIN` | configures `<param>` as an input pin (`inputpin`) |
| 7 | `OUTPUTPIN` | configures `<param>` as an output pin (`outputpin`) |
| 8 | `PULLUP` | configures `<param>` as an input pin with pull-up (`pulluppin`) |

`SYSCALL` is **not** a separate assembler mnemonic — `SYS` covers all the codes above and any
additional numeric code the VM may implement in the future.

## The C compiler

Source: `src/c/c.l` (lexer) + `src/c/c.y` (grammar, bison). It compiles a simplified subset of C
directly to ARPIA assembly (no standard library is used — everything is generated through
[builtin functions](#builtin-functions) that map to VM instructions/syscalls).

### Supported types

| Type | Size | Note |
|---|---|---|
| `char` | 8 bit | |
| `int` | 16 bit | |
| `long` | 32 bit | |
| `float` | 32 bit | handled via the `AF/BF/CF/DF` registers |
| `void` | — | only as a function return type |
| `struct <name> { ... };` | sum of fields | no `union`, `enum`, `typedef` |
| `type name[N];` | fixed-size, one-dimensional array | no multi-dimensional arrays |

There are no explicit pointers (`*`/`&` as type operators). Arrays and structs, when used as a
**function parameter**, are always implicitly passed **by reference** (32-bit address) — you
cannot (and don't need to) declare `struct Foo *p`.

### Declarations and scope

- Globals: `type name;`, `type name[N];`, `type name = <literal>;` (integer or float).
- Locals: same as globals, plus `type name = <expression>;`.
- `struct Name { field; ... };` at global scope, before use.
- Prototypes: `type name(parameters);` (no body, terminated by `;`).
- Every program needs a `main` function (the runtime emits `call F_main` followed by
  `sys halt,0`).

### Control flow and expressions

- `if` / `else`, `while`, `for (init; cond; incr)`, `break`, `continue`, `return [expr];`.
- Arithmetic operators `+ - * / %`, bitwise `& | ^ ~ << >>`, logical `&& || !`,
  relational `== != < > <= >=`, compound assignments `+= -= *= /= %= &= |= ^= <<= >>=`,
  `++`/`--` (prefix only: `++x`, `--x`).
- Array access `arr[i]`, struct field access `s.field`, function calls `f(a, b)`.
- Comments `/* ... */` and `// ...`.

### Function-call convention

- Arguments are evaluated left to right and pushed (`push`); the called function pops them in
  reverse order to match the declared parameters.
- The return value is left in `AX` (or `AF` for `float`) before `ret`.
- **There is no activation stack frame:** every local variable gets a fixed memory address
  (label `V<n>`) generated at compile time, exactly like a global variable. This means
  **recursive calls are not supported** — a recursive call overwrites the local variables of the
  call already in progress.

Example (`samples/fatorial.c`):

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

Structs and arrays as parameters (`samples/estruturas.c`):

```c
struct Point { int x; int y; };

void movepoint(struct Point p, int dx, int dy) {   /* p is passed by reference */
    p.x = p.x + dx;
    p.y = p.y + dy;
}

void soma_array(int arr[], int n, int resultado[]) {
    int i; int total;
    total = 0; i = 0;
    while (i < n) { total = total + arr[i]; i = i + 1; }
    resultado[0] = total;                            /* "return" via output parameter */
}
```

## Builtin functions

Since there is no standard library, all interaction with the VM's "operating system" (I/O,
program termination, keyboard input) happens through builtin functions recognized directly by
the compiler (they cannot — and do not need to — be redeclared by the user).

| Signature | Generated code | Return value |
|---|---|---|
| `void print(<address/string>);` | `sys print,<param>` (code 0) | — |
| `void debug(<address/string>);` | `sys debug,<param>` (code 3) — only prints in debug mode | — |
| `void halt(int <code>);` | `sys halt,<code>` (code 4) — stops the VM | — |
| `void printf(<format string>, <value>);` | `sys prints,<format>` followed by `sys printf,<value>` | — |
| `int input(int <port>);` | `in <reg>,<port>` | value read from the port |
| `void output(int <port>, int <value>);` | `out <value>,<port>` | — |
| `void delay(int <ms>);` | `delay <reg>` | — |
| `int getch(int <timeout>);` | `sys getch,<reg>` | character read from stdin (`getchar()`), value returned by the VM in `AX` |
| `int syscall(int <number>, int <param>);` | `sys <number>,<param>` | value returned by the VM in `AX` |
| `void inputpin(<pin>);` | `sys inputpin,<reg>` | — |
| `void outputpin(<pin>);` | `sys outputpin,<reg>` | — |
| `void pulluppin(<pin>);` | `sys pullup,<reg>` | — |

Notes:

- `printf` takes **exactly two** arguments (format string + a single value/pointer) — it is not
  variadic like the real `printf`; printing multiple values requires several calls (see the
  `fatorial.c` example above).
- `getch` and `syscall` work the same way: the argument(s) are popped into registers, the `sys`
  instruction is emitted, and the value the VM's syscall routine writes into `AX` (via
  `setregval(AX, ...)`) is adopted directly as the function's return value — there is no extra
  `pop` after the `sys`.
- `syscall(number, param)` lets you invoke **any** syscall code (including future ones not yet
  covered by a dedicated builtin) without having to extend the compiler.
- `inputpin`/`outputpin`/`pulluppin` follow the same pattern as `getch` (the argument is popped
  into a register and the `sys` instruction is emitted).

## Known limitations

- **No real recursion** — local variables don't live on an activation stack (see
  [Function-call convention](#function-call-convention)).
- **No pointers** — only arrays and structs, always passed by reference when they are a function
  parameter; there are no `*`/`&` pointer operators or pointer arithmetic.
- **No preprocessor** — no `#include`, `#define`, macros, or conditional compilation.
- **No `typedef`, `enum`, `union`** — only `struct`.
- **Arrays are one-dimensional only** and fixed-size (no `arr[N][M]`, no dynamic allocation).
- **`printf` takes a single value per call** (no varargs); type conversion (int/float/string) is
  decided at runtime by the format mask (`%d`, `%f`/`%g`/`%e`, `%s`).
- **Simple two-level scope** (global/local) — nested blocks don't create new scopes; redeclaring
  a name within the same function is an error, even in different `{ }` blocks.
- **Structs/arrays cannot be returned by value** — the pattern used instead is passing an
  "output" parameter and writing the result into it (see `soma_array` in
  `samples/estruturas.c`).
- **Watchdog (`WDC`/`EWD`/`DWD`/`RWD`) not implemented** — present in the ISA for future
  compatibility with the IoT variant, but ignored by the C language and by the desktop VM.
