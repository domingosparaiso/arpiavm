#define MAXRAM 10240

#define R1 0
#define R2 1
#define R3 2
#define R4 3
#define R5 4
#define R6 5
#define CT 6
#define F1 7
#define F2 8
#define F3 9
#define F4 10
#define SI 11
#define DI 12
#define SP 13
#define IP 14
#define FLAGS 15

#define FLAG_ALL 0
#define FLAG_ZR 1
#define FLAG_CY 2
#define FLAG_OV 3
#define FLAG_GT 4
#define FLAG_EQ 5
#define FLAG_STR 6
#define FLAG_WDOG 7
#define FLAG_ADDR 8

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
#define	SHL        0xC000
#define	SHR        0xC800
#define	ROL        0xD000
#define	ROR        0xD800
#define	CMPA       0xE000
#define	MOVA       0xE400
#define	INC        0xF800
#define	DEC        0xF840
#define	PUSH       0xF880
#define	POP        0xF8C0
#define	NOT        0xF900
#define	LOOP       0xF940
#define	LOOPNZ     0xF960
#define	JGT        0xF980
#define	JGE        0xF9A0
#define	JLT        0xF9C0
#define	JLE        0xF9E0
#define	JEQ        0xFA00
#define	JNE        0xFA20
#define	JC         0xFA40
#define	JNC        0xFA60
#define	JMP        0xFA80
#define	CALL       0xFAA0
#define	RET        0xFAC0
#define	NOP        0xFAC1
#define	SETFA      0XFAC2
#define	SETFS      0XFAC3
#define	RST        0XFAC4
#define	M20        0xFAC5
#define	M32        0xFAC6

#define P_IMEDIATO 0
#define P_REGISTRADOR 1
#define P_DIRETO 2
#define P_INDIRETO 3

#define MODE_8BITS 0
#define MODE_16BITS 1
#define MODE_32BITS 2

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

#define NIBBLE1 1
#define NIBBLE2 2