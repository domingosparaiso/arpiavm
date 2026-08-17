# ARPIA VM

*Lire dans d'autres langues : [Português](README.md) · [English](README-en.md) · [Español](README-es.md)*

Chaîne d'outils complète pour la **VM ARPIA** : une machine virtuelle 32 bits (inspirée de
l'architecture 8086, mais avec des registres 32 bits), un **assembleur** (`asm`) et un
**compilateur d'un sous-ensemble de C** (`c`) qui génère du code pour cette VM.

```
programme.c --[c.exe]--> programme.asm --[asm.exe]--> programme.bin --[vm.exe]--> exécution
```

La définition formelle des mnémoniques, du format binaire et des registres se trouve dans
`doc/VM ARPIA.xlsx` (onglets `Description`, `Instruction Set` et `Register`). Ce document résume
ce qu'il faut savoir pour utiliser et étendre l'assembleur et le compilateur.

## Sommaire

- [Structure du dépôt](#structure-du-dépôt)
- [Compiler les outils](#compiler-les-outils)
- [Utilisation des outils](#utilisation-des-outils)
- [La VM ARPIA](#la-vm-arpia)
- [VM IoT (vm-iot.ino)](#vm-iot-vm-iotino)
- [L'assembleur (asm)](#lassembleur-asm)
- [Le compilateur C](#le-compilateur-c)
- [Fonctions intégrées (builtin)](#fonctions-intégrées-builtin)
- [Limitations connues](#limitations-connues)

## Structure du dépôt

```
src/
  asm/            source de l'assembleur (asm.l / asm.y, flex + bison)
  c/              source du compilateur C (c.l / c.y, flex + bison)
  vm-x86/         VM pour ordinateur de bureau (vm.c, syscall.h) — Windows/Linux/Mac
  vm-iot/         VM pour microcontrôleur (arpia.h avec les constantes de l'ISA,
                  vm-iot.ino, syscall.h) — variante Arduino/IoT, avec un
                  minishell série, WiFi et un serveur web (LittleFS)
  Makefile        construit asm.exe / c.exe / vm.exe
bin/              exécutables générés (asm.exe, c.exe, vm.exe)
samples/          exemples en assembleur (.asm) et en C (.c), avec leurs .bin
doc/VM ARPIA.xlsx spécification complète de l'ISA (mnémoniques, encodage, registres)
```

`src/vm-iot/arpia.h` est la source unique des constantes de l'ISA (opcodes, registres, modes
d'adressage, codes de syscall) et est inclus à la fois par l'assembleur et par les deux variantes
de la VM.

## Compiler les outils

Nécessite **flex** et **bison** (sous Windows, les binaires `win_flex.exe` / `win_bison.exe` du
paquet [winflexbison](https://github.com/lexxmark/winflexbison) conviennent) et un `gcc`
compatible MinGW.

```
cd src
make asm     # génère bin/asm.exe
make c       # génère bin/c.exe
make vm      # génère bin/vm.exe
```

> **Remarque :** la cible `vm` du `Makefile` pointe encore vers `./vm/vm.c`, qui a été remplacé
> par `./vm-x86/vm.c` (la VM de bureau) — cette cible du Makefile est obsolète. Pour recompiler
> la VM manuellement :
> ```
> cd src/vm-x86
> gcc -o ../../bin/vm.exe vm.c
> ```

Si `bison`/`flex` ne sont pas dans le `PATH`, générez les fichiers manuellement avant `gcc` :

```
cd src/c   # ou src/asm
win_bison -d c.y      # génère c.tab.c / c.tab.h
win_flex c.l          # génère lex.yy.c
gcc -o ../../bin/c.exe c.tab.c
```

## Utilisation des outils

Les trois outils acceptent un fichier d'entrée positionnel et `-o <fichier>` pour la sortie ;
sans ces arguments, ils utilisent stdin/stdout.

```
bin/c.exe   -o programme.asm  programme.c     # compile C -> assembleur ARPIA
bin/asm.exe -o programme.bin  programme.asm   # assemble -> binaire
bin/vm.exe  programme.bin                     # exécute le binaire
bin/vm.exe  -d programme.bin                  # exécute en mode debug (pas à pas)
bin/vm.exe  -v                                # version
```

## La VM ARPIA

### Registres

| Registre | Taille | Description |
|---|---|---|
| `AH`, `AL` | 8 bits | moitiés haute/basse de `AW` |
| `AW` | 16 bits | moitié basse de `AX` |
| `AX` | 32 bits | accumulateur principal |
| `BH`, `BL`, `BW`, `BX` | 8/8/16/32 bits | même structure que AX, registre secondaire |
| `CX`, `DX`, `SX`, `SP` | 32 bits | registres généraux / pointeur de pile |
| `AF`, `BF`, `CF`, `DF` | 32 bits | registres de **virgule flottante** (float) |
| `IP` | 32 bits | pointeur d'instruction |
| `FLAGS` | — | voir les indicateurs ci-dessous |

Il n'existe aucune instruction `FLOAT` implémentée dans la VM : les valeurs `float` sont toujours
manipulées via les registres `AF/BF/CF/DF`, et les instructions arithmétiques (`ADD`, `SUB`,
`MUL`, `DIV`, `CMP`, `MOV`, ...) traitent automatiquement ces registres comme des flottants.

Indicateurs (bits de `FLAGS`) : `ZR` (zéro), `CY` (retenue), `OV` (débordement), `GT` (supérieur),
`EQ` (égal), `STR` (mode chaîne), `WDOG` (watchdog — non implémenté), `ADDR` (mode d'adressage
courant), `INT` (interruptions activées).

### Modes d'adressage

- **Immédiat** — valeur littérale ou label (`mov ax,10`).
- **Registre** — `mov ax,bx`.
- **Direct** — `mov ax,[label]` / `mov ax,(1234)`, accède au contenu d'une adresse mémoire.
- **Indirect** — `mov ax,(bx)`, accède au contenu de l'adresse contenue dans un registre.

`SM20` sélectionne l'adressage direct sur 20 bits (l'adresse est en partie intégrée dans le
mnémonique lui-même) ; `SM32` sélectionne l'adressage complet sur 32 bits (4 octets après le
mnémonique). Les valeurs immédiates utilisent toujours 32 bits.

### Format du binaire généré

`printcode()` (dans `asm.y`) émet : le nom du programme, la table `MAP` (optionnelle) et le dump
hexadécimal du code+données (section `.code` suivie de `.data`), terminé par un CRC16 simple.

## VM IoT (vm-iot.ino)

Source : `src/vm-iot/vm-iot.ino` (+ `arpia.h`, `vm-arpia.h`, `syscall.h`). Sketch Arduino pour
ESP32/ESP8266 qui exécute la même ISA que la VM ARPIA, en stockant les programmes dans un système
de fichiers **LittleFS** local à la carte.

### Minishell série

Au démarrage, le sketch monte le LittleFS, se connecte au WiFi et ouvre une console de commandes
sur le port série (115200 bps) :

| Commande | Effet |
|---|---|
| `ls` / `dir` | liste les fichiers du LittleFS (nom et taille) |
| `ver` / `version` | affiche la version du firmware |
| `help` / `?` | affiche la liste des commandes |
| `format` | formate le LittleFS |
| `rm` / `del <fichier>` | supprime un fichier |
| `cat` / `type <fichier>` | affiche le contenu du fichier |
| `dump <fichier>` | affiche le dump hexadécimal du fichier |
| `cls` / `clear` | efface l'écran du terminal |
| `term` | active/désactive l'affichage des codes de touches reçus (debug) |
| `<fichier>` | charge et exécute le fichier comme programme ARPIA |

Il n'y a plus de commande `upload` par le terminal série — l'envoi de fichiers vers le LittleFS se
fait désormais par l'interface web (voir ci-dessous).

### WiFi et interface web

Dans `setup()`, le sketch tente de se connecter en tant que station au réseau défini par
`WIFI_SSID`/`WIFI_PASSWORD` (constantes en haut de `vm-iot.ino`, modifiables ou surchargeables via
un flag de compilation) ; si la connexion échoue en 15 secondes, il démarre son propre point
d'accès (`ArpiaVM`, mot de passe `arpiavm123`) pour garantir l'accès à l'interface même sans réseau
configuré. L'adresse IP obtenue (réseau local ou point d'accès) est affichée sur le port série et
dans la commande `help`.

Un serveur web intégré (`WebServer` sur ESP32 / `ESP8266WebServer` sur ESP8266) sert une page à
`http://<ip>/` qui liste les fichiers du LittleFS avec :

- **Suppression** d'un fichier existant.
- **Upload** d'un nouveau fichier (formulaire multipart, écrit directement sur le LittleFS).

Le serveur web continue de répondre même pendant que le minishell attend une commande sur le port
série.

## L'assembleur (asm)

Source : `src/asm/asm.l` (lexique) + `src/asm/asm.y` (grammaire, bison).

### Structure d'un programme assembleur

```asm
.name "nom_du_programme"

.code
  <instructions>

.data
  <déclarations de données>

.end
```

### Groupes de mnémoniques

L'opcode de 16 bits intègre le mnémonique et les modes d'adressage des opérandes. Les
mnémoniques sont regroupés par « forme » d'instruction (`MNEM_A` .. `MNEM_F`) :

| Groupe | Mnémoniques | Opérandes |
|---|---|---|
| A | `ADD SUB MUL DIV CMP MOV AND OR XOR SYS IN OUT` | `<dest>, <source>` — toute combinaison registre/immédiat/direct/indirect |
| B | `SHL SHR ROL ROR` | `<dest_reg\|direct>, <reg\|immédiat>` |
| C | `CMPA MOVA SETINT` | `<reg\|immédiat>, <reg\|immédiat>` |
| D | `INC DEC PUSH POP NOT` | un opérande (registre/immédiat/direct/indirect) |
| E | `LOOP LOOPNZ JGT JGE JLT JLE JZ JNZ JC JNC JMP CALL WDC INT DELAY` | un opérande (label/registre) |
| F | `PUSHA POPA CLI STI RET IRET BIN STR RST SM20 SM32 EWD DWD RWD NOP` | sans opérande |

### Données (section `.data`)

`DB` (octet), `DW` (mot/16 bits), `DD` (double mot/32 bits), `FL` (flottant/32 bits, IEEE-754
sous forme d'entier), listes séparées par des virgules, chaînes entre guillemets, `?` pour une
position indéfinie (réservée, mise à zéro) et `<n> DUP(?)` pour réserver `n` positions
indéfinies.

```asm
.data
msg:      db "Hello, world!",13,10,0
compteur: dw 0
buffer:   db dup(64) ?
```

### `SYS` et codes de syscall

`SYS <code>,<param>` invoque une routine du « système d'exploitation » de la VM. `<code>` peut
être un nombre littéral ou l'un des mots-clés reconnus par le lexique de l'assembleur :

| Code | Mot-clé | Effet |
|---|---|---|
| 0 | `PRINT` | affiche la chaîne pointée par `<param>` (adresse) |
| 1 | `PRINTS` | définit le masque/format utilisé par le `PRINTF` suivant |
| 2 | `PRINTF` | affiche en utilisant le masque défini par `PRINTS`, avec `<param>` comme valeur |
| 3 | `DEBUG` | comme `PRINT`, mais n'affiche que si la VM est en mode debug |
| 4 | `HALT` | arrête l'exécution de la VM |
| 5 | — | lit un caractère depuis stdin (`getchar()`) et écrit la valeur lue dans `AX` |
| 6 | — | configure `<param>` comme broche d'entrée (`inputpin`) — pas encore implémenté dans la VM |
| 7 | — | configure `<param>` comme broche de sortie (`outputpin`) — pas encore implémenté dans la VM |
| 8 | — | configure `<param>` comme broche d'entrée avec pull-up (`pulluppin`) — pas encore implémenté dans la VM |

`SYSCALL` **n'est pas** un mnémonique distinct de l'assembleur — `SYS` couvre tous les codes
ci-dessus et tout code numérique supplémentaire que la VM viendrait à implémenter.

## Le compilateur C

Source : `src/c/c.l` (lexique) + `src/c/c.y` (grammaire, bison). Il compile un sous-ensemble
simplifié de C directement en assembleur ARPIA (aucune bibliothèque standard n'est utilisée —
tout est généré via les [fonctions intégrées](#fonctions-intégrées-builtin) qui correspondent à
des instructions/syscalls de la VM).

### Types pris en charge

| Type | Taille | Remarque |
|---|---|---|
| `char` | 8 bits | |
| `int` | 16 bits | |
| `long` | 32 bits | |
| `float` | 32 bits | géré via les registres `AF/BF/CF/DF` |
| `void` | — | uniquement comme type de retour d'une fonction |
| `struct <nom> { ... };` | somme des champs | pas de `union`, `enum`, `typedef` |
| `type nom[N];` | tableau de taille fixe, unidimensionnel | pas de tableaux multidimensionnels |

Il n'existe pas de pointeurs explicites (`*`/`&` comme opérateurs de type). Les tableaux et
structures, lorsqu'ils sont utilisés comme **paramètre de fonction**, sont toujours passés
implicitement **par référence** (adresse 32 bits) — il n'est ni nécessaire ni possible de
déclarer `struct Foo *p`.

### Déclarations et portée

- Globales : `type nom;`, `type nom[N];`, `type nom = <littéral>;` (entier ou flottant).
- Locales : identique aux globales, plus `type nom = <expression>;`.
- `struct Nom { champ; ... };` au niveau global, avant utilisation.
- Prototypes : `type nom(paramètres);` (sans corps, terminé par `;`).
- Chaque programme a besoin d'une fonction `main` (le runtime génère `call F_main` suivi de
  `sys halt,0`).

### Contrôle de flux et expressions

- `if` / `else`, `while`, `for (init; cond; incr)`, `break`, `continue`, `return [expr];`.
- Opérateurs arithmétiques `+ - * / %`, bit à bit `& | ^ ~ << >>`, logiques `&& || !`,
  relationnels `== != < > <= >=`, affectations composées `+= -= *= /= %= &= |= ^= <<= >>=`,
  `++`/`--` (préfixés uniquement : `++x`, `--x`).
- Accès aux tableaux `arr[i]`, aux champs de structure `s.champ`, appels de fonction `f(a, b)`.
- Commentaires `/* ... */` et `// ...`.

### Convention d'appel de fonction

- Les arguments sont évalués de gauche à droite et empilés (`push`) ; la fonction appelée les
  dépile dans l'ordre inverse pour correspondre aux paramètres déclarés.
- La valeur de retour est laissée dans `AX` (ou `AF` pour `float`) avant le `ret`.
- **Il n'y a pas de pile d'activation (stack frame) :** chaque variable locale reçoit une adresse
  mémoire fixe (label `V<n>`) générée à la compilation, exactement comme une variable globale.
  Cela signifie que **les appels récursifs ne sont pas pris en charge** — un appel récursif
  écrase les variables locales de l'appel en cours.

Exemple (`samples/fatorial.c`) :

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

Structures et tableaux en paramètre (`samples/estruturas.c`) :

```c
struct Point { int x; int y; };

void movepoint(struct Point p, int dx, int dy) {   /* p est passé par référence */
    p.x = p.x + dx;
    p.y = p.y + dy;
}

void soma_array(int arr[], int n, int resultado[]) {
    int i; int total;
    total = 0; i = 0;
    while (i < n) { total = total + arr[i]; i = i + 1; }
    resultado[0] = total;                            /* « retour » via un paramètre de sortie */
}
```

## Fonctions intégrées (builtin)

Comme il n'y a pas de bibliothèque standard, toute interaction avec le « système d'exploitation »
de la VM (E/S, fin de programme, lecture clavier) passe par des fonctions intégrées reconnues
directement par le compilateur (elles ne peuvent — et n'ont pas besoin — d'être redéclarées par
l'utilisateur).

| Signature | Code généré | Valeur de retour |
|---|---|---|
| `void print(<adresse/chaîne>);` | `sys print,<param>` (code 0) | — |
| `void debug(<adresse/chaîne>);` | `sys debug,<param>` (code 3) — n'affiche qu'en mode debug | — |
| `void halt(int <code>);` | `sys halt,<code>` (code 4) — arrête la VM | — |
| `void printf(<chaîne de format>, <valeur>);` | `sys prints,<format>` suivi de `sys printf,<valeur>` | — |
| `int input(int <port>);` | `in <reg>,<port>` | valeur lue sur le port |
| `void output(int <port>, int <valeur>);` | `out <valeur>,<port>` | — |
| `void delay(int <ms>);` | `delay <reg>` | — |
| `int getch(int <timeout>);` | `sys 5,<reg>` | caractère lu depuis stdin (`getchar()`), valeur renvoyée par la VM dans `AX` |
| `int syscall(int <number>, int <param>);` | `sys <number>,<param>` | valeur renvoyée par la VM dans `AX` |
| `void inputpin(<pin>);` | `sys 6,<reg>` | — |
| `void outputpin(<pin>);` | `sys 7,<reg>` | — |
| `void pulluppin(<pin>);` | `sys 8,<reg>` | — |

Remarques :

- `printf` accepte **exactement deux** arguments (chaîne de format + une seule valeur/pointeur) —
  ce n'est pas variadique comme le vrai `printf` ; pour afficher plusieurs valeurs, il faut
  plusieurs appels (voir l'exemple `fatorial.c` ci-dessus).
- `getch` et `syscall` fonctionnent de la même manière : le(s) argument(s) sont dépilés vers des
  registres, l'instruction `sys` est émise, et la valeur que la routine de syscall de la VM écrit
  dans `AX` (via `setregval(AX, ...)`) est directement adoptée comme valeur de retour de la
  fonction — il n'y a aucun `pop` supplémentaire après le `sys`.
- `syscall(number, param)` permet d'invoquer **n'importe quel** code de syscall (y compris de
  futurs codes non encore couverts par une fonction intégrée dédiée) sans avoir à étendre le
  compilateur.
- `inputpin`/`outputpin`/`pulluppin` suivent le même schéma que `getch` (l'argument est dépilé
  vers un registre et l'instruction `sys` est émise avec le code fixe 6/7/8), mais la routine de
  syscall de la VM ne traite **pas encore** ces codes (aucune des deux variantes, `vm-x86` ni
  `vm-iot`, n'a de `case` pour 6/7/8 dans `syscall.h`) — l'assembleur généré est correct, mais la
  VM actuelle ignore simplement l'appel (aucun effet, aucune erreur).

## Limitations connues

- **Pas de vraie récursion** — les variables locales ne vivent pas sur une pile d'activation
  (voir [Convention d'appel de fonction](#convention-dappel-de-fonction)).
- **Pas de pointeurs** — seulement des tableaux et des structures, toujours passés par référence
  lorsqu'ils sont paramètre de fonction ; il n'y a pas d'opérateurs de pointeur `*`/`&` ni
  d'arithmétique de pointeurs.
- **Pas de préprocesseur** — pas de `#include`, `#define`, macros ou compilation conditionnelle.
- **Pas de `typedef`, `enum`, `union`** — seulement `struct`.
- **Tableaux unidimensionnels uniquement** et de taille fixe (pas de `arr[N][M]`, pas
  d'allocation dynamique).
- **`printf` avec une seule valeur par appel** (pas de varargs) ; la conversion de type
  (int/float/chaîne) est décidée à l'exécution par le masque de format (`%d`, `%f`/`%g`/`%e`,
  `%s`).
- **Portée simple à deux niveaux** (global/local) — les blocs imbriqués ne créent pas de
  nouvelles portées ; redéclarer un nom dans la même fonction est une erreur, même dans des blocs
  `{ }` différents.
- **Les structures/tableaux ne peuvent pas être retournés par valeur** — le motif utilisé est de
  passer un paramètre de « sortie » et d'y écrire le résultat (voir `soma_array` dans
  `samples/estruturas.c`).
- **Watchdog (`WDC`/`EWD`/`DWD`/`RWD`) non implémenté** — présent dans l'ISA pour une
  compatibilité future avec la variante IoT, mais ignoré par le langage C et par la VM de bureau.
