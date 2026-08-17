# ARPIA VM

*Leer en otros idiomas: [Português](README.md) · [English](README-en.md) · [Français](README-fr.md)*

Toolchain completa para la **VM ARPIA**: una máquina virtual de 32 bits (inspirada en la
arquitectura 8086, pero con registros de 32 bits), un **ensamblador** (`asm`) y un **compilador
de un subconjunto de C** (`c`) que genera código para esa VM.

```
programa.c --[c.exe]--> programa.asm --[asm.exe]--> programa.bin --[vm.exe]--> ejecución
```

La definición formal de los mnemónicos, el formato binario y los registros está en
`doc/VM ARPIA.xlsx` (pestañas `Description`, `Instruction Set` y `Register`). Este documento
resume lo necesario para usar y extender el ensamblador y el compilador.

## Índice

- [Estructura del repositorio](#estructura-del-repositorio)
- [Compilando las herramientas](#compilando-las-herramientas)
- [Uso de las herramientas](#uso-de-las-herramientas)
- [La VM ARPIA](#la-vm-arpia)
- [VM IoT (vm-iot.ino)](#vm-iot-vm-iotino)
- [El ensamblador (asm)](#el-ensamblador-asm)
- [El compilador C](#el-compilador-c)
- [Funciones integradas (builtin)](#funciones-integradas-builtin)
- [Limitaciones conocidas](#limitaciones-conocidas)

## Estructura del repositorio

```
src/
  asm/            fuente del ensamblador (asm.l / asm.y, flex + bison)
  c/              fuente del compilador C (c.l / c.y, flex + bison)
  vm-x86/         VM de escritorio (vm.c, syscall.h) — usada en Windows/Linux/Mac
  vm-iot/         VM para microcontrolador (arpia.h con las constantes de la ISA,
                  vm-iot.ino, syscall.h) — variante para Arduino/IoT, con
                  minishell serie, WiFi y servidor web (LittleFS)
  Makefile        compila asm.exe / c.exe / vm.exe
bin/              ejecutables generados (asm.exe, c.exe, vm.exe)
samples/          ejemplos en ensamblador (.asm) y en C (.c), con sus .bin
doc/VM ARPIA.xlsx especificación completa de la ISA (mnemónicos, codificación, registros)
```

`src/vm-iot/arpia.h` es la fuente única de las constantes de la ISA (opcodes, registros, modos
de direccionamiento, códigos de syscall) y es incluida tanto por el ensamblador como por las dos
variantes de la VM.

## Compilando las herramientas

Requiere **flex** y **bison** (en Windows, los binarios `win_flex.exe` / `win_bison.exe` del
paquete [winflexbison](https://github.com/lexxmark/winflexbison) funcionan) y un `gcc`
compatible con MinGW.

```
cd src
make asm     # genera bin/asm.exe
make c       # genera bin/c.exe
make vm      # genera bin/vm.exe
```

> **Nota:** el objetivo `vm` del `Makefile` todavía apunta a `./vm/vm.c`, que fue reemplazado por
> `./vm-x86/vm.c` (la VM de escritorio) — ese objetivo del Makefile está desactualizado. Para
> recompilar la VM manualmente:
> ```
> cd src/vm-x86
> gcc -o ../../bin/vm.exe vm.c
> ```

Si `bison`/`flex` no están en el `PATH`, genere los archivos manualmente antes del `gcc`:

```
cd src/c   # o src/asm
win_bison -d c.y      # genera c.tab.c / c.tab.h
win_flex c.l          # genera lex.yy.c
gcc -o ../../bin/c.exe c.tab.c
```

## Uso de las herramientas

Las tres aceptan un archivo de entrada posicional y `-o <archivo>` para la salida; sin esos
argumentos, usan stdin/stdout.

```
bin/c.exe   -o programa.asm  programa.c     # compila C -> ensamblador ARPIA
bin/asm.exe -o programa.bin  programa.asm   # ensambla -> binario
bin/vm.exe  programa.bin                    # ejecuta el binario
bin/vm.exe  -d programa.bin                 # ejecuta en modo debug (paso a paso)
bin/vm.exe  -v                              # versión
```

## La VM ARPIA

### Registros

| Registro | Tamaño | Descripción |
|---|---|---|
| `AH`, `AL` | 8 bits | mitades alta/baja de `AW` |
| `AW` | 16 bits | mitad baja de `AX` |
| `AX` | 32 bits | acumulador principal |
| `BH`, `BL`, `BW`, `BX` | 8/8/16/32 bits | igual que AX, registro secundario |
| `CX`, `DX`, `SX`, `SP` | 32 bits | registros generales / puntero de pila |
| `AF`, `BF`, `CF`, `DF` | 32 bits | registros de **punto flotante** (float) |
| `IP` | 32 bits | puntero de instrucción |
| `FLAGS` | — | ver banderas abajo |

No existe una instrucción `FLOAT` implementada en la VM: los valores `float` siempre se manejan
a través de los registros `AF/BF/CF/DF`, y las instrucciones aritméticas (`ADD`, `SUB`, `MUL`,
`DIV`, `CMP`, `MOV`, ...) tratan esos registros como float automáticamente.

Banderas (bits de `FLAGS`): `ZR` (cero), `CY` (acarreo), `OV` (desbordamiento), `GT` (mayor),
`EQ` (igual), `STR` (modo cadena), `WDOG` (watchdog — no implementado), `ADDR` (modo de
direccionamiento actual), `INT` (interrupciones habilitadas).

### Modos de direccionamiento

- **Inmediato** — valor literal o etiqueta (`mov ax,10`).
- **Registro** — `mov ax,bx`.
- **Directo** — `mov ax,[label]` / `mov ax,(1234)`, accede al contenido de una dirección de
  memoria.
- **Indirecto** — `mov ax,(bx)`, accede al contenido de la dirección almacenada en un registro.

`SM20` selecciona direccionamiento directo de 20 bits (la dirección se incrusta parcialmente en
el propio mnemónico); `SM32` selecciona 32 bits completos (4 bytes después del mnemónico). Los
valores inmediatos siempre usan 32 bits.

### Formato del binario generado

`printcode()` (en `asm.y`) emite: el nombre del programa, la tabla `MAP` (opcional) y el volcado
en hexadecimal del código+datos (sección `.code` seguida de `.data`), terminado con un CRC16
simple.

## VM IoT (vm-iot.ino)

Fuente: `src/vm-iot/vm-iot.ino` (+ `arpia.h`, `vm-arpia.h`, `syscall.h`). Sketch de Arduino para
ESP32/ESP8266 que ejecuta la misma ISA de la VM ARPIA, almacenando los programas en un sistema de
archivos **LittleFS** local a la placa.

### Minishell por serie

Al arrancar, el sketch monta el LittleFS, se conecta al WiFi y abre una consola de comandos por el
puerto serie (115200 bps):

| Comando | Efecto |
|---|---|
| `ls` / `dir` | lista los archivos del LittleFS (nombre y tamaño) |
| `ver` / `version` | muestra la versión del firmware |
| `help` / `?` | muestra la lista de comandos |
| `format` | formatea el LittleFS |
| `rm` / `del <archivo>` | borra un archivo |
| `cat` / `type <archivo>` | muestra el contenido del archivo |
| `dump <archivo>` | muestra el volcado hexadecimal del archivo |
| `cls` / `clear` | limpia la pantalla del terminal |
| `term` | activa/desactiva la impresión de los códigos de tecla recibidos (debug) |
| `<archivo>` | carga y ejecuta el archivo como programa ARPIA |

Ya no existe un comando `upload` por el terminal serie — la subida de archivos al LittleFS ahora se
hace desde la interfaz web (ver abajo).

### WiFi e interfaz web

En `setup()`, el sketch intenta conectarse como estación a la red definida por `WIFI_SSID`/
`WIFI_PASSWORD` (constantes al inicio de `vm-iot.ino`, editables o sobrescribibles mediante una
flag de compilación); si la conexión falla en 15 segundos, levanta su propio punto de acceso
(`ArpiaVM`, contraseña `arpiavm123`) para garantizar el acceso a la interfaz incluso sin una red
configurada. La dirección IP obtenida (de la red local o del punto de acceso) se muestra por el
puerto serie y en el comando `help`.

Un servidor web integrado (`WebServer` en ESP32 / `ESP8266WebServer` en ESP8266) sirve una página
en `http://<ip>/` que lista los archivos del LittleFS con opciones para:

- **Eliminar** un archivo existente.
- **Subir** un archivo nuevo (formulario multipart, escrito directamente en el LittleFS).

El servidor web sigue respondiendo incluso mientras el minishell espera un comando por el puerto
serie.

## El ensamblador (asm)

Fuente: `src/asm/asm.l` (léxico) + `src/asm/asm.y` (gramática, bison).

### Estructura de un programa en ensamblador

```asm
.name "nombre_del_programa"

.code
  <instrucciones>

.data
  <declaraciones de datos>

.end
```

### Grupos de mnemónicos

El opcode de 16 bits incorpora el mnemónico y los modos de direccionamiento de los operandos.
Los mnemónicos se agrupan por "forma" de instrucción (`MNEM_A` .. `MNEM_F`):

| Grupo | Mnemónicos | Operandos |
|---|---|---|
| A | `ADD SUB MUL DIV CMP MOV AND OR XOR SYS IN OUT` | `<dest>, <origen>` — cualquier combinación de registro/inmediato/directo/indirecto |
| B | `SHL SHR ROL ROR` | `<dest_reg\|directo>, <reg\|inmediato>` |
| C | `CMPA MOVA SETINT` | `<reg\|inmediato>, <reg\|inmediato>` |
| D | `INC DEC PUSH POP NOT` | un operando (registro/inmediato/directo/indirecto) |
| E | `LOOP LOOPNZ JGT JGE JLT JLE JZ JNZ JC JNC JMP CALL WDC INT DELAY` | un operando (etiqueta/registro) |
| F | `PUSHA POPA CLI STI RET IRET BIN STR RST SM20 SM32 EWD DWD RWD NOP` | sin operandos |

### Datos (sección `.data`)

`DB` (byte), `DW` (word/16 bits), `DD` (double word/32 bits), `FL` (float/32 bits, IEEE-754 como
entero), listas separadas por comas, cadenas entre comillas, `?` para una posición indefinida
(reservada, en cero) y `<n> DUP(?)` para reservar `n` posiciones indefinidas.

```asm
.data
msg:      db "Hello, world!",13,10,0
contador: dw 0
buffer:   db dup(64) ?
```

### `SYS` y códigos de syscall

`SYS <code>,<param>` invoca una rutina del "sistema operativo" de la VM. `<code>` puede ser un
número literal o una de las palabras clave reconocidas por el léxico del ensamblador:

| Código | Palabra clave | Efecto |
|---|---|---|
| 0 | `PRINT` | imprime la cadena apuntada por `<param>` (dirección) |
| 1 | `PRINTS` | define la máscara/formato usado por el `PRINTF` siguiente |
| 2 | `PRINTF` | imprime usando la máscara definida por `PRINTS`, con `<param>` como valor |
| 3 | `DEBUG` | como `PRINT`, pero solo imprime si la VM está en modo debug |
| 4 | `HALT` | detiene la ejecución de la VM |
| 5 | — | lee un carácter de stdin (`getchar()`) y escribe el valor leído en `AX` |
| 6 | — | configura `<param>` como pin de entrada (`inputpin`) — aún no implementado en la VM |
| 7 | — | configura `<param>` como pin de salida (`outputpin`) — aún no implementado en la VM |
| 8 | — | configura `<param>` como pin de entrada con pull-up (`pulluppin`) — aún no implementado en la VM |

`SYSCALL` **no** es un mnemónico separado del ensamblador — `SYS` cubre todos los códigos
anteriores y cualquier código numérico adicional que la VM llegue a implementar.

## El compilador C

Fuente: `src/c/c.l` (léxico) + `src/c/c.y` (gramática, bison). Compila un subconjunto
simplificado de C directamente a ensamblador ARPIA (no se usa ninguna biblioteca estándar — todo
se genera mediante [funciones integradas](#funciones-integradas-builtin) que se corresponden con
instrucciones/syscalls de la VM).

### Tipos soportados

| Tipo | Tamaño | Observación |
|---|---|---|
| `char` | 8 bits | |
| `int` | 16 bits | |
| `long` | 32 bits | |
| `float` | 32 bits | manejado mediante los registros `AF/BF/CF/DF` |
| `void` | — | solo como tipo de retorno de función |
| `struct <nombre> { ... };` | suma de los campos | sin `union`, `enum`, `typedef` |
| `tipo nombre[N];` | arreglo de tamaño fijo, unidimensional | sin arreglos multidimensionales |

No existen punteros explícitos (`*`/`&` como operadores de tipo). Los arreglos y structs, cuando
se usan como **parámetro de función**, siempre se pasan implícitamente **por referencia**
(dirección de 32 bits) — no es necesario (ni posible) declarar `struct Foo *p`.

### Declaraciones y ámbito

- Globales: `tipo nombre;`, `tipo nombre[N];`, `tipo nombre = <literal>;` (entero o float).
- Locales: igual que las globales, más `tipo nombre = <expresión>;`.
- `struct Nombre { campo; ... };` en el ámbito global, antes de su uso.
- Prototipos: `tipo nombre(parámetros);` (sin cuerpo, terminado en `;`).
- Todo programa necesita una función `main` (el runtime genera `call F_main` seguido de
  `sys halt,0`).

### Control de flujo y expresiones

- `if` / `else`, `while`, `for (init; cond; incr)`, `break`, `continue`, `return [expr];`.
- Operadores aritméticos `+ - * / %`, bit a bit `& | ^ ~ << >>`, lógicos `&& || !`,
  relacionales `== != < > <= >=`, asignaciones compuestas `+= -= *= /= %= &= |= ^= <<= >>=`,
  `++`/`--` (solo prefijos: `++x`, `--x`).
- Acceso a arreglo `arr[i]`, a campo de struct `s.campo`, llamada a función `f(a, b)`.
- Comentarios `/* ... */` y `// ...`.

### Convención de llamada a función

- Los argumentos se evalúan de izquierda a derecha y se apilan (`push`); la función llamada los
  desapila en orden inverso para los parámetros declarados.
- El valor de retorno se deja en `AX` (o `AF` para `float`) antes del `ret`.
- **No hay marco de pila de activación (stack frame):** cada variable local recibe una dirección
  de memoria fija (etiqueta `V<n>`) generada en tiempo de compilación, igual que una variable
  global. Esto significa que **las llamadas recursivas no son compatibles** — una llamada
  recursiva sobrescribe las variables locales de la llamada en curso.

Ejemplo (`samples/fatorial.c`):

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

Structs y arreglos como parámetro (`samples/estruturas.c`):

```c
struct Point { int x; int y; };

void movepoint(struct Point p, int dx, int dy) {   /* p se pasa por referencia */
    p.x = p.x + dx;
    p.y = p.y + dy;
}

void soma_array(int arr[], int n, int resultado[]) {
    int i; int total;
    total = 0; i = 0;
    while (i < n) { total = total + arr[i]; i = i + 1; }
    resultado[0] = total;                            /* "retorno" mediante parámetro de salida */
}
```

## Funciones integradas (builtin)

Como no hay biblioteca estándar, toda interacción con el "sistema operativo" de la VM (E/S,
finalización del programa, lectura de teclado) se hace mediante funciones integradas reconocidas
directamente por el compilador (no necesitan — ni pueden — ser redeclaradas por el usuario).

| Firma | Código generado | Retorno |
|---|---|---|
| `void print(<dirección/cadena>);` | `sys print,<param>` (código 0) | — |
| `void debug(<dirección/cadena>);` | `sys debug,<param>` (código 3) — solo imprime en modo debug | — |
| `void halt(int <código>);` | `sys halt,<código>` (código 4) — detiene la VM | — |
| `void printf(<cadena de formato>, <valor>);` | `sys prints,<formato>` seguido de `sys printf,<valor>` | — |
| `int input(int <puerto>);` | `in <reg>,<puerto>` | valor leído del puerto |
| `void output(int <puerto>, int <valor>);` | `out <valor>,<puerto>` | — |
| `void delay(int <ms>);` | `delay <reg>` | — |
| `int getch(int <timeout>);` | `sys 5,<reg>` | carácter leído de stdin (`getchar()`), valor devuelto por la VM en `AX` |
| `int syscall(int <number>, int <param>);` | `sys <number>,<param>` | valor devuelto por la VM en `AX` |
| `void inputpin(<pin>);` | `sys 6,<reg>` | — |
| `void outputpin(<pin>);` | `sys 7,<reg>` | — |
| `void pulluppin(<pin>);` | `sys 8,<reg>` | — |

Notas:

- `printf` acepta **exactamente dos** argumentos (cadena de formato + un único valor/puntero) —
  no es variádico como el `printf` real; para imprimir varios valores hacen falta varias llamadas
  (ver el ejemplo de `fatorial.c` arriba).
- `getch` y `syscall` funcionan de la misma manera: el(los) argumento(s) se desapilan a
  registros, se emite la instrucción `sys`, y el valor que la rutina de syscall de la VM escribe
  en `AX` (vía `setregval(AX, ...)`) se adopta directamente como el valor de retorno de la
  función — no hay ningún `pop` adicional después del `sys`.
- `syscall(number, param)` permite invocar **cualquier** código de syscall (incluidos futuros
  códigos aún no cubiertos por una función integrada dedicada) sin necesidad de extender el
  compilador.
- `inputpin`/`outputpin`/`pulluppin` siguen el mismo patrón que `getch` (el argumento se desapila
  a un registro y se emite la instrucción `sys` con el código fijo 6/7/8), pero la rutina de
  syscall de la VM aún **no maneja** esos códigos (ninguna de las dos variantes, `vm-x86` ni
  `vm-iot`, tiene un `case` para 6/7/8 en `syscall.h`) — el ensamblador generado es correcto,
  pero la VM actual simplemente ignora la llamada (sin efecto, sin error).

## Limitaciones conocidas

- **Sin recursión real** — las variables locales no viven en una pila de activación (ver
  [Convención de llamada a función](#convención-de-llamada-a-función)).
- **Sin punteros** — solo arreglos y structs, siempre pasados por referencia cuando son
  parámetro de función; no hay operadores de puntero `*`/`&` ni aritmética de punteros.
- **Sin preprocesador** — no hay `#include`, `#define`, macros ni compilación condicional.
- **Sin `typedef`, `enum`, `union`** — solo `struct`.
- **Arreglos solo unidimensionales** y de tamaño fijo (sin `arr[N][M]`, sin asignación dinámica).
- **`printf` con un único valor por llamada** (sin varargs); la conversión de tipo
  (int/float/cadena) se decide en tiempo de ejecución según la máscara de formato (`%d`,
  `%f`/`%g`/`%e`, `%s`).
- **Ámbito simple de dos niveles** (global/local) — los bloques anidados no crean nuevos
  ámbitos; redeclarar un nombre dentro de la misma función es un error, incluso en bloques `{ }`
  diferentes.
- **Los structs/arreglos no pueden devolverse por valor** — el patrón usado es pasar un
  parámetro de "salida" y escribir el resultado en él (ver `soma_array` en
  `samples/estruturas.c`).
- **Watchdog (`WDC`/`EWD`/`DWD`/`RWD`) no implementado** — presente en la ISA para compatibilidad
  futura con la variante IoT, pero ignorado por el lenguaje C y por la VM de escritorio.
