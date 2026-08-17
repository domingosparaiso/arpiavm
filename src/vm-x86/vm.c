#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#ifdef _WIN32
	#include <conio.h>
#else
	#include "getch.h"
#endif
#include "vm.h"
#include "../vm-iot/arpia.h"
#include "../vm-iot/vm-arpia.h"
#include "syscall.h"

int debugmode = DEBUGMODE;

void set_debugmode(int mode) {
	debugmode = mode;
}

int get_debugmode() {
	return debugmode;
}

void print_str(const char *s) {
	printf("%s", s);
}

void print_num(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void a_print(const char *s) {
	printf("%s", s);
}

void a_printd(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void debug(unsigned char *msg) {
	if(debugmode) {
		print_str((const char*)msg);
		print_str("\n");
	}
}

void run() {
	reg.ip = 0;
	reg.sp = MAXRAM;
	haltsystem = FALSE;
	while(!haltsystem) {
		decode();
		if(debugmode) displaydebug();
		execute();
	}
}

const char * nomeinstrucao() {
	switch(cod.mne) {
		case ADD:
			return("ADD");
			break;
		case SUB:
			return("SUB");
			break;
		case MUL:
			return("MUL");
			break;
		case DIV:
			return("DIV");
			break;
		case CMP:
			return("CMP");
			break;
		case MOV:
			return("MOV");
			break;
		case AND:
			return("AND");
			break;
		case OR:
			return("OR");
			break;
		case XOR:
			return("XOR");
			break;
		case SYS:
			return("SYS");
			break;
		case IN:
			return("IN");
			break;
		case OUT:
			return("OUT");
			break;
		case SHL:
			return("SHL");
			break;
		case SHR:
			return("SHR");
			break;
		case ROL:
			return("ROL");
			break;
		case ROR:
			return("ROR");
			break;
		case CMPA:
			return("CMPA");
			break;
		case MOVA:
			return("MOVA");
			break;
		case SETINT:
			return("SETINT");
			break;
		case INC:
			return("INC");
			break;
		case DEC:
			return("DEC");
			break;
		case PUSH:
			return("PUSH");
			break;
		case POP:
			return("POP");
			break;
		case NOT:
			return("NOT");
			break;
		case LOOP:
			return("LOOP");
			break;
		case LOOPNZ:
			return("LOOPNZ");
			break;
		case JGT:
			return("JGT");
			break;
		case JGE:
			return("JGE");
			break;
		case JLT:
			return("JLT");
			break;
		case JLE:
			return("JLE");
			break;
		case JZ:
			return("JZ");
			break;
		case JNZ:
			return("JNZ");
			break;
		case JC:
			return("JC");
			break;
		case JNC:
			return("JNC");
			break;
		case JMP:
			return("JMP");
			break;
		case CALL:
			return("CALL");
			break;
		case WDC:
			return("WDC");
			break;
		case INT:
			return("INT");
			break;
		case DELAY:
			return("DELAY");
			break;
		case PUSHA:
			return("POPA");
			break;
		case CLI:
			return("CLI");
			break;
		case STI:
			return("STI");
			break;
		case RET:
			return("RET");
			break;
		case IRET:
			return("IRET");
			break;
		case BIN:
			return("BIN");
			break;
		case STR:
			return("STR");
			break;
		case RST:
			return("RST");
			break;
		case SM20:
			return("SM20");
			break;
		case SM32:
			return("SM32");
			break;
		case EWD:
			return("EWD");
			break;
		case DWD:
			return("DWD");
			break;
		case RWD:
			return("RWD");
			break;
		case NOP:
			return("NOP");
			break;
	}
	return("???");
}

int loadram(char *nomearquivo) {
	FILE *f;
	int c;
	int d = 0;
	int state = 0;
	unsigned int fcrc = 0;
	unsigned int ccrc = 0;
	unsigned int i;

	if(debugmode) printf("Loading file \"%s\"...\n", nomearquivo);
	f=fopen(nomearquivo, "r");
	if(!f) {
		printf("Fail loading bytecode file.\n");
		exit(1);
	}
	maxmem = 0;
	i = 0;
	while(!feof(f)) {
		if((c = fgetc(f)) == EOF) break;
		if(c == '\n') {
			i = 0;
			state++;
		} else {
			switch(state) {
				case BIN_NAME:
					if(i < 128) {
						progname[i++] = c;
					}
					break;
				case BIN_MAP:
					if(i < 1024) {
						progmap[i++] = c;
					}
					break;
				case BIN_CODE:
					if(maxmem <= MAXRAM) {
						if(c != ' ') {
							c = c-'0'; if(c > 9) c -= 7; if(c > 15) c -= 32;
							if(d == 0) {
								d = 1;
								i = c;
							} else {
								d = 0;
								i = (i << 4) + c;
								ccrc = (ccrc + i) & 0xFFFF;
								memory[maxmem++]=(unsigned char)(i & 0xff);
								if(debugmode) printf("%02X ", memory[maxmem-1]);
							}
						}
					}
					break;
				case BIN_CRC:
					c=c-'0'; if(c>9) c-=7; if(c>15) c-=32;
					fcrc=(fcrc<<4)+c;
					break;
			}
		}
	}
	fclose(f);
	if(debugmode) printf("\n");
	if(ccrc == fcrc) {
		if(debugmode) {
			printf("File \"%s\" loaded into memory.\n", progname);
			printf("Map: %s\n", progmap);
			printf("CRC: OK\n");
		}
		return(1);
	} else {
		printf("CRC Fail: Read=%04X Calc=%04X\n", fcrc, ccrc);
		return(0);
	}
}

void displayend(int e, int p, int v) {
	char *nomereg[] = { "AH","AL","AW","AX","BH","BL","BW","BX","CX","DX","SX","SP","AF","BF","CF","DF","FLAGS","IP"};
	switch(e) {
		case P_IMEDIATO:
			print_num("%05X", (p << 16) | v);
			break;
		case P_REGISTRADOR:
			print_str(nomereg[p]);
			break;
		case P_DIRETO:
			print_num("[%05X]", (p << 16) | v);
			break;
		case P_INDIRETO:
			print_num("[%s]", nomereg[p]);
			break;
	}
}

void dumpmem() {
	unsigned int i;
	unsigned int c;
	unsigned int l;
	unsigned int e;
	
	for(l=0;l<=(maxmem/16);l++) {
		printf("%04X: ", l*16);
		for(c=0;c<16;c++) {
			e=l*16+c;
			if(e<maxmem) {
				i=getmemval(e, MODE_8BITS);
				a_printd("%02X ", i);
			} else {
				a_print("   ");
			}
		}
		for(c=0;c<16;c++) {
			e=l*16+c;
			if(e<maxmem) {
				i=getmemval(e, MODE_8BITS);
				if(i>=32 && i<=127) {
					printf("%c", i);
				} else {
					printf(".");
				}
			}
		}
		printf("\n");
	}
	printf("***** Stack SP=%04X *****\n", reg.sp);
	for(c=reg.sp;c<MAXRAM;c++) {
		i=getmemval(c, MODE_8BITS);
		printf("%02X ", i);
	}
	printf("\n");
}

void displaydebug() {
	int c;
	printf("AX=%04X BX=%04X CX=%04X DX=%04X SX=%04X SP=%04X\n", reg.ax.x, reg.bx.x, reg.cx, reg.dx, reg.sx, reg.sp);
	printf("AF=%04X BF=%04X CF=%04X DF=%04X\n", reg.af.i, reg.bf.i, reg.cf.i, reg.df.i);
	printf("Flags: ZR=%01d CY=%01d OV=%01d GT=%01d ST=%01d WD=%01d AM=%01d IN=%01d IP=%04X Mode: ", getflag(FLAG_ZR),getflag(FLAG_CY),getflag(FLAG_OV),getflag(FLAG_GT),getflag(FLAG_STR),getflag(FLAG_WDOG),getflag(FLAG_ADDR),getflag(FLAG_INT), reg.ip);
	printf("%s ", nomeinstrucao());
	if(cod.mne < 0xff20) displayend(cod.e1, cod.p1, cod.v1);
	if(cod.mne < 0xfc00) {
		printf(",");
		displayend(cod.e2, cod.p2, cod.v2);
	}
	printf("\n>");
	c=0;
	int display_info = 0;
	while(display_info == 0) {
		display_info = 0;
		while(c!=0x2A) {
			c=getchar()|0x20;
			if(c=='d') display_info = 1;
			if(c=='q') haltsystem=TRUE;
			if(c=='g') debugmode=FALSE;
			if(c=='h') display_info = 2;
		}
		if(display_info > 0) {
			if(display_info == 1) dumpmem();
			if(display_info == 2) printf("d - memory dump\nq - quit\ng - go, run without debug\n");
		}
	}

}

int main(int argc, char **argv) {
	int c;
	int f = 0;
	for(c=1;c<argc;c++) {
		if(*argv[c] == '-') {
			switch(*(argv[c]+1)) {
				case 'h':
				case '?':
					printf("VM Arpia\nHelp\n\n-d ..... Debug mode\n-v ..... Version\n-a ..... ASCII file\n-b ..... Binary file (default)\n<filename>   Bytecode file.\n\n");
					break;
				case 'd':
					debugmode = TRUE;
					break;
				case 'v':
					printf("Arpia VM. Version 0.01\nby Domingos Paraiso\n\n");
					break;
			}
		} else {
			f = c;
		}
	}
	if(f > 0) {
		loadram(argv[f]);
		if(debugmode) printf("Starting VM.\n");
		run();
	} else {
		printf("ERROR: File not specified.\n");
	}
}
