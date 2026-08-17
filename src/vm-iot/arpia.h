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
#define	ADD        0x0000
#define	SUB        0x1000
#define	MUL        0x2000
#define	DIV        0x3000
#define	CMP        0x4000
#define	MOV        0x5000
#define	AND        0x6000
#define	OR         0x7000
#define	XOR        0x8000
#define	SYS        0x9000
#define	IN         0xA000
#define	OUT        0xB000
// (B)
#define	SHL        0xC000
#define	SHR        0xC800
#define	ROL        0xD000
#define	ROR        0xD800
// (C)
#define	CMPA       0xE000
#define	MOVA       0xE400
#define	SETINT     0xE800
// (D)
#define	INC        0xFC00
#define	DEC        0xFC40
#define	PUSH       0xFC80
#define	POP        0xFCC0
#define	NOT        0xFD00
// (E)
#define	LOOP       0xFD40
#define	LOOPNZ     0xFD60
#define	JGT        0xFD80
#define	JGE        0xFDA0
#define	JLT        0xFDC0
#define	JLE        0xFDE0
#define	JZ         0xFE00
#define	JNZ        0xFE20
#define	JC         0xFE40
#define	JNC        0xFE60
#define	JMP        0xFE80
#define	CALL       0xFEA0
#define	WDC        0xFEC0
#define	INT        0xFEE0
#define	DELAY      0xFF00
// SPARE INSTRUCTION SET
#define SPARE_1ST  0xFF20
#define SPARE_LAST 0xFFF0
// (F)
#define	PUSHA      0xFFF1
#define	POPA       0xFFF2
#define	CLI        0xFFF3
#define	STI        0xFFF4
#define	RET        0xFFF5
#define	IRET       0XFFF6
#define	BIN        0XFFF7
#define	STR        0XFFF8
#define	RST        0XFFF9
#define	SM20       0xFFFA
#define	SM32       0xFFFB
#define	EWD        0XFFFC
#define	DWD        0XFFFD
#define	RWD        0XFFFE
#define	NOP        0xFFFF

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
#define SYS_DEBUG     3
#define SYS_HALT      4
#define SYS_GETCHAR   5
#define SYS_INPUTPIN  6
#define SYS_OUTPUTPIN 7
#define SYS_PULLUP    8
