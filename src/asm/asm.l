%option noyywrap
%option caseless
%{
#define YYSTYPE int
#include <stdio.h>
#include "../include/arpia.h"
#include "../sys/system.h"
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
"R1"		{ yylval = R1; return REGISTRADOR; }
"R2"		{ yylval = R2; return REGISTRADOR; }
"R3"		{ yylval = R3; return REGISTRADOR; }
"R4"		{ yylval = R4; return REGISTRADOR; }
"R5"		{ yylval = R5; return REGISTRADOR; }
"R6"		{ yylval = R6; return REGISTRADOR; }
"CT"		{ yylval = CT; return REGISTRADOR; }
"F1"		{ yylval = F1; return REGISTRADOR; }
"F2"		{ yylval = F2; return REGISTRADOR; }
"F3"		{ yylval = F3; return REGISTRADOR; }
"F4"		{ yylval = F4; return REGISTRADOR; }
"SI"		{ yylval = SI; return REGISTRADOR; }
"DI"		{ yylval = DI; return REGISTRADOR; }
"SP"		{ yylval = SP; return REGISTRADOR; }
"IP"		{ yylval = IP; return REGISTRADOR; }
"FLAGS"		{ yylval = FLAGS; return REGISTRADOR; }

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
"JEQ"		{ yylval = JEQ; return MNEM_E; }
"JNE"		{ yylval = JNE; return MNEM_E; }
"JC"		{ yylval = JC; return MNEM_E; }
"JNC"		{ yylval = JNC; return MNEM_E; }
"JMP"		{ yylval = JMP; return MNEM_E; }
"CALL"		{ yylval = CALL; return MNEM_E; }

"RET"		{ yylval = RET; return MNEM_F; }
"NOP"		{ yylval = NOP; return MNEM_F; }
"SETFA"		{ yylval = SETFA; return MNEM_F; }
"SETFS"		{ yylval = SETFS; return MNEM_F; }
"RST"		{ yylval = RST; return MNEM_F; }
"M20"		{ yylval = M20; return MNEM_F; }
"M32"		{ yylval = M32; return MNEM_F; }
"\.NAME"	{ contval = 1; return NAME; }
"\.MAP"		{ return MAP; }
"\.CODE"	{ return CODE; }
"\.DATA"	{ return DATA; }
"\.END"		{ return ENDPROG; }
"DB"		{ return DATA_BYTE; }
"DW"		{ return DATA_WORD; }
"DD"		{ return DATA_FLOAT; }
"PRINT"		{ yylval = SYS_PRINT; return NUMERO; }
"PARAM"		{ yylval = SYS_PARAM; return NUMERO; }
"DEBUG"		{ yylval = SYS_DEBUG; return NUMERO; }
"HALT"		{ yylval = SYS_HALT; return NUMERO; }
"DUP"		{ return DUP; }

"?"							{ return INDEFINIDO; }
\"([^"]|"\\\"")*\"			{ strcpy(str, yytext); return STRING; }
("-"|"+")?{dig}+			{ yylval = atoi(yytext); return NUMERO; }
("-"|"+")?{dig}{hexdig}*H	{ yylval = hextoi(yytext); return NUMERO; }
"["[ ]*{ident}[ ]*"]"		{ strcpy(str, yytext+1); str[strlen(str)-1] = 0; yylval = criareloc(); return IIDENT; }
{ident}						{ strcpy(str, yytext); if(contval==0) yylval = criareloc(); return IDENT; }
{ident}:					{ strcpy(str, yytext); yylval = crialabel(TRUE); return LABEL; }
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
