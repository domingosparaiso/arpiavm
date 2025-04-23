#include <stdio.h>
#include <stdlib.h>
#include "../include/arpia.h"
#include "../sys/system.h"

typedef struct {
	unsigned int mne;
	unsigned int e1;
	unsigned int e2;
	unsigned int p1;
	unsigned int p2;
	unsigned int v1;
	unsigned int v2;
} codigo;

typedef struct {
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int ct;
	unsigned int f1;
	unsigned int f2;
	unsigned int f3;
	unsigned int f4;
	unsigned int si;
	unsigned int di;
	unsigned int sp;
	unsigned int ip;
	unsigned int flags;
	unsigned int mode;
} registradores;

codigo cod;
registradores reg;
int needdump = TRUE;
unsigned char memory[MAXRAM];
unsigned int haltsystem = TRUE;
unsigned int maxmem;
char progname[128];
char map[1024];
int debugmode = DEBUGMODE;
unsigned char *printfmask;

void debug(unsigned char *msg) {
	if(debugmode) {
		printf(msg);
		printf("\n");
	}
}

unsigned int getregval(unsigned int nreg) {
	switch(nreg) {
		case R1:
			return(reg.r1);
			break;
		case R2:
			return(reg.r2);
			break;
		case R3:
			return(reg.r3);
			break;
		case R4:
			return(reg.r4);
			break;
		case R5:
			return(reg.r5);
			break;
		case R6:
			return(reg.r6);
			break;
		case CT:
			return(reg.ct);
			break;
		case F1:
			return(reg.f1);
			break;
		case F2:
			return(reg.f2);
			break;
		case F3:
			return(reg.f3);
			break;
		case F4:
			return(reg.f4);
			break;
		case SI:
			return(reg.si);
			break;
		case DI:
			return(reg.di);
			break;
		case SP:
			return(reg.sp);
			break;
		case IP:
			return(reg.ip);
			break;
		case FLAGS:
			return(reg.flags);
			break;
	}
}

void setregval(unsigned int nreg, unsigned int value) {
	switch(nreg) {
		case R1:
			reg.r1 = value;
			break;
		case R2:
			reg.r2 = value;
			break;
		case R3:
			reg.r3 = value;
			break;
		case R4:
			reg.r4 = value;
			break;
		case R5:
			reg.r5 = value;
			break;
		case R6:
			reg.r6 = value;
			break;
		case CT:
			reg.ct = value;
			break;
		case F1:
			reg.f1 = value;
			break;
		case F2:
			reg.f2 = value;
			break;
		case F3:
			reg.f3 = value;
			break;
		case F4:
			reg.f4 = value;
			break;
		case SI:
			reg.si = value;
			break;
		case DI:
			reg.di = value;
			break;
		case SP:
			reg.sp = value;
			break;
		case IP:
			reg.ip = value;
			break;
	}
}

unsigned int getmemvalmode(unsigned int pointer, int mode) {
	unsigned int r;
	unsigned char c;
	
	c = memory[pointer];
	r = c;
	if(mode == MODE_16BITS || mode == MODE_32BITS) {
		c = memory[pointer+1];
		r = (r<<8) | c;
		if(mode == MODE_32BITS) {
			c = memory[pointer+2];
			r = (r<<8) | c;
			c = memory[pointer+3];
			r = (r<<8) | c;
		}
	}
	return(r);
}

void setmemvalmode(unsigned int pointer, unsigned int value, int mode) {	
	switch(mode) {
		case MODE_8BITS:
			memory[pointer] = (unsigned char)(value&0xff);
			break;
		case MODE_16BITS:
			memory[pointer] = (unsigned char)((value>>8)&0xff);
			memory[pointer+1] = (unsigned char)(value&0xff);
			break;
		case MODE_32BITS:
			memory[pointer] = (unsigned char)((value>>24)&0xff);
			memory[pointer+1] = (unsigned char)((value>>16)&0xff);
			memory[pointer+2] = (unsigned char)((value>>8)&0xff);
			memory[pointer+3] = (unsigned char)(value&0xff);
			break;
	}
}

unsigned int getmemval(unsigned int pointer) {
	return(getmemvalmode(pointer, reg.mode));
}

void setmemval(unsigned int pointer, unsigned int value) {
	setmemvalmode(pointer, value, reg.mode);
}

unsigned int getcodigo() {
	unsigned int w;
	
	w = getmemvalmode(reg.ip, MODE_16BITS);
	reg.ip += 2;
	return(w);
}

void decode() {
	unsigned int i;
	cod.e1 = -1;
	cod.e2 = -1;
	cod.v1 = 0;
	cod.v2 = 0;
	i = getcodigo();
	if(i < SHL) {
		// A
		cod.mne = i & 0xf000;
		cod.e1 = (i&0x0c00)>>10;
		cod.e2 = (i&0x0300)>>8;
		cod.p1 = (i&0x00f0)>>4;
		cod.p2 = (i&0x000f);
	} else {
		if(i < CMPS) {
			// B
			cod.mne = i & 0xf800;
			cod.e1 = (i&0x0600)>>9;
			cod.e2 = (i&0x0100)>>8;
			cod.p1 = (i&0x00f0)>>4;
			cod.p2 = (i&0x000f);
		} else {
			if(i < INC) {
				// C
				cod.mne = i & 0xfc00;
				cod.e1 = (i&0x0200)>>9;
				cod.e2 = (i&0x0100)>>8;
				cod.p1 = (i&0x00f0)>>4;
				cod.p2 = (i&0x000f);
			} else {
				if(i < LOOP) {
					// D
					cod.mne = i & 0xffc0;
					cod.e1 = (i&0x0030)>>4;
					cod.p1 = (i&0x000f);
				} else {
					if(i < RET) {
						// E
						cod.mne = i & 0xffe0;
						cod.e1 = (i&0x0010)>>4;
						cod.p1 = (i&0x000f);
					} else {
						if(i <= M32) {
							// F
							cod.mne = i;
						} else {
							// indefinido, converter em NOP
							cod.mne = NOP;
						}
					}
				}
			}
		}
	}
	if(cod.e1 == P_IMEDIATO || cod.e1 == P_DIRETO) cod.v1  = (cod.p1 << 16) | getcodigo();
	if(cod.e2 == P_IMEDIATO || cod.e2 == P_DIRETO) cod.v2  = (cod.p2 << 16) | getcodigo();
}

void push32(unsigned int val) {
		reg.sp -= 4;
		setmemvalmode(reg.sp, val, MODE_32BITS);
}

void push16(unsigned int val) {
		reg.sp -= 2;
		setmemvalmode(reg.sp, val, MODE_16BITS);
}

void push(unsigned int val) {
	if(reg.mode == MODE_32BITS) {
		push32(val);
	} else {
		push16(val);
	}
}

int stack_overflow() {
	if(reg.sp >= MAXRAM) {
		reg.sp = MAXRAM+1;
		return(1);
	} else {
		return(0);
	}
}

unsigned int pop32() {
	unsigned int r = 0;
	if(!stack_overflow()) {
		r = getmemvalmode(reg.sp, MODE_32BITS);
		reg.sp += 4;
	}
	return(r);
}

unsigned int pop16() {
	unsigned int r = 0;
	if(!stack_overflow()) {
		r = getmemvalmode(reg.sp, MODE_16BITS);
		reg.sp += 2;
	}
	return(r);
}

unsigned int pop() {
	if(reg.mode == MODE_32BITS) {
		return(pop32());
	} else {
		return(pop16());
	}
}

void setflag(unsigned int flag, unsigned int value) {
	if(flag == FLAG_ALL) {
		reg.flags = value;
	} else {
		reg.flags = reg.flags&(~(1<<flag));
		reg.flags = reg.flags|(value<<flag);
	}
}

unsigned int getflag(unsigned int flag) {
	return((reg.flags&(1<<flag))?1:0);
}

#include "../sys/system.c"

void execute() {
	unsigned int storeresult = FALSE;
	unsigned int r = 0;
	unsigned int p1;
	unsigned int p2;
	unsigned int salvamemoria;

	switch(cod.e1) {
		case P_IMEDIATO:
			p1 = cod.v1;
			break;
		case P_REGISTRADOR:
			p1 = getregval(cod.p1);
			break;
		case P_DIRETO:
			salvamemoria = cod.v1;
			p1 = getmemval(cod.v1);
			break;
		case P_INDIRETO:
			salvamemoria = getregval(cod.p1);
			p1 = getmemval(salvamemoria);
			break;
	}
	switch(cod.e2) {
		case P_IMEDIATO:
			p2 = cod.v2;
			break;
		case P_REGISTRADOR:
			p2 = getregval(cod.p2);
			break;
		case P_DIRETO:
			p2 = getmemval(cod.v2);
			break;
		case P_INDIRETO:
			p2 = getmemval(getregval(cod.p2));
			break;
	}
	switch(cod.mne) {
		case ADD:
			r = p1+p2;
			storeresult = TRUE;
//			debug("ADD");
			break;
		case SUB:
			r = p1-p2;
			storeresult = TRUE;
//			debug("SUB");
			break;
		case MUL:
			r = p1*p2;
			storeresult = TRUE;
//			debug("MUL");
			break;
		case DIV:
			r = p1/p2;
			reg.r7 = p1%p2;
			storeresult = TRUE;
//			debug("DIV");
			break;
		case CMP:
			setflag(FLAG_ALL,0);
			if(p1>p2) setflag(FLAG_GT,TRUE);
			if(p1<p2) setflag(FLAG_LT,TRUE);
			if(p1==p2) {
				setflag(FLAG_EQ,TRUE);
				if(p1==0) setflag(FLAG_ZR,TRUE);
			}
//			debug("CMP");
			break;
		case MOV:
			r = p2;
			storeresult = TRUE;
//			debug("MOV");
			break;
		case AND:
			r = p1&p2;
			storeresult = TRUE;
//			debug("AND");
			break;
		case OR:
			r = p1|p2;
			storeresult = TRUE;
//			debug("OR");
			break;
		case XOR:
			r = p1^p2;
			storeresult = TRUE;
//			debug("XOR");
			break;
		case SYS:
			syscall(p1,p2);
//			debug("SYS");
			break;
		case IN:
//			debug("IN");
			break;
		case OUT:
//			debug("OUT");
			break;
		case SHL:
			r = p1<<p2;
			switch(reg.mode) {
				case MODE_8BITS:
					setflag(FLAG_CY,(p1>>(8-p2)) & 1);
					break;
				case MODE_16BITS:
					setflag(FLAG_CY,(p1>>(16-p2)) & 1);
					break;
				case MODE_32BITS:
					setflag(FLAG_CY,(p1>>(32-p2)) & 1);
					break;
			}
			storeresult = TRUE;
//			debug("SHL");		
			break;
		case SHR:
			r = p1>>p2;
			setflag(FLAG_CY, (p1>>(p2-1)) & 1);
			storeresult = TRUE;
//			debug("SHR");		
			break;
		case ROL:
			switch(reg.mode) {
				case MODE_8BITS:
					r = p1<<p2 | p1>>(8-p2);
					setflag(FLAG_CY,(p1>>(8-p2)) & 1);
					break;
				case MODE_16BITS:
					r = p1<<p2 | p1>>(16-p2);
					setflag(FLAG_CY,(p1>>(16-p2)) & 1);
					break;
				case MODE_32BITS:
					r = p1<<p2 | p1>>(32-p2);
					setflag(FLAG_CY,(p1>>(32-p2)) & 1);
					break;
			}
			storeresult = TRUE;
//			debug("ROL");
			break;
		case ROR:
			setflag(FLAG_CY,(p1>>(p2-1)) & 1);
			switch(reg.mode) {
				case MODE_8BITS:
					r = p1>>p2 | p1<<(8-p2);
					break;
				case MODE_16BITS:
					r = p1>>p2 | p1<<(16-p2);
					break;
				case MODE_32BITS:
					r = p1>>p2 | p1<<(32-p2);
					break;
			}
			storeresult = TRUE;
//			debug("ROR");
			break;
		case CMPS:
			setflag(FLAG_EQ,FALSE);
			while(getmemvalmode(p1, MODE_8BITS)) {
				if(getmemvalmode(p1, MODE_8BITS)>getmemvalmode(p2, MODE_8BITS)) {
					setflag(FLAG_GT,TRUE);
					break;
				} else {
					if(getmemvalmode(p1, MODE_8BITS)>getmemvalmode(p2, MODE_8BITS)) {
						setflag(FLAG_LT,TRUE);
						break;
					}
				}
				p1++;
				p2++;
				if(p1>MAXRAM || p2>MAXRAM) break;
			}
			if(getmemvalmode(p1, MODE_8BITS)==0) setflag(FLAG_EQ,TRUE);
//			debug("CMPS");
			break;
		case MOVS:
			while(getmemvalmode(p1, MODE_8BITS)) {
				setmemvalmode(p2,getmemvalmode(p1, MODE_8BITS),MODE_8BITS);
				p1++;
				p2++;
				if(p1>MAXRAM || p2>MAXRAM) break;
			}				
//			debug("MOVS");
			break;
		case MOVNZ:
			while(reg.cx) {
				setmemvalmode(p2,getmemvalmode(p1, MODE_8BITS), MODE_8BITS);
				p1++;
				p2++;
				reg.cx--;
				if(p1>MAXRAM || p2>MAXRAM) break;
			}				
//			debug("MOVNZ");
			break;
		case INC:
			r = p1+1;
			storeresult = TRUE;
//			debug("INC");
			break;
		case DEC:
			r = p1-1;
			storeresult = TRUE;
//			debug("DEC");
			break;
		case PUSH:
			push(p1);
//			debug("PUSH");
			break;
		case POP:
			r = pop();
			storeresult = TRUE;
//			debug("POP");
			break;
		case NOT:
			r = ~p1;
			storeresult = TRUE;
//			debug("NOT");
			break;
		case LOOP:
			reg.cx--;
			if(reg.cx) reg.ip = p1;
//			debug("LOOP");
			break;
		case LOOPNZ:
			if(getflag(FLAG_ZR)==FALSE) reg.ip = p1;
//			debug("LOOPNZ");
			break;
		case LOOPNE:
			if(getflag(FLAG_EQ)==FALSE) reg.ip = p1;
//			debug("LOOPNE");
			break;
		case JGT:
			if(getflag(FLAG_GT)==TRUE && getflag(FLAG_EQ)==FALSE) reg.ip = p1;
//			debug("JGT");
			break;
		case JGE:
			if(getflag(FLAG_GT)==TRUE || getflag(FLAG_EQ)==TRUE) reg.ip = p1;
//			debug("JGE");
			break;
		case JLT:
			if(getflag(FLAG_LT)==TRUE && getflag(FLAG_EQ)==FALSE) reg.ip = p1;
//			debug("JLT");
			break;
		case JLE:
			if(getflag(FLAG_LT)==TRUE || getflag(FLAG_EQ)==TRUE) reg.ip = p1;
//			debug("JLE");
			break;
		case JEQ:
			if(getflag(FLAG_EQ)==TRUE) reg.ip = p1;
//			debug("JEQ");
			break;
		case JNE:
			if(getflag(FLAG_EQ)==FALSE) reg.ip = p1;
//			debug("JNE");
			break;
		case JC:
			if(getflag(FLAG_CY)==TRUE) reg.ip = p1;
//			debug("JC");
			break;
		case JNC:
			if(getflag(FLAG_CY)==FALSE) reg.ip = p1;
//			debug("JNC");
			break;
		case JMP:
			reg.ip = p1;
//			debug("JMP");
			break;
		case CALL:
			push32(reg.ip);
			reg.ip = p1;
//			debug("CALL");
			break;
		case RET:
			reg.ip = pop32();
//			debug("RET");
			break;
		case NOP:
//			debug("NOP");
			break;
		case M8:
			reg.mode = MODE_8BITS;
//			debug("M8");
			break;
		case M16:
			reg.mode = MODE_16BITS;
//			debug("M16");
			break;
		case M32:
			reg.mode = MODE_32BITS;
//			debug("M32");
			break;
	}
	if(storeresult) {
		switch(reg.mode) {
			case MODE_8BITS:
				r = r & 0xFF;
				break;
			case MODE_16BITS:
				r = r & 0xFFFF;
				break;
			case MODE_32BITS:
				r = r & 0xFFFFFFFF;
				break;
		}
		if(r==0) setflag(FLAG_ZR,TRUE);
		switch(cod.e1) {
			case P_IMEDIATO:
				// nada a ser feito
				break;
			case P_REGISTRADOR:
				setregval(cod.p1, r);
				break;
			case P_DIRETO:
			case P_INDIRETO:
				setmemval(salvamemoria, r);
				break;
		}
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
		case CMPS:
			return("CMPS");
			break;
		case MOVS:
			return("MOVS");
			break;
		case MOVNZ:
			return("MOVNZ");
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
		case LOOPNE:
			return("LOOPNE");
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
		case JEQ:
			return("JEQ");
			break;
		case JNE:
			return("JNE");
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
		case RET:
			return("RET");
			break;
		case NOP:
			return("NOP");
			break;
		case M8:
			return("M8");
			break;
		case M16:
			return("M16");
			break;
		case M32:
			return("M32");
			break;
	}
	return("???");
}

void displayend(int e, int p, int v) {
	char *nomereg[] = { "AX","BX","CX","DX","SI","DI","SP","R1","R2","R3","R4","R5","R6","R7","FLAGS","IP"};
	switch(e) {
		case P_IMEDIATO:
			printf("%05X", (p << 16) | v);
			break;
		case P_REGISTRADOR:
			printf(nomereg[p]);
			break;
		case P_DIRETO:
			printf("[%05X]", (p << 16) | v);
			break;
		case P_INDIRETO:
			printf("[%s]", nomereg[p]);
			break;
	}
}

void displaydebug() {
	int c;
	printf("AX=%04X BX=%04X CX=%04X DX=%04X SI=%04X DI=%04X SP=%04X\n", reg.ax, reg.bx, reg.cx, reg.dx, reg.si, reg.di, reg.sp);
	printf("R1=%04X R2=%04X R3=%04X R4=%04X R5=%04X R6=%04X R7=%04X\n", reg.r1, reg.r2, reg.r3, reg.r4, reg.r5, reg.r6, reg.r7);
	printf("Flags: GT=%01d LT=%01d EQ=%01d ZR=%01d CY=%01d IP=%04X Mode: ", getflag(FLAG_GT),getflag(FLAG_LT),getflag(FLAG_EQ),getflag(FLAG_ZR),getflag(FLAG_CY), reg.ip);
	switch(reg.mode) {
		case MODE_8BITS:
			printf("8");
			break;
		case MODE_16BITS:
			printf("16");
			break;
		case MODE_32BITS:
			printf("32");
			break;
	}
	printf(" bits\n%s ", nomeinstrucao());
	if(cod.mne < 0xfec0) displayend(cod.e1, cod.p1, cod.v1);
	if(cod.mne < 0xfc00) {
		printf(",");
		displayend(cod.e2, cod.p2, cod.v2);
	}
	printf("\n>");
	c=0;
	while(c!=0x2A) {
		c=getchar()|0x20;
		if(c=='d') needdump=TRUE;
		if(c=='q') haltsystem=TRUE;
		if(c=='g') debugmode=FALSE;
	}
}

int loadram(char *nomearquivo) {
	FILE *f;
	int c;
	int d = 0;
	int state = 0;
	unsigned int fcrc = 0;
	unsigned int ccrc = 0;
	unsigned int i;
	
	if(debugmode) printf("Carregando arquivo \"%s\"...\n", nomearquivo);
	f=fopen(nomearquivo, "r");
	if(!f) {
		printf("Erro na carga do arquivo bytecode.\n");
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
						map[i++] = c;
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
			printf("Programa \"%s\" carregado.\n", progname);
			printf("Map: %s\n", map);
			printf("CRC: OK\n");
		}
		return(1);
	} else {
		printf("Falha de CRC: Read=%04X Calc=%04X\n", fcrc, ccrc);
		return(0);
	}
}

void dumpmem() {
	unsigned int i;
	unsigned int c;
	unsigned int l;
	unsigned int e;
	
	printf("Memory Dump, %d bytes\n", maxmem);
	for(l=0;l<=(maxmem/16);l++) {
		printf("%04X: ", l*16);
		for(c=0;c<16;c++) {
			e=l*16+c;
			if(e<maxmem) {
				i=getmemvalmode(e, MODE_8BITS);
				printf("%02X ", i);
			} else {
				printf("   ", i);
			}
		}
		for(c=0;c<16;c++) {
			e=l*16+c;
			if(e<maxmem) {
				i=getmemvalmode(e, MODE_8BITS);
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
		i=getmemvalmode(c, MODE_8BITS);
		printf("%02X ", i);
	}
	printf("\n");
}

void run() {
	reg.ip = 0;
	reg.sp = MAXRAM;
	haltsystem = FALSE;
	if(debugmode) printf("Iniciando VM.\n");
	while(!haltsystem) {
		decode();
		if(debugmode) {
			if(needdump) dumpmem();
			needdump = FALSE;
			displaydebug();
		}
		execute();
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
					printf("VM Arpia\nHelp\n\n-d ..... Debug mode\n-v ..... Versao\n<nome arquivo>   Arquivo bytecode.\n\n");
					break;
				case 'd':
					debugmode = TRUE;
					break;
				case 'v':
					printf("VM Arpia. Versao 0.01\nDomingos Paraiso\n\n");
					break;
			}
		} else {
			f = c;
		}
	}
	if(f > 0) {
		loadram(argv[f]);
		run();
	} else {
		printf("ERRO: Informe o nome do arquivo bytecode.\n");
	}
}