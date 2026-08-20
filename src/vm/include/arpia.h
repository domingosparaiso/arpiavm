#ifndef ARPIA_H
#define ARPIA_H
#define MAXRAM 10240

#define AH 0
#define AL 1
#define AW 2
#define AX 3
#define BH 4
#define BL 5
#define BW 6
#define BX 7
#define CX 8
#define DX 9
#define SX 10
#define SP 11
#define AF 12
#define BF 13
#define CF 14
#define DF 15
#define IP 16
#define FLAGS 17
#define WDMAX 18
#define WDCOUNT 19

#define FLAG_ALL 0
#define FLAG_ZR 1
#define FLAG_CY 2
#define FLAG_OV 3
#define FLAG_GT 4
#define FLAG_EQ 5
#define FLAG_STR 6
#define FLAG_WDOG 7
#define FLAG_ADDR 8
#define FLAG_INT 9

// (A)
#define	VM_ADD        0x0000
#define	VM_SUB        0x1000
#define	VM_MUL        0x2000
#define	VM_DIV        0x3000
#define	VM_CMP        0x4000
#define	VM_MOV        0x5000
#define	VM_AND        0x6000
#define	VM_OR         0x7000
#define	VM_XOR        0x8000
#define	VM_SYS        0x9000
#define	VM_IN         0xA000
#define	VM_OUT        0xB000
// (B)
#define	VM_SHL        0xC000
#define	VM_SHR        0xC800
#define	VM_ROL        0xD000
#define	VM_ROR        0xD800
// (C)
#define	VM_CMPA       0xE000
#define	VM_MOVA       0xE400
#define	VM_SETINT     0xE800
// (D)
#define	VM_INC        0xFC00
#define	VM_DEC        0xFC40
#define	VM_PUSH       0xFC80
#define	VM_POP        0xFCC0
#define	VM_NOT        0xFD00
// (E)
#define	VM_LOOP       0xFD40
#define	VM_LOOPNZ     0xFD60
#define	VM_JGT        0xFD80
#define	VM_JGE        0xFDA0
#define	VM_JLT        0xFDC0
#define	VM_JLE        0xFDE0
#define	VM_JZ         0xFE00
#define	VM_JNZ        0xFE20
#define	VM_JC         0xFE40
#define	VM_JNC        0xFE60
#define	VM_JMP        0xFE80
#define	VM_CALL       0xFEA0
#define	VM_WDC        0xFEC0
#define	VM_INT        0xFEE0
#define	VM_DELAY      0xFF00
// SPARE INSTRUCTION SET
#define SPARE_1ST     0xFF20
#define SPARE_LAST    0xFFF0
// (F)
#define	VM_PUSHA      0xFFF1
#define	VM_POPA       0xFFF2
#define	VM_CLI        0xFFF3
#define	VM_STI        0xFFF4
#define	VM_RET        0xFFF5
#define	VM_IRET       0XFFF6
#define	VM_BIN        0XFFF7
#define	VM_STR        0XFFF8
#define	VM_RST        0XFFF9
#define	VM_SM20       0xFFFA
#define	VM_SM32       0xFFFB
#define	VM_EWD        0XFFFC
#define	VM_DWD        0XFFFD
#define	VM_RWD        0XFFFE
#define	VM_NOP        0xFFFF

#define P_IMEDIATO 0
#define P_REGISTRADOR 1
#define P_DIRETO 2
#define P_INDIRETO 3

#define MODE_32BITS 0
#define MODE_20BITS 1
#define MODE_16BITS 2
#define MODE_8BITS 3

#define DEBUGMODE 0

#define FALSE 0
#define TRUE 1
#define GETSTATUS 2

#define BIN_NAME 0
#define BIN_MAP 1
#define BIN_CODE 2
#define BIN_CRC 3

#define MAXLBL 1024
#define MAXRELOC 1024
#define MAXNOMELBL 64
#define MAXSTR 1024

#define REGTYPE_L 0
#define REGTYPE_H 1
#define REGTYPE_W 2
#define REGTYPE_X 3

#define NIBBLE1 1
#define NIBBLE2 2

#define PORT_ANALOG 0
#define PORT_DIGITAL 1

#define A1 0x0000
#define A2 0x0001
#define A3 0x0002

#define D1 0x1000
#define D2 0x1001
#define D3 0x1002

#define SYS_PRINT     0
#define SYS_PRINTS    1
#define SYS_PRINTF    2
#define SYS_DBGPRINT  3
#define SYS_HALT      4
#define SYS_GETCHAR   5
#define SYS_INPUTPIN  6
#define SYS_OUTPUTPIN 7
#define SYS_PULLUP    8

typedef struct {
	unsigned int mne;
	unsigned int e1;
	unsigned int e2;
	unsigned int p1;
	unsigned int p2;
	unsigned int v1;
	unsigned int v2;
	unsigned int size1;
	unsigned int size2;
} codigo;

typedef struct N {
	unsigned char h;
	unsigned char l;
} typeN;

typedef union {
	struct N n;
	unsigned short int w;
	unsigned int x;
} regX;

typedef union F {
	unsigned int i;
	float f;
} regFloat;

typedef struct {
	regX ax;
	regX bx;
	unsigned int cx;
	unsigned int dx;
	unsigned int sx;
	unsigned int sp;
	regFloat af;
	regFloat bf;
	regFloat cf;
	regFloat df;
	unsigned int ip;
	unsigned int flags;
	unsigned int wdmax;
	unsigned int wdcount;
} registradores;
#endif