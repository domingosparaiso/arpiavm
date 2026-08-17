%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c.tab.h"

char strbuf[4096];
%}

digit		[0-9]
hexdigit	[0-9A-Fa-f]
ident		[A-Za-z_][A-Za-z0-9_]*

%%

"/*"					{ int c1=0,c2=input(); for(;;){ c1=c2; c2=input(); if(c2==EOF) break; if(c1=='*'&&c2=='/') break; } }
"//".*					{ }

"char"					{ return KW_CHAR; }
"int"					{ return KW_INT; }
"long"					{ return KW_LONG; }
"float"					{ return KW_FLOAT; }
"void"					{ return KW_VOID; }
"struct"				{ return KW_STRUCT; }
"if"					{ return KW_IF; }
"else"					{ return KW_ELSE; }
"while"					{ return KW_WHILE; }
"for"					{ return KW_FOR; }
"return"				{ return KW_RETURN; }
"break"					{ return KW_BREAK; }
"continue"				{ return KW_CONTINUE; }

"0x"{hexdigit}+			{ yylval.ival = (int)strtol(yytext+2, NULL, 16); return NUMBER; }
{digit}+"."{digit}+		{ yylval.fval = atof(yytext); return FLOATNUM; }
{digit}+				{ yylval.ival = atoi(yytext); return NUMBER; }
'([^'\\]|\\.)'			{ yylval.ival = yytext[1]=='\\' ? (yytext[2]=='n'?'\n':yytext[2]=='t'?'\t':yytext[2]=='0'?'\0':yytext[2]) : yytext[1]; return CHARLIT; }
\"([^"\\]|\\.)*\"		{
							char *src = yytext+1;
							char *dst = strbuf;
							while(*src && !(*src=='"' && *(src+1)==0)) {
								if(*src=='\\') {
									src++;
									switch(*src) {
										case 'n': *dst++='\n'; break;
										case 't': *dst++='\t'; break;
										case 'r': *dst++='\r'; break;
										case '0': *dst++='\0'; break;
										case '\\': *dst++='\\'; break;
										case '"': *dst++='"'; break;
										default: *dst++=*src; break;
									}
									src++;
								} else {
									*dst++ = *src++;
								}
							}
							*dst = 0;
							yylval.sval = strdup(strbuf);
							return STRINGLIT;
						}

{ident}					{ yylval.sval = strdup(yytext); return IDENT; }

"<<="					{ return SHLEQ; }
">>="					{ return SHREQ; }
"+="					{ return PLUSEQ; }
"-="					{ return MINUSEQ; }
"*="					{ return STAREQ; }
"/="					{ return SLASHEQ; }
"%="					{ return PERCENTEQ; }
"&="					{ return ANDEQ; }
"|="					{ return OREQ; }
"^="					{ return XOREQ; }
"=="					{ return EQ; }
"!="					{ return NE; }
"<="					{ return LE; }
">="					{ return GE; }
"&&"					{ return ANDAND; }
"||"					{ return OROR; }
"<<"					{ return SHL; }
">>"					{ return SHR; }
"++"					{ return INCOP; }
"--"					{ return DECOP; }

"="						{ return '='; }
"<"						{ return '<'; }
">"						{ return '>'; }
"+"						{ return '+'; }
"-"						{ return '-'; }
"*"						{ return '*'; }
"/"						{ return '/'; }
"%"						{ return '%'; }
"&"						{ return '&'; }
"|"						{ return '|'; }
"^"						{ return '^'; }
"~"						{ return '~'; }
"!"						{ return '!'; }
"."						{ return '.'; }
","						{ return ','; }
";"						{ return ';'; }
":"						{ return ':'; }
"{"						{ return '{'; }
"}"						{ return '}'; }
"("						{ return '('; }
")"						{ return ')'; }
"["						{ return '['; }
"]"						{ return ']'; }

"\r"					{ }
"\n"					{ yylineno++; }
[ \t]+					{ }
.						{ fprintf(stderr, "Caractere inesperado na linha %d: %s\n", yylineno, yytext); }

%%
