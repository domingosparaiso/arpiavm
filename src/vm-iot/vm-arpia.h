#ifndef VM_ARPIA_H

#define VM_ARPIA_H
#include "arpia.h"

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

codigo cod;
registradores reg;
unsigned char memory[MAXRAM];
unsigned int haltsystem = TRUE;
unsigned int maxmem;
unsigned int interrupt_table[32];
char progname[128];
char progmap[1024];

unsigned int regtype(unsigned int regnum) {
	switch(regnum) {
		case AH:
		case BH:
			return(REGTYPE_H);
			break;
		case AL:
		case BL:
			return(REGTYPE_L);
			break;
		case AW:
		case BW:
			return(REGTYPE_W);
			break;
	}
	return(REGTYPE_X);
}

unsigned int regsize(unsigned int regnum) {
	switch(regnum) {
		case AH:
		case BH:
		case AL:
		case BL:
			return(MODE_8BITS);
			break;
		case AW:
		case BW:
			return(MODE_16BITS);
			break;
	}
	return(MODE_32BITS);
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

unsigned int enderecamento() {
	return(getflag(FLAG_ADDR));
}

unsigned int getregval(unsigned int nreg) {
	switch(nreg) {
		case AX:
			return(reg.ax.x);
			break;
		case AW:
			return(reg.ax.w);
			break;
		case AH:
			return(reg.ax.n.h);
			break;
		case AL:
			return(reg.ax.n.l);
			break;
		case BX:
			return(reg.bx.x);
			break;
		case BW:
			return(reg.bx.w);
			break;
		case BH:
			return(reg.bx.n.h);
			break;
		case BL:
			return(reg.bx.n.l);
			break;
		case CX:
			return(reg.cx);
			break;
		case DX:
			return(reg.dx);
			break;
		case SX:
			return(reg.sx);
			break;
		case SP:
			return(reg.sp);
			break;
		case AF:
			return(reg.af.i);
			break;
		case BF:
			return(reg.bf.i);
			break;
		case CF:
			return(reg.cf.i);
			break;
		case DF:
			return(reg.df.i);
			break;
		case FLAGS:
			return(reg.flags);
			break;
		case IP:
			return(reg.ip);
			break;
		case WDMAX:
			return(reg.wdmax);
			break;
		case WDCOUNT:
			return(reg.wdcount);
			break;
	}
}

void setregval(unsigned int nreg, unsigned int value) {
	switch(nreg) {
		case AX:
			reg.ax.x = value;
			break;
		case AW:
			reg.ax.w = value & 0xffff;
			break;
		case AH:
			reg.ax.n.h = value & 0xff;
			break;
		case AL:
			reg.ax.n.l = value & 0xff;
			break;
		case BX:
			reg.bx.x = value;
			break;
		case BW:
			reg.bx.w = value & 0xffff;
			break;
		case BH:
			reg.bx.n.h = value & 0xff;
			break;
		case CX:
			reg.cx = value;
			break;
		case DX:
			reg.dx = value;
			break;
		case SX:
			reg.sx = value;
			break;
		case SP:
			reg.sp = value;
			break;
		case AF:
			reg.af.i = value;
			break;
		case BF:
			reg.bf.i = value;
			break;
		case CF:
			reg.cf.i = value;
			break;
		case DF:
			reg.df.i = value;
			break;
		case IP:
			reg.ip = value;
			break;
		case WDMAX:
			reg.wdmax = value;
			break;
		case WDCOUNT:
			reg.wdcount = value;
			break;
	}
}

unsigned int getmemval(unsigned int pointer, int mode) {
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

void setmemval(unsigned int pointer, unsigned int value, int mode) {
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

unsigned int getcodigo() {
	unsigned int w;
	w = getmemval(reg.ip, MODE_16BITS);
	reg.ip += 2;
	return(w);
}

/* le um valor imediato de largura variavel (1, 2 ou 4 bytes) a partir do
   fluxo de codigo, avancando IP de acordo. O nibble da instrucao nunca e
   usado para valores imediatos. */
unsigned int getcodigo_n(unsigned int size) {
	unsigned int v;
	switch(size) {
		case MODE_8BITS:
			v = getmemval(reg.ip, MODE_8BITS);
			reg.ip += 1;
			break;
		case MODE_16BITS:
			v = getmemval(reg.ip, MODE_16BITS);
			reg.ip += 2;
			break;
		default:
			v = getmemval(reg.ip, MODE_32BITS);
			reg.ip += 4;
			break;
	}
	return(v);
}

unsigned int resolve_endereco(unsigned int e, unsigned int v, unsigned int p, unsigned int size) {
	if(e == P_IMEDIATO) {
		v = getcodigo_n(size);
	}
	if(e == P_DIRETO) {
		if(enderecamento() == MODE_20BITS) {
			v = (p << 16) | getcodigo();
		} else {
			v = (getcodigo() << 16) | getcodigo();
		}
	}
	return(v);
}

void decode() {
	unsigned int i;
	cod.e1 = -1;
	cod.e2 = -1;
	cod.v1 = 0;
	cod.v2 = 0;
	cod.size1 = MODE_32BITS;
	cod.size2 = MODE_32BITS;
	i = getcodigo();
	if(i < SHL) {
		// A
		cod.mne = i & 0xf000;
		cod.e1 = (i&0x0c00)>>10;
		cod.e2 = (i&0x0300)>>8;
		cod.p1 = (i&0x00f0)>>4;
		cod.p2 = (i&0x000f);
	} else {
		if(i < CMPA) {
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
					if(i < SPARE_1ST) {
						// E
						cod.mne = i & 0xffe0;
						cod.e1 = (i&0x0010)>>4;
						cod.p1 = (i&0x000f);
					} else {
						if(i > SPARE_LAST) {
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
	if(cod.e1 == P_REGISTRADOR || cod.e1 == P_INDIRETO) cod.size1 = regsize(cod.p1);
	if(cod.e2 == P_REGISTRADOR || cod.e2 == P_INDIRETO) cod.size2 = regsize(cod.p2);
	cod.v1 = resolve_endereco(cod.e1, cod.v1, cod.p1, cod.size2);
	cod.v2 = resolve_endereco(cod.e2, cod.v2, cod.p2, cod.size1);
}

int stack_overflow() {
	if(reg.sp >= MAXRAM) {
		reg.sp = MAXRAM+1;
		return(1);
	} else {
		return(0);
	}
}

void push32(unsigned int val) {
	reg.sp -= 4;
	setmemval(reg.sp, val, MODE_32BITS);
}

void push16(unsigned int val) {
	reg.sp -= 2;
	setmemval(reg.sp, val, MODE_16BITS);
}

void push8(unsigned int val) {
	reg.sp -= 1;
	setmemval(reg.sp, val, MODE_8BITS);
}

void push(unsigned int val, unsigned int size) {
	switch(size) {
		case MODE_8BITS:
			push8(val);
			break;
		case MODE_16BITS:
			push16(val);
			break;
		default:
			push32(val);
			break;
	}
}

unsigned int pop32() {
	unsigned int r = 0;
	if(!stack_overflow()) {
		r = getmemval(reg.sp, MODE_32BITS);
		reg.sp += 4;
	}
	return(r);
}

unsigned int pop16() {
	unsigned int r = 0;
	if(!stack_overflow()) {
		r = getmemval(reg.sp, MODE_16BITS);
		reg.sp += 2;
	}
	return(r);
}

unsigned int pop8() {
	unsigned int r = 0;
	if(!stack_overflow()) {
		r = getmemval(reg.sp, MODE_8BITS);
		reg.sp += 2;
	}
	return(r);
}

unsigned int pop(unsigned int size) {
	unsigned int result = 0;
	switch(size) {
		case MODE_8BITS:
			result = pop8();
			break;
		case MODE_16BITS:
			result = pop16();
			break;
		default:
			result = pop32();
			break;
	}
	return(result);
}

void pusha() {
	push32(reg.ax.x);
	push32(reg.bx.x);
	push32(reg.cx);
	push32(reg.dx);
	push32(reg.sx);
	push32(reg.af.i);
	push32(reg.bf.i);
	push32(reg.cf.i);
	push32(reg.df.i);
	push32(reg.flags);
}

void popa() {
	reg.flags = pop32();
	reg.df.i = pop32();
	reg.cf.i = pop32();
	reg.bf.i = pop32();
	reg.af.i = pop32();
	reg.sx = pop32();
	reg.dx = pop32();
	reg.cx = pop32();
	reg.bx.x = pop32();
	reg.ax.x = pop32();
}

void push_next_ip() {
	/* decode() already advances reg.ip past the whole instruction
	   (including any operand words) before execute() runs, so the
	   current reg.ip is already the correct return address. */
	push32(reg.ip);
}

unsigned int isfloatreg(unsigned int r) {
	return(r==AF || r==BF || r==CF || r==DF);
}

unsigned int ajuste_param(unsigned int p, unsigned int e, unsigned int v, unsigned int* m, unsigned int size) {
	unsigned int result = 0;
	switch(e) {
		case P_IMEDIATO:
			result = v;
			break;
		case P_REGISTRADOR:
			result = getregval(p);
			break;
		case P_DIRETO:
			*m = v;
			result = getmemval(v, size);
			break;
		case P_INDIRETO:
			*m = getregval(p);
			result = getmemval(*m, size);
			break;
	}
	return(result);
}

unsigned int get_pin(unsigned int port) {
	if(port >= D1) return(port - 0x1000);
	return(port);
}

unsigned int port_type(unsigned int port) {
	if(port >= D1) return(PORT_DIGITAL);
	return(PORT_ANALOG);
}

void write_io(unsigned int port, unsigned int val) {
	#ifdef IOT
		unsigned int pin;
		pin = get_pin(port);
		switch(port_type(port)) {
			case PORT_DIGITAL:
				digitalWrite(pin, val);
				break;
			case PORT_ANALOG:
				analogWrite(pin, val);
				break;
		}
	#endif
}

unsigned int read_io(unsigned int port) {
	#ifdef IOT
		unsigned int pin;
		unsigned int val = 0;
		pin = get_pin(port);
		switch(port_type(port)) {
			case PORT_DIGITAL:
				val = digitalRead(pin);
				break;
			case PORT_ANALOG:
				val = analogRead(pin);
				break;
		}
		return(val);
	#else
		return(0);
	#endif
}

void execute() {
	unsigned int storeresult = FALSE;
	unsigned int r = 0;
	unsigned int p1;
	unsigned int p2;
	unsigned int size1 = MODE_32BITS;
	unsigned int size2 = MODE_32BITS;
	unsigned int salvamemoria;
	unsigned long long int longint;
	regX regx;
	regFloat fa, fb, fr;
	int compare;
	int floatop;

	size1 = cod.size1;
	size2 = cod.size2;
	p1 = ajuste_param(cod.p1, cod.e1, cod.v1, &salvamemoria, size2);
	p2 = ajuste_param(cod.p2, cod.e2, cod.v2, &salvamemoria, size1);
	floatop = (cod.e1 == P_REGISTRADOR) && isfloatreg(cod.p1);
	switch(cod.mne) {
		case ADD:
			if(floatop) {
				fa.i = p1; fb.i = p2; fr.f = fa.f + fb.f; r = fr.i;
			} else {
				r = p1+p2;
			}
			storeresult = TRUE;
//			debug("ADD");
			break;
		case SUB:
			if(floatop) {
				fa.i = p1; fb.i = p2; fr.f = fa.f - fb.f; r = fr.i;
			} else {
				r = p1-p2;
			}
			storeresult = TRUE;
//			debug("SUB");
			break;
		case MUL:
			if(floatop) {
				fa.i = p1; fb.i = p2; fr.f = fa.f * fb.f; r = fr.i;
			} else {
				longint = p1*p2;
				r = longint & 0xffffffff;
				reg.dx = longint >> 32;
			}
			storeresult = TRUE;
//			debug("MUL");
			break;
		case DIV:
			if(floatop) {
				fa.i = p1; fb.i = p2; fr.f = fa.f / fb.f; r = fr.i;
			} else {
				r = p1/p2;
				reg.dx = p1%p2;
			}
			storeresult = TRUE;
//			debug("DIV");
			break;
		case CMP:
			setflag(FLAG_GT,FALSE);
			setflag(FLAG_EQ,FALSE);
			setflag(FLAG_ZR,FALSE);
			if(floatop) {
				fa.i = p1; fb.i = p2;
				if(fa.f>=fb.f) setflag(FLAG_GT,TRUE);
				if(fa.f<=fb.f) setflag(FLAG_GT,FALSE);
				if(fa.f==fb.f) {
					setflag(FLAG_EQ,TRUE);
					if(fa.f==0) setflag(FLAG_ZR,TRUE);
				}
			} else {
				if(p1>=p2) setflag(FLAG_GT,TRUE);
				if(p1<=p2) setflag(FLAG_GT,FALSE);
				if(p1==p2) {
					setflag(FLAG_EQ,TRUE);
					if(p1==0) setflag(FLAG_ZR,TRUE);
				}
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
			vmsyscall(p1,p2);
//			debug("SYS");
			break;
		case IN:
			r = read_io(p2);
			storeresult = TRUE;
//			debug("IN");
			break;
		case OUT:
			write_io(p2, p1);
//			debug("OUT");
			break;
		case SHL:
			r = p1<<p2;
			switch(size1) {
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
			if(cod.e1 == P_REGISTRADOR) {
				switch(regsize(cod.p1)) {
					case MODE_8BITS:
						r = r & 0xff;
						break;
					case MODE_16BITS:
						r = r & 0xffff;
						break;
					case MODE_32BITS:
						r = r & 0xffffffff;
						break;
				}
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
			switch(size1) {
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
			if(cod.e1 == P_REGISTRADOR) {
				switch(regsize(cod.p1)) {
					case MODE_8BITS:
						r = r & 0xff;
						break;
					case MODE_16BITS:
						r = r & 0xffff;
						break;
					case MODE_32BITS:
						r = r & 0xffffffff;
						break;
				}
			}
			storeresult = TRUE;
//			debug("ROL");
			break;
		case ROR:
			setflag(FLAG_CY,(p1>>(p2-1)) & 1);
			switch(size1) {
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
		case CMPA:
			setflag(FLAG_OV,FALSE);
			if(p1>MAXRAM || p2>MAXRAM) {
				setflag(FLAG_OV,TRUE);
			} else {
				setflag(FLAG_EQ,FALSE);
				if(getflag(FLAG_STR)==TRUE) {
					compare = strcmp((const char *)memory + p1, (const char *)memory + p2);
				} else {
					compare = memcmp((const char *)memory + p1, (const char *)memory + p2, reg.cx);
				}
				if(compare > 0) {
					setflag(FLAG_GT,TRUE);
				} else {
					if(compare < 0) {
						setflag(FLAG_GT,FALSE);
					} else {
						setflag(FLAG_EQ,TRUE);
					}
				}
			}
//			debug("CMPA");
			break;
		case MOVA:
			setflag(FLAG_OV,FALSE);
			if(p1>MAXRAM || p2>MAXRAM) {
				setflag(FLAG_OV,TRUE);
			} else {
				if(getflag(FLAG_STR)==TRUE) {
					strcpy((char *)memory + p1, (char *)memory + p2);
				} else {
					memcpy((char *)memory + p1, (char *)memory + p2, reg.cx);
				}
			}
//			debug("MOVA");
			break;
		case SETINT:
			interrupt_table[p1] = p2;
			break;
//			debug("SETINT");
		case INC:
			if(floatop) {
				fa.i = p1; fr.f = fa.f + 1.0f; r = fr.i;
			} else {
				r = p1+1;
			}
			storeresult = TRUE;
//			debug("INC");
			break;
		case DEC:
			if(floatop) {
				fa.i = p1; fr.f = fa.f - 1.0f; r = fr.i;
			} else {
				r = p1-1;
			}
			storeresult = TRUE;
//			debug("DEC");
			break;
		case PUSH:
			push(p1, size1);
//			debug("PUSH");
			break;
		case POP:
			r = pop(size1);
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
			reg.cx--;
			if(reg.cx>0 && getflag(FLAG_ZR)==FALSE) reg.ip = p1;
//			debug("LOOPNZ");
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
			if(getflag(FLAG_GT)==FALSE && getflag(FLAG_EQ)==FALSE) reg.ip = p1;
//			debug("JLT");
			break;
		case JLE:
			if(getflag(FLAG_GT)==FALSE || getflag(FLAG_EQ)==TRUE) reg.ip = p1;
//			debug("JLE");
			break;
		case JZ:
			if(getflag(FLAG_EQ)==TRUE) reg.ip = p1;
//			debug("JEZ");
			break;
		case JNZ:
			if(getflag(FLAG_EQ)==FALSE) reg.ip = p1;
//			debug("JNZ");
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
			push_next_ip();
			reg.ip = p1;
//			debug("CALL");
			break;
		case WDC:
			reg.wdmax = p1;
//			debug("WDC");
			break;
		case INT:
			pusha();
			push_next_ip();
			reg.ip = interrupt_table[p1];
//			debug("INT");
			break;
		case DELAY:
			#ifdef IOT
				delay(p1); // hardware delay
			#endif
//			debug("DELAY");
			break;
		case PUSHA:
			pusha();
//			debug("PUSHA");
			break;
		case POPA:
			popa();
//			debug("POPA");
			break;
		case CLI:
			setflag(FLAG_INT,FALSE);
//			debug("CLI");
			break;
		case STI:
			setflag(FLAG_INT,TRUE);
//			debug("STI");
			break;
		case RET:
			reg.ip = pop32();
//			debug("RET");
			break;
		case IRET:
			reg.ip = pop32();
			popa();
//			debug("IRET");
			break;
		case BIN:
			setflag(FLAG_STR,FALSE);
//			debug("BIN");
			break;
		case STR:
			setflag(FLAG_STR,TRUE);
//			debug("STR");
			break;
		case SM20:
			setflag(FLAG_ADDR,TRUE);
//			debug("SM20");
			break;
		case SM32:
			setflag(FLAG_ADDR,FALSE);
//			debug("SM32");
			break;
		case EWD:
			setflag(FLAG_WDOG,TRUE);
//			debug("EWD");
			break;
		case DWD:
			setflag(FLAG_WDOG,FALSE);
//			debug("DWD");
			break;
		case RWD:
			reg.wdcount=reg.wdmax;
//			debug("EWD");
			break;
		case NOP:
//			debug("NOP");
			break;
	}
	if(storeresult) {
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
				setmemval(salvamemoria, r, regsize(cod.p2));
				break;
		}
	}
}

#endif
