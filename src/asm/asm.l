%option noyywrap
%option caseless
%{
#define YYSTYPE int
#include <stdio.h>
#include "../vm-iot/arpia.h"
#include "asm.tab.h"

void skipcmnts();
int hextoi(char *s);
%}
dig		[0-9]
hexdig	[0-9A-F]
ident	[A-Z][A-Z0-9_]*
%%
"/*"		{ skipcmnts(); }
";".*$		{ }
"("			{ return ABREPAR; }
")"			{ return FECHAPAR; }
","			{ return VIRG; }
"AH"		{ yylval = AH; return REGISTRADOR; }
"AL"		{ yylval = AL; return REGISTRADOR; }
"AW"		{ yylval = AW; return REGISTRADOR; }
"AX"		{ yylval = AX; return REGISTRADOR; }
"BH"		{ yylval = BH; return REGISTRADOR; }
"BL"		{ yylval = BL; return REGISTRADOR; }
"BW"		{ yylval = BW; return REGISTRADOR; }
"BX"		{ yylval = BX; return REGISTRADOR; }
"CX"		{ yylval = CX; return REGISTRADOR; }
"DX"		{ yylval = DX; return REGISTRADOR; }
"SX"		{ yylval = SX; return REGISTRADOR; }
"SP"		{ yylval = SP; return REGISTRADOR; }
"AF"		{ yylval = AF; return REGISTRADOR; }
"BF"		{ yylval = BF; return REGISTRADOR; }
"CF"		{ yylval = CF; return REGISTRADOR; }
"DF"		{ yylval = DF; return REGISTRADOR; }
"FLAGS"		{ yylval = FLAGS; return REGISTRADOR; }
"IP"		{ yylval = IP; return REGISTRADOR; }

"ADD"		{ yylval = ADD; return MNEM_A; }
"SUB"		{ yylval = SUB; return MNEM_A; }
"MUL"		{ yylval = MUL; return MNEM_A; }
"DIV"		{ yylval = DIV; return MNEM_A; }
"CMP"		{ yylval = CMP; return MNEM_A; }
"MOV"		{ yylval = MOV; return MNEM_A; }
"AND"		{ yylval = AND; return MNEM_A; }
"OR"		{ yylval = OR; return MNEM_A; }
"XOR"		{ yylval = XOR; return MNEM_A; }
"SYS"		{ yylval = SYS; return MNEM_A; }
"IN"		{ yylval = IN; return MNEM_A; }
"OUT"		{ yylval = OUT; return MNEM_A; }

"SHL"		{ yylval = SHL; return MNEM_B; }
"SHR"		{ yylval = SHR; return MNEM_B; }
"ROL"		{ yylval = ROL; return MNEM_B; }
"ROR"		{ yylval = ROR; return MNEM_B; }

"CMPA"		{ yylval = CMPA; return MNEM_C; }
"MOVA"		{ yylval = MOVA; return MNEM_C; }
"SETINT"	{ yylval = SETINT; return MNEM_C; }

"INC"		{ yylval = INC; return MNEM_D; }
"DEC"		{ yylval = DEC; return MNEM_D; }
"PUSH"		{ yylval = PUSH; return MNEM_D; }
"POP"		{ yylval = POP; return MNEM_D; }
"NOT"		{ yylval = NOT; return MNEM_D; }

"LOOP"		{ yylval = LOOP; return MNEM_E; }
"LOOPNZ"	{ yylval = LOOPNZ; return MNEM_E; }
"JGT"		{ yylval = JGT; return MNEM_E; }
"JGE"		{ yylval = JGE; return MNEM_E; }
"JLT"		{ yylval = JLT; return MNEM_E; }
"JLE"		{ yylval = JLE; return MNEM_E; }
"JZ"		{ yylval = JZ; return MNEM_E; }
"JNZ"		{ yylval = JNZ; return MNEM_E; }
"JC"		{ yylval = JC; return MNEM_E; }
"JNC"		{ yylval = JNC; return MNEM_E; }
"JMP"		{ yylval = JMP; return MNEM_E; }
"CALL"		{ yylval = CALL; return MNEM_E; }
"WDC"		{ yylval = WDC; return MNEM_E; }
"INT"		{ yylval = INT; return MNEM_E; }
"DELAY"		{ yylval = DELAY; return MNEM_E; }

"PUSHA"		{ yylval = PUSHA; return MNEM_F; }
"POPA"		{ yylval = POPA; return MNEM_F; }
"CLI"		{ yylval = CLI; return MNEM_F; }
"STI"		{ yylval = STI; return MNEM_F; }
"RET"		{ yylval = RET; return MNEM_F; }
"IRET"		{ yylval = IRET; return MNEM_F; }
"BIN"		{ yylval = BIN; return MNEM_F; }
"STR"		{ yylval = STR; return MNEM_F; }
"RST"		{ yylval = RST; return MNEM_F; }
"SM20"		{ yylval = SM20; return MNEM_F; }
"SM32"		{ yylval = SM32; return MNEM_F; }
"EWD"		{ yylval = EWD; return MNEM_F; }
"DWD"		{ yylval = DWD; return MNEM_F; }
"RWD"		{ yylval = RWD; return MNEM_F; }
"NOP"		{ yylval = NOP; return MNEM_F; }

"\.NAME"	{ contval = 1; return NAME; }
"\.MAP"		{ return MAP; }
"\.CODE"	{ return CODE; }
"\.DATA"	{ return DATA; }
"\.END"		{ return ENDPROG; }
"DB"		{ return DATA_BYTE; }
"DW"		{ return DATA_WORD; }
"DD"		{ return DATA_DOUBLE; }
"FL"		{ return DATA_FLOAT; }
"PRINT"		{ yylval = SYS_PRINT; return NUMERO; }
"PRINTS"	{ yylval = SYS_PRINTS; return NUMERO; }
"PRINTF"	{ yylval = SYS_PRINTF; return NUMERO; }
"DEBUG"		{ yylval = SYS_DEBUG; return NUMERO; }
"HALT"		{ yylval = SYS_HALT; return NUMERO; }
"GETCHAR"	{ yylval = SYS_GETCHAR; return NUMERO; }
"INPUTPIN"	{ yylval = SYS_INPUTPIN; return NUMERO; }
"OUTPUTPIN"	{ yylval = SYS_OUTPUTPIN; return NUMERO; }
"PULLUP"	{ yylval = SYS_PULLUP; return NUMERO; }
"DUP"		{ return DUP; }

"?"							{ return INDEFINIDO; }
\"([^"]|"\\\"")*\"			{ strcpy(str, yytext); return STRING; }
("-"|"+")?{dig}+			{ yylval = atoi(yytext); return NUMERO; }
("-"|"+")?{dig}{hexdig}*H	{ yylval = hextoi(yytext); return NUMERO; }
"["[ ]*{ident}[ ]*"]"		{ strcpy(str, yytext+1); str[strlen(str)-1] = 0; yylval = criareloc(); return IIDENT; }
{ident}						{ strcpy(str, yytext); if(contval==0) yylval = criareloc(); return IDENT; }
{ident}:					{ strcpy(str, yytext); yylval = crialabel(TRUE); return LABEL; }
"\r"						{ }
"\n"						{ yylineno++; contval = 0; return EOL; }
([ \t]*)					{ }
([ \t]*)"\n"$				{ }
%%

void skipcmnts()
{
	for(;;) {
		while (input() != '*');
		if (input() != '/') {
			unput(yytext[yyleng-1]);
		} else {
			return;
		}
	}
}

int hextoi(char *s) {
	char c;
	int r=0;
	while(c=*s++) {
		if(c=='h' || c=='H') {
			return(r);
		} else {
			if(c>='0' && c<='9') {
				r<<=4;
				r+=c-'0';
			} else {
				c|=0x20;
				if(c>='a' && c<='f') {
					r<<=4;
					r+=c-'a'+10;
				}
			}
		}
	}
	return(r);
}
