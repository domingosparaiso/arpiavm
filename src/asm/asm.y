%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vm-iot/arpia.h"

int yylex();
int yyerror(const char* s);
int criareloc();
int parse_ok;
int crialabel(int novo);
void entrada_dados_int(int valor);
void entrada_dados_str();
void entrada_dados_indefinido();
void entrada_dados_duplica_indefinido(int qtd);
void geracodigo_A(int mne, int p1, int p2, int v1, int v2);
void geracodigo_B(int mne, int p1, int p2, int v1, int v2);
void geracodigo_C(int mne, int p1, int p2, int v1, int v2);
void geracodigo_D(int mne, int p1, int v1);
void geracodigo_E(int mne, int p1, int v1);
void geracodigo_F(int mne);
void printmne(int m);
void setup_name();
void setup_map();
void incode(unsigned char c);
void reloctable();
void printcode();
int mnemonico = 0;
int tipodado = 0;
int contval = 0;
int ip = 0;
int ultimo_indefinido = 0;
char progname[128];
char map[1024];
char *map_point;
char str[MAXSTR];
char code[MAXRAM];
struct labels {
	char nome[MAXNOMELBL];
	int ip;
};
struct relocs {
	int id;
	int nibble_ip;   /* -1 = nenhum nibble a corrigir */
	int nibble_alto; /* 1 = corrige bits 4-7 de code[nibble_ip], 0 = corrige bits 0-3 */
	int value_ip;
	int width;       /* 1, 2 ou 4 bytes */
};
int contlabel = 0;
int contreloc = 0;
struct relocs tabreloc[MAXRELOC];
struct labels tablabel[MAXLBL];
int curaddrmode = MODE_32BITS; /* rastreia o ADDRMODE assumido durante a montagem, atualizado por SM20/SM32 */

%}
%token MNEM_A
%token MNEM_B
%token MNEM_C
%token MNEM_D
%token MNEM_E
%token MNEM_F
%token REGISTRADOR
%token ABREPAR
%token FECHAPAR
%token VIRG
%token IDENT
%token IIDENT
%token LABEL
%token DATA_BYTE
%token DATA_WORD
%token DATA_DOUBLE
%token DATA_FLOAT
%token NUMERO
%token STRING
%token EOL
%token NAME
%token MAP
%token CODE
%token DATA
%token ENDPROG
%token INDEFINIDO
%token DUP
%%
input:
		 name maps code data endp
		|name code data endp
		|name maps code endp
		|name code endp
		|name maps data endp
		|name data endp
		;

name:
		 NAME STRING EOL { setup_name(); }
		;

maps:
		 map
		|maps map
		;
		
map:
		 MAP IDENT {setup_map(); } STRING EOL { setup_map(); }
		;
		
endp:
		 ENDPROG
		|ENDPROG eols
		;
		
eols:
		 EOL
		|eols EOL
		;

code:
		 CODE EOL codes
		;

codes:
		 codeline
		|codes codeline
		;

codeline:
		 EOL
		|codeinstruct EOL
		;

codeinstruct:
		 codeinstruct1
		|label
		|label codeinstruct1
		
codeinstruct1:
		 mnemonico_A
		|mnemonico_B
		|mnemonico_C
		|mnemonico_D
		|mnemonico_E
		|mnemonico_F
		;

data:	 DATA EOL datas

datas:
		 dataline
		|datas dataline
		;
		
dataline:
		 EOL
		|datainstruct EOL
		;

datainstruct:
		 datadec
		|label
		|label datadec
		;
		
label:
		 LABEL
		;
		
mnemonico_A:
		 MNEM_A REGISTRADOR VIRG REGISTRADOR { geracodigo_A($1, P_REGISTRADOR, P_REGISTRADOR, $2, $4); }
		|MNEM_A REGISTRADOR VIRG imediato { geracodigo_A($1, P_REGISTRADOR, P_IMEDIATO, $2, $4); }
		|MNEM_A REGISTRADOR VIRG direto { geracodigo_A($1, P_REGISTRADOR, P_DIRETO, $2, $4); }
		|MNEM_A REGISTRADOR VIRG indireto { geracodigo_A($1, P_REGISTRADOR, P_INDIRETO, $2, $4); }
		|MNEM_A imediato VIRG REGISTRADOR { geracodigo_A($1, P_IMEDIATO, P_REGISTRADOR, $2, $4); }
		|MNEM_A imediato VIRG imediato { geracodigo_A($1, P_IMEDIATO, P_IMEDIATO, $2, $4); }
		|MNEM_A imediato VIRG direto { geracodigo_A($1, P_IMEDIATO, P_DIRETO, $2, $4); }
		|MNEM_A imediato VIRG indireto { geracodigo_A($1, P_IMEDIATO, P_INDIRETO, $2, $4); }
		|MNEM_A direto VIRG REGISTRADOR { geracodigo_A($1, P_DIRETO, P_REGISTRADOR, $2, $4); }
		|MNEM_A direto VIRG imediato { geracodigo_A($1, P_DIRETO, P_IMEDIATO, $2, $4); }
		|MNEM_A direto VIRG direto { geracodigo_A($1, P_DIRETO, P_DIRETO, $2, $4); }
		|MNEM_A direto VIRG indireto { geracodigo_A($1, P_DIRETO, P_INDIRETO, $2, $4); }
		|MNEM_A indireto VIRG REGISTRADOR { geracodigo_A($1, P_INDIRETO, P_REGISTRADOR, $2, $4); }
		|MNEM_A indireto VIRG imediato { geracodigo_A($1, P_INDIRETO, P_IMEDIATO, $2, $4); }
		|MNEM_A indireto VIRG direto { geracodigo_A($1, P_INDIRETO, P_DIRETO, $2, $4); }
		|MNEM_A indireto VIRG indireto { geracodigo_A($1, P_INDIRETO, P_INDIRETO, $2, $4); }
		
mnemonico_B:
		 MNEM_B REGISTRADOR VIRG REGISTRADOR { geracodigo_B($1, P_REGISTRADOR, P_REGISTRADOR, $2, $4); }
		|MNEM_B REGISTRADOR VIRG imediato { geracodigo_B($1, P_REGISTRADOR, P_IMEDIATO, $2, $4); }
		|MNEM_B direto VIRG REGISTRADOR { geracodigo_B($1, P_DIRETO, P_REGISTRADOR, $2, $4); }
		|MNEM_B direto VIRG imediato { geracodigo_B($1, P_DIRETO, P_IMEDIATO, $2, $4); }
		|MNEM_B indireto VIRG REGISTRADOR { geracodigo_B($1, P_INDIRETO, P_REGISTRADOR, $2, $4); }
		|MNEM_B indireto VIRG imediato { geracodigo_B($1, P_INDIRETO, P_IMEDIATO, $2, $4); }
		;

mnemonico_C:
		 MNEM_C REGISTRADOR VIRG REGISTRADOR { geracodigo_C($1, P_REGISTRADOR, P_REGISTRADOR, $2, $4); }
		|MNEM_C REGISTRADOR VIRG imediato { geracodigo_C($1, P_REGISTRADOR, P_IMEDIATO, $2, $4); }
		|MNEM_C imediato VIRG REGISTRADOR { geracodigo_C($1, P_IMEDIATO, P_REGISTRADOR, $2, $4); }
		|MNEM_C imediato VIRG imediato { geracodigo_C($1, P_IMEDIATO, P_IMEDIATO, $2, $4); }
		;
		
mnemonico_D:
		 MNEM_D REGISTRADOR { geracodigo_D($1, P_REGISTRADOR, $2); }
		|MNEM_D imediato { geracodigo_D($1, P_IMEDIATO, $2); }
		|MNEM_D direto { geracodigo_D($1, P_DIRETO, $2); }
		|MNEM_D indireto { geracodigo_D($1, P_INDIRETO, $2); }
		;

mnemonico_E:
		 MNEM_E REGISTRADOR { geracodigo_E($1, P_REGISTRADOR, $2); }
		|MNEM_E imediato { geracodigo_E($1, P_IMEDIATO, $2); }
		;

mnemonico_F:
		 MNEM_F { geracodigo_F($1); }
		;

imediato:
		 NUMERO
		|IDENT
		;

direto:
		 ABREPAR NUMERO FECHAPAR
		|IIDENT
		;

indireto:
		 ABREPAR REGISTRADOR FECHAPAR { $$ = $2; }
		;

datadec:
		 declaradados dados { tipodado = 0; }
		;

declaradados:
		 db
		|dw
		|dd
		|fl
		;

dados:	 dado
		|dados VIRG dado
		;

dado:	 imediato { entrada_dados_int($1); }
		|STRING { entrada_dados_str(); }
		|DUP ABREPAR NUMERO FECHAPAR INDEFINIDO { entrada_dados_duplica_indefinido(yylval); }
		|INDEFINIDO { entrada_dados_indefinido(); }
		;

db:		 DATA_BYTE { tipodado = DATA_BYTE; }

dw:		 DATA_WORD { tipodado = DATA_WORD; }

dd:		 DATA_DOUBLE { tipodado = DATA_DOUBLE; }

fl:		 DATA_FLOAT { tipodado = DATA_FLOAT; }
		;

%%
#include "lex.yy.c"

int main(int argc, char**argv) {
	int i;
	char *inname = NULL;
	char *outname = NULL;

	for(i=1; i<argc; i++) {
		if(strcmp(argv[i], "-o") == 0 && i+1 < argc) {
			outname = argv[++i];
		} else {
			inname = argv[i];
		}
	}
	if(inname) {
		yyin = fopen(inname, "r");
		if(!yyin) {
			printf("Erro ao abrir arquivo de entrada: %s\n", inname);
			return(1);
		}
	}

	map_point = map;
	parse_ok = 1;
	yyparse();
	if(parse_ok) {
		if(outname) {
			if(!freopen(outname, "w", stdout)) {
				printf("Erro ao abrir arquivo de saida: %s\n", outname);
				return(1);
			}
		}
		reloctable();
		printcode();
	}
	return(parse_ok ? 0 : 1);
}

int yyerror(const char *s) {
	printf("Erro na linha: %d\nMensagem: %s\n\"%s\"\n", yylineno, s, yytext );
	parse_ok = 0;
	return 0;
}

void printcode() {
	char *mp;
	int c;
	int maxprint;
	
	unsigned int crc = 0;
	printf("PROG \"%s\"\n", progname);
	c=0;
	for(mp=map;mp<=map_point;mp++) {
		if(*mp) {
			if(c==0) {
				printf("MAP ");
				c=1;
			}
			printf("%c", *mp);
		} else {
			if(c==1) {
				printf(" ");
				c=2;
			} else {
				printf("\n");
				c=0;
			}
		}
	}
	
	maxprint = ip;
	if(ultimo_indefinido<ip) maxprint=ultimo_indefinido;
	for(c=0;c<maxprint;c++) {
		crc = (crc + (unsigned char) code[c]) & 0xFFFF;
		printf("%02X ", (unsigned char) code[c]);
	}
	printf("\n%04X\n", crc);
}

void reloctable() {
	int c;
	int end_label;

	for(c=0;c<contreloc;c++) {
		end_label = tablabel[tabreloc[c].id].ip;
		if(tabreloc[c].nibble_ip >= 0) {
			if(tabreloc[c].nibble_alto) {
				code[tabreloc[c].nibble_ip] = code[tabreloc[c].nibble_ip] | ((end_label>>12)&0xf0);
			} else {
				code[tabreloc[c].nibble_ip] = code[tabreloc[c].nibble_ip] | ((end_label>>16)&0xf);
			}
		}
		switch(tabreloc[c].width) {
			case 1:
				code[tabreloc[c].value_ip] = (unsigned char)(end_label&0xff);
				break;
			case 2:
				code[tabreloc[c].value_ip] = (unsigned char)((end_label>>8)&0xff);
				code[tabreloc[c].value_ip+1] = (unsigned char)(end_label&0xff);
				break;
			case 4:
				code[tabreloc[c].value_ip] = (unsigned char)((end_label>>24)&0xff);
				code[tabreloc[c].value_ip+1] = (unsigned char)((end_label>>16)&0xff);
				code[tabreloc[c].value_ip+2] = (unsigned char)((end_label>>8)&0xff);
				code[tabreloc[c].value_ip+3] = (unsigned char)(end_label&0xff);
				break;
		}
	}
}

void entrada_dados_int(int valor) {
	switch(tipodado) {
	case DATA_BYTE:
		incode(valor&0xff);
		break;
	case DATA_WORD:
		incode((valor>>8)&0xff);
		incode(valor&0xff);
		break;
	case DATA_DOUBLE:
	case DATA_FLOAT:
		incode((valor>>24)&0xff);
		incode((valor>>16)&0xff);
		incode((valor>>8)&0xff);
		incode(valor&0xff);
		break;
	}
}

void entrada_dados_indefinido() {
	if(ip < MAXRAM) {
		code[ip++] = 0;
	} else {
		printf("Erro: codigo maior que RAM definida");
	}
}

void entrada_dados_duplica_indefinido(int qtd) {
	int cont;
	for(cont=1;cont<=qtd;cont++)
		entrada_dados_indefinido();
}

void entrada_dados_str() {
	char *pstr=str;
	char c;
	pstr++;
	while(*(pstr+1)) {
		c=*pstr++;
		incode(c);
	}
}

void setup_name() {
	str[strlen(str)-1]=0;
	strcpy(progname, str+1);
}

void setup_map() {
	strcpy(map_point, str);
	map_point += strlen(str);
	*map_point=0;
	map_point++;
}

/* ajustareloc: se v for um marcador de relocacao pendente (negativo), registra
   uma entrada na tabela de relocacao (posicao do nibble opcional + posicao/
   largura do valor) e devolve 0 como placeholder; senao devolve v inalterado. */
int ajustareloc(int v, int value_ip, int width, int nibble_ip, int nibble_alto) {
	if(v<0) {
		v = -v -1;
		tabreloc[v].value_ip = value_ip;
		tabreloc[v].width = width;
		tabreloc[v].nibble_ip = nibble_ip;
		tabreloc[v].nibble_alto = nibble_alto;
		v=0;
	}
	return(v);
}

/* tamanho (em bytes) de um registrador, para fins de montagem (espelha
   regsize() da VM). */
int regsize_asm(int r) {
	switch(r) {
		case AH:
		case AL:
		case BH:
		case BL:
			return(1);
		case AW:
		case BW:
			return(2);
	}
	return(4);
}

/* largura, em bytes, que um valor imediato deve usar: a largura do
   registrador do operando companheiro, se houver; senao 32 bits (4 bytes). */
int largura_imediato(int modo_companheiro, int valor_companheiro) {
	if(modo_companheiro == P_REGISTRADOR || modo_companheiro == P_INDIRETO) {
		return(regsize_asm(valor_companheiro));
	}
	return(4);
}

void emitvalor(int v, int width) {
	switch(width) {
		case 1:
			incode((unsigned char)(v&0xff));
			break;
		case 2:
			incode((unsigned char)((v>>8)&0xff));
			incode((unsigned char)(v&0xff));
			break;
		default:
			incode((unsigned char)((v>>24)&0xff));
			incode((unsigned char)((v>>16)&0xff));
			incode((unsigned char)((v>>8)&0xff));
			incode((unsigned char)(v&0xff));
			break;
	}
}

/* largura (em bytes) que uma referencia DIRETO ocupara apos o mnemonico,
   conforme o modo de enderecamento assumido no momento (rastreado por
   SM20/SM32 durante a montagem). */
int largura_direto() {
	return(curaddrmode == MODE_20BITS ? 2 : 4);
}

void geracodigo_A(int mne, int p1, int p2, int v1, int v2) {
	int w1 = 0, w2 = 0;
	int base_ip, v1_ip, v2_ip;

	if(p1 == P_IMEDIATO) w1 = largura_imediato(p2, v2);
	else if(p1 == P_DIRETO) w1 = largura_direto();

	if(p2 == P_IMEDIATO) w2 = largura_imediato(p1, v1);
	else if(p2 == P_DIRETO) w2 = largura_direto();

	base_ip = ip;
	v1_ip = base_ip + 2;
	v2_ip = v1_ip + w1;

	mne = mne | p1<<10 | p2<<8;

	switch(p1) {
		case P_IMEDIATO:
			v1 = ajustareloc(v1, v1_ip, w1, -1, 0);
			break;
		case P_DIRETO:
			if(curaddrmode == MODE_20BITS) {
				v1 = ajustareloc(v1, v1_ip, 2, base_ip+1, 1);
				mne = mne | ((v1>>12)&0xf0);
			} else {
				v1 = ajustareloc(v1, v1_ip, 4, -1, 0);
			}
			break;
		case P_REGISTRADOR:
		case P_INDIRETO:
			mne = mne | (v1<<4);
			break;
	}
	switch(p2) {
		case P_IMEDIATO:
			v2 = ajustareloc(v2, v2_ip, w2, -1, 0);
			break;
		case P_DIRETO:
			if(curaddrmode == MODE_20BITS) {
				v2 = ajustareloc(v2, v2_ip, 2, base_ip+1, 0);
				mne = mne | ((v2>>16)&0xf);
			} else {
				v2 = ajustareloc(v2, v2_ip, 4, -1, 0);
			}
			break;
		case P_REGISTRADOR:
		case P_INDIRETO:
			mne = mne | v2;
			break;
	}
	printmne(mne);
	if(w1) emitvalor(v1, w1);
	if(w2) emitvalor(v2, w2);
}

void geracodigo_B(int mne, int p1, int p2, int v1, int v2) {
	int w1 = 0, w2 = 0;
	int base_ip, v1_ip, v2_ip;

	if(p1 == P_DIRETO) w1 = largura_direto();
	if(p2 == P_IMEDIATO) w2 = largura_imediato(p1, v1);

	base_ip = ip;
	v1_ip = base_ip + 2;
	v2_ip = v1_ip + w1;

	mne = mne | p1<<9 | p2<<8;

	switch(p1) {
		case P_DIRETO:
			if(curaddrmode == MODE_20BITS) {
				v1 = ajustareloc(v1, v1_ip, 2, base_ip+1, 1);
				mne = mne | ((v1>>12)&0xf0);
			} else {
				v1 = ajustareloc(v1, v1_ip, 4, -1, 0);
			}
			break;
		case P_REGISTRADOR:
		case P_INDIRETO:
			mne = mne | (v1<<4);
			break;
	}
	switch(p2) {
		case P_IMEDIATO:
			v2 = ajustareloc(v2, v2_ip, w2, -1, 0);
			break;
		case P_REGISTRADOR:
			mne = mne | v2;
			break;
	}
	printmne(mne);
	if(w1) emitvalor(v1, w1);
	if(w2) emitvalor(v2, w2);
}

void geracodigo_C(int mne, int p1, int p2, int v1, int v2) {
	int w1 = 0, w2 = 0;
	int base_ip, v1_ip, v2_ip;

	if(p1 == P_IMEDIATO) w1 = largura_imediato(p2, v2);
	if(p2 == P_IMEDIATO) w2 = largura_imediato(p1, v1);

	base_ip = ip;
	v1_ip = base_ip + 2;
	v2_ip = v1_ip + w1;

	mne = mne | p1<<9 | p2<<8;

	switch(p1) {
		case P_IMEDIATO:
			v1 = ajustareloc(v1, v1_ip, w1, -1, 0);
			break;
		case P_REGISTRADOR:
			mne = mne | (v1<<4);
			break;
	}
	switch(p2) {
		case P_IMEDIATO:
			v2 = ajustareloc(v2, v2_ip, w2, -1, 0);
			break;
		case P_REGISTRADOR:
			mne = mne | v2;
			break;
	}
	printmne(mne);
	if(w1) emitvalor(v1, w1);
	if(w2) emitvalor(v2, w2);
}

void geracodigo_D(int mne, int p1, int v1) {
	int base_ip, v1_ip;

	base_ip = ip;
	v1_ip = base_ip + 2;

	mne = mne | p1<<4;

	switch(p1) {
		case P_IMEDIATO:
			v1 = ajustareloc(v1, v1_ip, 4, -1, 0);
			printmne(mne);
			emitvalor(v1, 4);
			break;
		case P_DIRETO:
			if(curaddrmode == MODE_20BITS) {
				v1 = ajustareloc(v1, v1_ip, 2, base_ip+1, 0);
				printmne(mne | ((v1>>16)&0xf));
				emitvalor(v1, 2);
			} else {
				v1 = ajustareloc(v1, v1_ip, 4, -1, 0);
				printmne(mne);
				emitvalor(v1, 4);
			}
			break;
		case P_INDIRETO:
		case P_REGISTRADOR:
			printmne(mne | v1);
			break;
	}
}

void geracodigo_E(int mne, int p1, int v1) {
	int base_ip, v1_ip;

	base_ip = ip;
	v1_ip = base_ip + 2;

	mne = mne | p1<<4;

	switch(p1) {
		case P_IMEDIATO:
			v1 = ajustareloc(v1, v1_ip, 4, -1, 0);
			printmne(mne);
			emitvalor(v1, 4);
			break;
		case P_REGISTRADOR:
			printmne(mne | v1);
			break;
	}
}

void geracodigo_F(int mne) {
	if(mne == SM20) curaddrmode = MODE_20BITS;
	if(mne == SM32) curaddrmode = MODE_32BITS;
	printmne(mne);
}

void printmne(int m) {
	incode((unsigned char)(m>>8)&0xff);
	incode((unsigned char) m&0xff);
}

void incode(unsigned char c) {
	if(ip < MAXRAM) {
		code[ip++] = c;
		ultimo_indefinido = ip;
	} else {
		printf("Erro: codigo maior que RAM definida");
	}
}

int crialabel(int novo) {
	char label[MAXLBL];
	int ind;
	int tamlabel;
	int c;
	
	ind = -1;
	strcpy(label, str);
	tamlabel = strlen(label);
	if(novo) {
		label[tamlabel-1] = 0;
	}
	for(c=0;c<contlabel;c++) {
		if(strcmp(label, tablabel[c].nome)==0) {
			ind=c;
			break;
		}
	}
	if(ind<0) {
		if(tamlabel>MAXNOMELBL) {
			printf("Nome muito longo: %s\n", label);
			ind = -1;
		} else {
			strcpy(tablabel[contlabel].nome, label);
			ind = contlabel;
			contlabel++;
		}
	}
	if(novo && ind>=0) {
		tablabel[ind].ip = ip;
	}
	return(ind);
}

int criareloc() {
	char label[MAXLBL];
	int ind;
	
	strcpy(label, str);
	ind = crialabel(FALSE);
	tabreloc[contreloc].id = ind;
	ind = contreloc;
	contreloc++;
	ind = -(ind+1);
	return(ind);
}

