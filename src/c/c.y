%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int yylex();
int yyerror(const char *s);
extern int yylineno;
extern char *yytext;

int parse_ok = 1;

/* ---------- type system ---------- */

typedef enum { T_VOID, T_CHAR, T_INT, T_LONG, T_FLOAT, T_STRUCT } basekind_t;

typedef struct structdef {
	char name[64];
	struct field *fields;
	int size;
	struct structdef *next;
} structdef_t;

typedef struct {
	basekind_t base;
	structdef_t *sdef;
	int array;     /* 1 = array type */
	int arraylen;  /* element count, when array */
	int isref;     /* 1 = stored as a 4-byte address (array/struct function parameter) */
} ctype_t;

typedef struct field {
	char name[64];
	ctype_t type;
	int offset;
	struct field *next;
} field_t;

typedef struct param {
	char name[64];
	ctype_t type;
	char label[32];
	struct param *next;
} param_t;

typedef enum { SYM_VAR, SYM_FUNC } symkind_t;

typedef struct symbol {
	char name[64];
	symkind_t kind;
	ctype_t type;      /* SYM_VAR: variable type. SYM_FUNC: return type */
	char label[32];
	param_t *params;
	int nparams;
	int defined;
	struct symbol *next;
} symbol_t;

/* expression attribute: after a grammar action runs, kind==0 means the
   value (or, for array/struct types, its address) is already sitting in
   AX; kind==1/2 mean it is an lvalue whose load has been deferred. */
typedef struct {
	ctype_t type;
	int kind;      /* 0=value in AX, 1=simple (direct [label]), 2=complex (address pushed on VM stack) */
	char label[32];
	int isref;     /* only used when kind==1: this simple identifier itself holds an address */
} expr_t;

typedef struct paramnode {
	char name[64];
	ctype_t type;
	struct paramnode *next;
} paramnode_t;

/* ---------- globals ---------- */

structdef_t *g_structs = NULL;
symbol_t *g_globals = NULL;
symbol_t *g_locals = NULL;
ctype_t curdecltype;
structdef_t *curstruct = NULL;
int curstructoffset = 0;
symbol_t *curfunc = NULL;

int labelcounter = 0, varcounter = 0, strcounter = 0;

FILE *codeout, *dataout;
FILE *capture_stack[8];
int capture_depth = 0;

#define MAXLOOP 32
typedef struct { char contlbl[32]; char breaklbl[32]; } loopctx_t;
loopctx_t loopstack[MAXLOOP];
int loopsp = 0;

/* ---------- forward declarations ---------- */

void emitc(const char *fmt, ...);
void emitd(const char *fmt, ...);
char *newlabel(void);
void semerror(const char *fmt, ...);

structdef_t *find_struct(const char *name);
field_t *find_field(structdef_t *sd, const char *name);
void begin_struct(const char *name);
void add_field(ctype_t basetype, const char *name, int arraylen);
void end_struct(void);

int elemsize(ctype_t *t);
int basewidth(ctype_t *t);
int is_float_type(ctype_t *t);
void push_acc(ctype_t *t);
ctype_t promote(ctype_t a, ctype_t b);

symbol_t *find_symbol(const char *name);
symbol_t *declare_global(ctype_t type, const char *name);
symbol_t *declare_global_array(ctype_t type, const char *name, int len);
void declare_global_init(ctype_t type, const char *name, int value);
void declare_global_init_float(ctype_t type, const char *name, double value);
symbol_t *declare_local(ctype_t type, const char *name);
symbol_t *declare_local_array(ctype_t type, const char *name, int len);
void gen_store_symbol(symbol_t *s);

paramnode_t *mkparam(ctype_t type, const char *name);
paramnode_t *append_param(paramnode_t *list, paramnode_t *p);
symbol_t *declare_func(ctype_t rettype, const char *name, paramnode_t *params, int isdef);
void start_func(ctype_t rettype, const char *name, paramnode_t *params);
void end_func(void);
void gen_return_void(void);
void gen_return_value(void);

void push_loop(const char *cont, const char *brk);
void pop_loop(void);
const char *loop_cont_top(void);
void gen_break(void);
void gen_continue(void);
void begin_capture(void);
char *end_capture(void);

void gen_rvalue(expr_t *e);
expr_t gen_assign(expr_t *l, expr_t *r, const char *op);
expr_t gen_binop(expr_t *l, expr_t *r, const char *mnem);
expr_t gen_relop(expr_t *l, expr_t *r, const char *jmpmnem);
expr_t gen_logic(expr_t *l, expr_t *r, const char *op);
expr_t gen_neg(expr_t *e);
expr_t gen_not(expr_t *e);
expr_t gen_bitnot(expr_t *e);
expr_t gen_index(expr_t *base, expr_t *idx);
expr_t gen_field(expr_t *base, const char *fieldname);
expr_t gen_call(const char *name, int argcount);
expr_t gen_ident(const char *name);
expr_t gen_intlit(int v);
expr_t gen_charlit(int v);
expr_t gen_floatlit(double v);
expr_t gen_stringlit(const char *s);

%}

%union {
	int ival;
	double fval;
	char *sval;
	ctype_t ctype;
	expr_t expr;
	paramnode_t *plist;
	int count;
}

%token <sval> IDENT STRINGLIT
%token <ival> NUMBER CHARLIT
%token <fval> FLOATNUM
%token KW_CHAR KW_INT KW_LONG KW_FLOAT KW_VOID KW_STRUCT
%token KW_IF KW_ELSE KW_WHILE KW_FOR KW_RETURN KW_BREAK KW_CONTINUE
%token SHLEQ SHREQ PLUSEQ MINUSEQ STAREQ SLASHEQ PERCENTEQ ANDEQ OREQ XOREQ
%token EQ NE LE GE ANDAND OROR SHL SHR INCOP DECOP

%type <ctype> type_spec
%type <expr> expr assignment_expr postfix_expr primary_expr
%type <plist> param_list param_list_opt param
%type <count> call_args call_args_opt opt_expr
%type <sval> if_head

%nonassoc LOWER_THAN_ELSE
%nonassoc KW_ELSE

%left OROR
%left ANDAND
%left '|'
%left '^'
%left '&'
%left EQ NE
%left '<' '>' LE GE
%left SHL SHR
%left '+' '-'
%left '*' '/' '%'
%right UMINUS UNOT UBITNOT

%%

program:
	  toplevel_list
	;

toplevel_list:
	  /* empty */
	| toplevel_list toplevel
	;

toplevel:
	  struct_def
	| func_def
	| func_proto ';'
	| global_decl ';'
	;

struct_def:
	  KW_STRUCT IDENT '{' { begin_struct($2); } member_list '}' ';' { end_struct(); }
	;

member_list:
	  member
	| member_list member
	;

member:
	  type_spec member_declarator_list ';'
	;

member_declarator_list:
	  member_declarator
	| member_declarator_list ',' member_declarator
	;

member_declarator:
	  IDENT                          { add_field(curdecltype, $1, 0); }
	| IDENT '[' NUMBER ']'           { add_field(curdecltype, $1, $3); }
	;

type_spec:
	  KW_CHAR                        { $$.base=T_CHAR; $$.sdef=NULL; $$.array=0; $$.arraylen=0; $$.isref=0; curdecltype=$$; }
	| KW_INT                         { $$.base=T_INT; $$.sdef=NULL; $$.array=0; $$.arraylen=0; $$.isref=0; curdecltype=$$; }
	| KW_LONG                        { $$.base=T_LONG; $$.sdef=NULL; $$.array=0; $$.arraylen=0; $$.isref=0; curdecltype=$$; }
	| KW_FLOAT                       { $$.base=T_FLOAT; $$.sdef=NULL; $$.array=0; $$.arraylen=0; $$.isref=0; curdecltype=$$; }
	| KW_VOID                        { $$.base=T_VOID; $$.sdef=NULL; $$.array=0; $$.arraylen=0; $$.isref=0; curdecltype=$$; }
	| KW_STRUCT IDENT                { $$.base=T_STRUCT; $$.sdef=find_struct($2); $$.array=0; $$.arraylen=0; $$.isref=0;
	                                    if(!$$.sdef) semerror("struct '%s' nao definido", $2); curdecltype=$$; }
	;

global_decl:
	  type_spec global_declarator_list
	;

global_declarator_list:
	  global_declarator
	| global_declarator_list ',' global_declarator
	;

global_declarator:
	  IDENT                          { declare_global(curdecltype, $1); }
	| IDENT '[' NUMBER ']'           { declare_global_array(curdecltype, $1, $3); }
	| IDENT '=' NUMBER               { declare_global_init(curdecltype, $1, $3); }
	| IDENT '=' FLOATNUM             { declare_global_init_float(curdecltype, $1, $3); }
	;

func_proto:
	  type_spec IDENT '(' param_list_opt ')'  { declare_func($1, $2, $4, 0); }
	;

func_def:
	  type_spec IDENT '(' param_list_opt ')' '{' { start_func($1, $2, $4); }
	  block_item_list '}'  { end_func(); }
	;

param_list_opt:
	  /* empty */                    { $$ = NULL; }
	| KW_VOID                        { $$ = NULL; }
	| param_list                     { $$ = $1; }
	;

param_list:
	  param                          { $$ = $1; }
	| param_list ',' param           { $$ = append_param($1, $3); }
	;

param:
	  type_spec IDENT                { ctype_t t=$1; if(t.base==T_STRUCT) t.isref=1; $$ = mkparam(t, $2); }
	| type_spec IDENT '[' ']'        { ctype_t t=$1; t.array=1; t.isref=1; $$ = mkparam(t, $2); }
	;

block_item_list:
	  /* empty */
	| block_item_list block_item
	;

block_item:
	  statement
	| local_decl ';'
	;

local_decl:
	  type_spec local_declarator_list
	;

local_declarator_list:
	  local_declarator
	| local_declarator_list ',' local_declarator
	;

local_declarator:
	  IDENT                          { declare_local(curdecltype, $1); }
	| IDENT '[' NUMBER ']'           { declare_local_array(curdecltype, $1, $3); }
	| IDENT '=' assignment_expr      { symbol_t *s = declare_local(curdecltype, $1); gen_rvalue(&$3); gen_store_symbol(s); }
	;

statement:
	  ';'
	| '{' block_item_list '}'
	| assignment_expr ';'            { gen_rvalue(&$1); }
	| if_head statement %prec LOWER_THAN_ELSE
	    { emitc("%s:", $<sval>1); }
	| if_head statement KW_ELSE
	    { char *le=newlabel(); emitc("\tjmp %s", le); emitc("%s:", $<sval>1); $<sval>$=le; }
	  statement
	    { emitc("%s:", $<sval>4); }
	| KW_WHILE '('
	    { char *l=newlabel(); emitc("%s:", l); $<sval>$=l; }
	  assignment_expr ')'
	    { gen_rvalue(&$4); char *le=newlabel(); emitc("\tcmp ax,0"); emitc("\tjz %s", le); push_loop($<sval>3, le); $<sval>$=le; }
	  statement
	    { emitc("\tjmp %s", $<sval>3); emitc("%s:", $<sval>6); pop_loop(); }
	| KW_FOR '(' opt_expr ';'
	    { char *l=newlabel(); emitc("%s:", l); $<sval>$=l; }
	  opt_expr ';'
	    { char *le=newlabel(); char *lc=newlabel();
	      if($6) { emitc("\tcmp ax,0"); emitc("\tjz %s", le); }
	      push_loop(lc, le);
	      begin_capture();
	      $<sval>$=le; }
	  opt_expr
	    { $<sval>$ = end_capture(); }
	  ')' statement
	    { emitc("%s:", loop_cont_top());
	      emitc("%s", $<sval>10);
	      emitc("\tjmp %s", $<sval>5);
	      emitc("%s:", $<sval>8);
	      pop_loop(); }
	| KW_RETURN ';'                  { gen_return_void(); }
	| KW_RETURN assignment_expr ';'  { gen_rvalue(&$2); gen_return_value(); }
	| KW_BREAK ';'                   { gen_break(); }
	| KW_CONTINUE ';'                { gen_continue(); }
	;

opt_expr:
	  /* empty */                    { $$ = 0; }
	| assignment_expr                { gen_rvalue(&$1); $$ = 1; }
	;

if_head:
	  KW_IF '(' assignment_expr ')'
	    { gen_rvalue(&$3); char *l=newlabel(); emitc("\tcmp ax,0"); emitc("\tjz %s", l); $$=l; }
	;

assignment_expr:
	  postfix_expr '=' assignment_expr        { $$ = gen_assign(&$1, &$3, "="); }
	| postfix_expr PLUSEQ assignment_expr     { $$ = gen_assign(&$1, &$3, "add"); }
	| postfix_expr MINUSEQ assignment_expr    { $$ = gen_assign(&$1, &$3, "sub"); }
	| postfix_expr STAREQ assignment_expr     { $$ = gen_assign(&$1, &$3, "mul"); }
	| postfix_expr SLASHEQ assignment_expr    { $$ = gen_assign(&$1, &$3, "/"); }
	| postfix_expr PERCENTEQ assignment_expr  { $$ = gen_assign(&$1, &$3, "%"); }
	| postfix_expr ANDEQ assignment_expr      { $$ = gen_assign(&$1, &$3, "and"); }
	| postfix_expr OREQ assignment_expr       { $$ = gen_assign(&$1, &$3, "or"); }
	| postfix_expr XOREQ assignment_expr      { $$ = gen_assign(&$1, &$3, "xor"); }
	| postfix_expr SHLEQ assignment_expr      { $$ = gen_assign(&$1, &$3, "shl"); }
	| postfix_expr SHREQ assignment_expr      { $$ = gen_assign(&$1, &$3, "shr"); }
	| expr                                    { $$ = $1; }
	;

expr:
	  expr '+' { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "add"); }
	| expr '-' { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "sub"); }
	| expr '*' { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "mul"); }
	| expr '/' { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "div"); }
	| expr '%' { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "mod"); }
	| expr '&' { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "and"); }
	| expr '|' { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "or"); }
	| expr '^' { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "xor"); }
	| expr SHL { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "shl"); }
	| expr SHR { push_acc(&$1.type); } expr           { $$ = gen_binop(&$1, &$4, "shr"); }
	| expr '<' { push_acc(&$1.type); } expr           { $$ = gen_relop(&$1, &$4, "jlt"); }
	| expr '>' { push_acc(&$1.type); } expr           { $$ = gen_relop(&$1, &$4, "jgt"); }
	| expr LE  { push_acc(&$1.type); } expr           { $$ = gen_relop(&$1, &$4, "jle"); }
	| expr GE  { push_acc(&$1.type); } expr           { $$ = gen_relop(&$1, &$4, "jge"); }
	| expr EQ  { push_acc(&$1.type); } expr           { $$ = gen_relop(&$1, &$4, "jz"); }
	| expr NE  { push_acc(&$1.type); } expr           { $$ = gen_relop(&$1, &$4, "jnz"); }
	| expr ANDAND { push_acc(&$1.type); } expr        { $$ = gen_logic(&$1, &$4, "and"); }
	| expr OROR { push_acc(&$1.type); } expr          { $$ = gen_logic(&$1, &$4, "or"); }
	| '-' expr %prec UMINUS                          { $$ = gen_neg(&$2); }
	| '!' expr %prec UNOT                            { $$ = gen_not(&$2); }
	| '~' expr %prec UBITNOT                         { $$ = gen_bitnot(&$2); }
	| INCOP postfix_expr                             { $$ = gen_assign(&$2, NULL, "preinc"); }
	| DECOP postfix_expr                             { $$ = gen_assign(&$2, NULL, "predec"); }
	| postfix_expr                                   { gen_rvalue(&$1); $$ = $1; }
	;

postfix_expr:
	  primary_expr
	| postfix_expr '[' expr ']'      { $$ = gen_index(&$1, &$3); }
	| postfix_expr '.' IDENT         { $$ = gen_field(&$1, $3); }
	| IDENT '(' call_args_opt ')'    { $$ = gen_call($1, $3); }
	;

call_args_opt:
	  /* empty */                    { $$ = 0; }
	| call_args                      { $$ = $1; }
	;

call_args:
	  assignment_expr                { gen_rvalue(&$1); push_acc(&$1.type); $$ = 1; }
	| call_args ',' assignment_expr  { gen_rvalue(&$3); push_acc(&$3.type); $$ = $1 + 1; }
	;

primary_expr:
	  IDENT                          { $$ = gen_ident($1); }
	| NUMBER                         { $$ = gen_intlit($1); }
	| CHARLIT                        { $$ = gen_charlit($1); }
	| FLOATNUM                       { $$ = gen_floatlit($1); }
	| STRINGLIT                      { $$ = gen_stringlit($1); }
	| '(' assignment_expr ')'        { $$ = $2; }
	;

%%
#include "lex.yy.c"

/* ================= implementation ================= */

void emitc(const char *fmt, ...) {
	va_list a;
	va_start(a, fmt);
	vfprintf(codeout, fmt, a);
	va_end(a);
	fputc('\n', codeout);
}

void emitd(const char *fmt, ...) {
	va_list a;
	va_start(a, fmt);
	vfprintf(dataout, fmt, a);
	va_end(a);
	fputc('\n', dataout);
}

char *newlabel(void) {
	char buf[32];
	sprintf(buf, "L%d", ++labelcounter);
	return strdup(buf);
}

void semerror(const char *fmt, ...) {
	va_list a;
	fprintf(stderr, "Erro na linha %d: ", yylineno);
	va_start(a, fmt);
	vfprintf(stderr, fmt, a);
	va_end(a);
	fprintf(stderr, "\n");
	parse_ok = 0;
}

/* ---------- structs ---------- */

structdef_t *find_struct(const char *name) {
	structdef_t *s;
	for(s = g_structs; s; s = s->next) if(strcmp(s->name, name) == 0) return s;
	return NULL;
}

field_t *find_field(structdef_t *sd, const char *name) {
	field_t *f;
	if(!sd) return NULL;
	for(f = sd->fields; f; f = f->next) if(strcmp(f->name, name) == 0) return f;
	return NULL;
}

void begin_struct(const char *name) {
	structdef_t *s = calloc(1, sizeof(structdef_t));
	strcpy(s->name, name);
	s->next = g_structs;
	g_structs = s;
	curstruct = s;
	curstructoffset = 0;
}

void add_field(ctype_t basetype, const char *name, int arraylen) {
	field_t *f = calloc(1, sizeof(field_t));
	field_t *p;
	ctype_t t = basetype;
	if(arraylen > 0) { t.array = 1; t.arraylen = arraylen; }
	strcpy(f->name, name);
	f->type = t;
	f->offset = curstructoffset;
	curstructoffset += elemsize(&t) * (t.array ? t.arraylen : 1);
	f->next = NULL;
	if(!curstruct->fields) {
		curstruct->fields = f;
	} else {
		p = curstruct->fields;
		while(p->next) p = p->next;
		p->next = f;
	}
}

void end_struct(void) {
	curstruct->size = curstructoffset;
	curstruct = NULL;
}

/* ---------- sizes ---------- */

int elemsize(ctype_t *t) {
	if(t->base == T_STRUCT) return t->sdef ? t->sdef->size : 0;
	switch(t->base) {
		case T_CHAR: return 1;
		case T_INT: return 2;
		case T_LONG: return 4;
		case T_FLOAT: return 4;
		default: return 4;
	}
}

int basewidth(ctype_t *t) {
	if(t->isref) return 4;
	switch(t->base) {
		case T_CHAR: return 1;
		case T_INT: return 2;
		default: return 4;
	}
}

int is_float_type(ctype_t *t) {
	return t->base == T_FLOAT && !t->isref && !t->array;
}

void push_acc(ctype_t *t) {
	emitc(is_float_type(t) ? "\tpush af" : "\tpush ax");
}

int ranktype(ctype_t *t) {
	switch(t->base) {
		case T_CHAR: return 1;
		case T_INT: return 2;
		case T_LONG: return 3;
		case T_FLOAT: return 4;
		default: return 0;
	}
}

ctype_t promote(ctype_t a, ctype_t b) {
	return ranktype(&a) >= ranktype(&b) ? a : b;
}

/* ---------- symbol table ---------- */

symbol_t *find_local(const char *name) {
	symbol_t *s;
	for(s = g_locals; s; s = s->next) if(strcmp(s->name, name) == 0) return s;
	return NULL;
}

symbol_t *find_global(const char *name) {
	symbol_t *s;
	for(s = g_globals; s; s = s->next) if(strcmp(s->name, name) == 0) return s;
	return NULL;
}

symbol_t *find_symbol(const char *name) {
	symbol_t *s = find_local(name);
	if(s) return s;
	return find_global(name);
}

void emit_storage(const char *label, ctype_t *t) {
	int sz;
	if(t->isref) { emitd("%s:\tdd 0", label); return; }
	if(t->array) {
		sz = elemsize(t) * t->arraylen;
		emitd("%s:\tdb dup(%d) ?", label, sz);
		return;
	}
	switch(t->base) {
		case T_CHAR: emitd("%s:\tdb 0", label); break;
		case T_INT: emitd("%s:\tdw 0", label); break;
		case T_LONG: emitd("%s:\tdd 0", label); break;
		case T_FLOAT: emitd("%s:\tfl 0", label); break;
		case T_STRUCT:
			sz = t->sdef ? t->sdef->size : 0;
			emitd("%s:\tdb dup(%d) ?", label, sz);
			break;
		default: emitd("%s:\tdd 0", label); break;
	}
}

symbol_t *declare_global(ctype_t type, const char *name) {
	symbol_t *s;
	if(find_symbol(name)) semerror("'%s' ja declarado", name);
	s = calloc(1, sizeof(symbol_t));
	strcpy(s->name, name);
	s->kind = SYM_VAR;
	s->type = type;
	sprintf(s->label, "V%d", ++varcounter);
	s->next = g_globals;
	g_globals = s;
	emit_storage(s->label, &type);
	return s;
}

symbol_t *declare_global_array(ctype_t type, const char *name, int len) {
	type.array = 1;
	type.arraylen = len;
	return declare_global(type, name);
}

void declare_global_init(ctype_t type, const char *name, int value) {
	symbol_t *s;
	if(find_symbol(name)) semerror("'%s' ja declarado", name);
	s = calloc(1, sizeof(symbol_t));
	strcpy(s->name, name);
	s->kind = SYM_VAR;
	s->type = type;
	sprintf(s->label, "V%d", ++varcounter);
	s->next = g_globals;
	g_globals = s;
	switch(type.base) {
		case T_CHAR: emitd("%s:\tdb %d", s->label, value); break;
		case T_INT: emitd("%s:\tdw %d", s->label, value); break;
		default: emitd("%s:\tdd %d", s->label, value); break;
	}
}

void declare_global_init_float(ctype_t type, const char *name, double value) {
	symbol_t *s;
	union { float f; int i; } conv;
	if(find_symbol(name)) semerror("'%s' ja declarado", name);
	if(type.base != T_FLOAT) semerror("'%s' inicializado com literal float mas nao e float", name);
	s = calloc(1, sizeof(symbol_t));
	strcpy(s->name, name);
	s->kind = SYM_VAR;
	s->type = type;
	sprintf(s->label, "V%d", ++varcounter);
	s->next = g_globals;
	g_globals = s;
	conv.f = (float)value;
	emitd("%s:\tfl %d", s->label, conv.i);
}

symbol_t *declare_local(ctype_t type, const char *name) {
	symbol_t *s;
	if(find_local(name)) semerror("'%s' ja declarado", name);
	s = calloc(1, sizeof(symbol_t));
	strcpy(s->name, name);
	s->kind = SYM_VAR;
	s->type = type;
	sprintf(s->label, "V%d", ++varcounter);
	s->next = g_locals;
	g_locals = s;
	emit_storage(s->label, &type);
	return s;
}

symbol_t *declare_local_array(ctype_t type, const char *name, int len) {
	type.array = 1;
	type.arraylen = len;
	return declare_local(type, name);
}

void gen_store_symbol(symbol_t *s) {
	if(is_float_type(&s->type)) {
		emitc("\tmov [%s],af", s->label);
		return;
	}
	switch(basewidth(&s->type)) {
		case 1: emitc("\tmov [%s],ah", s->label); break;
		case 2: emitc("\tmov [%s],aw", s->label); break;
		default: emitc("\tmov [%s],ax", s->label); break;
	}
}

/* ---------- functions ---------- */

paramnode_t *mkparam(ctype_t type, const char *name) {
	paramnode_t *p = calloc(1, sizeof(paramnode_t));
	strcpy(p->name, name);
	p->type = type;
	p->next = NULL;
	return p;
}

paramnode_t *append_param(paramnode_t *list, paramnode_t *p) {
	paramnode_t *q = list;
	if(!q) return p;
	while(q->next) q = q->next;
	q->next = p;
	return list;
}

symbol_t *declare_func(ctype_t rettype, const char *name, paramnode_t *params, int isdef) {
	symbol_t *s = find_global(name);
	if(!s) {
		s = calloc(1, sizeof(symbol_t));
		strcpy(s->name, name);
		s->kind = SYM_FUNC;
		s->type = rettype;
		sprintf(s->label, "F_%s", name);
		s->next = g_globals;
		g_globals = s;
	}
	if(!s->params) {
		param_t *tail = NULL;
		paramnode_t *pn;
		int n = 0;
		for(pn = params; pn; pn = pn->next) {
			param_t *pp = calloc(1, sizeof(param_t));
			strcpy(pp->name, pn->name);
			pp->type = pn->type;
			sprintf(pp->label, "V%d", ++varcounter);
			pp->next = NULL;
			if(!tail) s->params = pp; else tail->next = pp;
			tail = pp;
			n++;
		}
		s->nparams = n;
	}
	if(isdef) s->defined = 1;
	return s;
}

void start_func(ctype_t rettype, const char *name, paramnode_t *params) {
	param_t *p;
	symbol_t *s = declare_func(rettype, name, params, 1);
	curfunc = s;
	g_locals = NULL;
	emitc("%s:", s->label);
	for(p = s->params; p; p = p->next) {
		symbol_t *ls = calloc(1, sizeof(symbol_t));
		strcpy(ls->name, p->name);
		ls->kind = SYM_VAR;
		ls->type = p->type;
		strcpy(ls->label, p->label);
		ls->next = g_locals;
		g_locals = ls;
		emit_storage(p->label, &p->type);
	}
}

void end_func(void) {
	emitc("\tret");
	curfunc = NULL;
	g_locals = NULL;
}

void gen_return_void(void) {
	emitc("\tret");
}

void gen_return_value(void) {
	emitc("\tret");
}

/* ---------- loops ---------- */

void push_loop(const char *cont, const char *brk) {
	if(loopsp >= MAXLOOP) { semerror("loops aninhados demais"); return; }
	strcpy(loopstack[loopsp].contlbl, cont);
	strcpy(loopstack[loopsp].breaklbl, brk);
	loopsp++;
}

void pop_loop(void) {
	if(loopsp > 0) loopsp--;
}

const char *loop_cont_top(void) {
	if(loopsp <= 0) return "L0";
	return loopstack[loopsp - 1].contlbl;
}

void gen_break(void) {
	if(loopsp > 0) emitc("\tjmp %s", loopstack[loopsp - 1].breaklbl);
	else semerror("break fora de um loop");
}

void gen_continue(void) {
	if(loopsp > 0) emitc("\tjmp %s", loopstack[loopsp - 1].contlbl);
	else semerror("continue fora de um loop");
}

void begin_capture(void) {
	capture_stack[capture_depth++] = codeout;
	codeout = tmpfile();
}

char *end_capture(void) {
	long len;
	char *buf;
	FILE *cap = codeout;
	codeout = capture_stack[--capture_depth];
	len = ftell(cap);
	buf = malloc(len + 1);
	rewind(cap);
	len = fread(buf, 1, len, cap);
	buf[len] = 0;
	fclose(cap);
	return buf;
}

/* ---------- expression codegen ---------- */

void gen_rvalue(expr_t *e) {
	int isaggr = e->type.array || e->type.base == T_STRUCT;
	int flt = is_float_type(&e->type);
	switch(e->kind) {
		case 0:
			return;
		case 1:
			if(isaggr) {
				if(e->isref) emitc("\tmov ax,[%s]", e->label);
				else emitc("\tmov ax,%s", e->label);
			} else if(flt) {
				emitc("\tmov af,[%s]", e->label);
			} else {
				switch(basewidth(&e->type)) {
					case 1: emitc("\tmov ax,0"); emitc("\tmov ah,[%s]", e->label); break;
					case 2: emitc("\tmov ax,0"); emitc("\tmov aw,[%s]", e->label); break;
					default: emitc("\tmov ax,[%s]", e->label); break;
				}
			}
			break;
		case 2:
			emitc("\tpop cx");
			if(isaggr) {
				emitc("\tmov ax,cx");
			} else if(flt) {
				emitc("\tmov af,(cx)");
			} else {
				switch(basewidth(&e->type)) {
					case 1: emitc("\tmov ax,0"); emitc("\tmov ah,(cx)"); break;
					case 2: emitc("\tmov ax,0"); emitc("\tmov aw,(cx)"); break;
					default: emitc("\tmov ax,(cx)"); break;
				}
			}
			break;
	}
	e->kind = 0;
}

static void store_simple_or_pop(expr_t *l, int fromcx) {
	if(is_float_type(&l->type)) {
		if(fromcx) emitc("\tmov (cx),af");
		else emitc("\tmov [%s],af", l->label);
		return;
	}
	if(fromcx) {
		switch(basewidth(&l->type)) {
			case 1: emitc("\tmov (cx),ah"); break;
			case 2: emitc("\tmov (cx),aw"); break;
			default: emitc("\tmov (cx),ax"); break;
		}
	} else {
		switch(basewidth(&l->type)) {
			case 1: emitc("\tmov [%s],ah", l->label); break;
			case 2: emitc("\tmov [%s],aw", l->label); break;
			default: emitc("\tmov [%s],ax", l->label); break;
		}
	}
}

static void load_simple_or_cx(expr_t *l, int fromcx) {
	if(is_float_type(&l->type)) {
		if(fromcx) emitc("\tmov af,(cx)");
		else emitc("\tmov af,[%s]", l->label);
		return;
	}
	if(fromcx) {
		switch(basewidth(&l->type)) {
			case 1: emitc("\tmov ax,0"); emitc("\tmov ah,(cx)"); break;
			case 2: emitc("\tmov ax,0"); emitc("\tmov aw,(cx)"); break;
			default: emitc("\tmov ax,(cx)"); break;
		}
	} else {
		switch(basewidth(&l->type)) {
			case 1: emitc("\tmov ax,0"); emitc("\tmov ah,[%s]", l->label); break;
			case 2: emitc("\tmov ax,0"); emitc("\tmov aw,[%s]", l->label); break;
			default: emitc("\tmov ax,[%s]", l->label); break;
		}
	}
}

expr_t gen_assign(expr_t *l, expr_t *r, const char *op) {
	expr_t result;
	result.kind = 0;
	result.type = l->type;
	result.label[0] = 0;
	result.isref = 0;

	if(l->kind == 0) {
		semerror("expressao nao pode ser usada como alvo de atribuicao");
		return result;
	}

	if(strcmp(op, "preinc") == 0 || strcmp(op, "predec") == 0) {
		int flt = is_float_type(&l->type);
		const char *acc = flt ? "af" : "ax";
		const char *incdec = strcmp(op, "preinc") == 0 ? "inc" : "dec";
		if(l->kind == 1) {
			load_simple_or_cx(l, 0);
		} else {
			emitc("\tpop cx");
			load_simple_or_cx(l, 1);
		}
		if(flt) {
			emitc("\t%s %s", incdec, acc);
		} else {
			emitc("\t%s %s,1", strcmp(op, "preinc") == 0 ? "add" : "sub", acc);
		}
		if(l->kind == 1) store_simple_or_pop(l, 0);
		else store_simple_or_pop(l, 1);
		return result;
	}

	if(l->type.base == T_STRUCT || l->type.array) {
		/* whole-aggregate assignment: r has already been reduced to an
		   address value in ax (aggregate rvalues yield their address) */
		emitc("\tmov bx,ax");
		if(l->kind == 1) {
			if(l->isref) emitc("\tmov ax,[%s]", l->label);
			else emitc("\tmov ax,%s", l->label);
		} else {
			emitc("\tpop ax");
		}
		emitc("\tmov cx,%d", elemsize(&l->type) * (l->type.array ? l->type.arraylen : 1));
		emitc("\tbin");
		emitc("\tmova ax,bx");
		return result;
	}

	if(strcmp(op, "=") == 0) {
		if(l->kind == 1) {
			store_simple_or_pop(l, 0);
		} else {
			emitc("\tpop cx");
			store_simple_or_pop(l, 1);
		}
		return result;
	}

	/* compound assignment: ax (or af, if r is float) currently holds r's
	   already-evaluated value */
	{
		int flt = is_float_type(&l->type);
		const char *acc = flt ? "af" : "ax";
		const char *sec = flt ? "bf" : "bx";
		emitc("\tmov %s,%s", sec, acc);
		if(l->kind == 1) {
			load_simple_or_cx(l, 0);
		} else {
			emitc("\tpop cx");
			load_simple_or_cx(l, 1);
		}
		if(strcmp(op, "/") == 0) {
			emitc("\tdiv %s,%s", acc, sec);
		} else if(strcmp(op, "%") == 0) {
			emitc("\tdiv %s,%s", acc, sec);
			emitc("\tmov ax,dx");
		} else {
			emitc("\t%s %s,%s", op, acc, sec);
		}
		if(l->kind == 1) store_simple_or_pop(l, 0);
		else store_simple_or_pop(l, 1);
	}
	return result;
}

expr_t gen_binop(expr_t *l, expr_t *r, const char *mnem) {
	expr_t result;
	int flt = is_float_type(&l->type);
	const char *acc = flt ? "af" : "ax";
	const char *sec = flt ? "bf" : "bx";
	gen_rvalue(r);
	emitc("\tmov %s,%s", sec, acc);
	emitc("\tpop %s", acc);
	if(strcmp(mnem, "div") == 0) {
		emitc("\tdiv %s,%s", acc, sec);
	} else if(strcmp(mnem, "mod") == 0) {
		emitc("\tdiv %s,%s", acc, sec);
		emitc("\tmov ax,dx");
	} else {
		emitc("\t%s %s,%s", mnem, acc, sec);
	}
	result.kind = 0;
	result.type = promote(l->type, r->type);
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

static ctype_t inttype(void) {
	ctype_t t;
	t.base = T_INT; t.sdef = NULL; t.array = 0; t.arraylen = 0; t.isref = 0;
	return t;
}

expr_t gen_relop(expr_t *l, expr_t *r, const char *jmpmnem) {
	expr_t result;
	char *lt, *le;
	int flt = is_float_type(&l->type);
	const char *acc = flt ? "af" : "ax";
	const char *sec = flt ? "bf" : "bx";
	gen_rvalue(r);
	emitc("\tmov %s,%s", sec, acc);
	emitc("\tpop %s", acc);
	lt = newlabel();
	le = newlabel();
	emitc("\tcmp %s,%s", acc, sec);
	emitc("\t%s %s", jmpmnem, lt);
	emitc("\tmov ax,0");
	emitc("\tjmp %s", le);
	emitc("%s:", lt);
	emitc("\tmov ax,1");
	emitc("%s:", le);
	result.kind = 0;
	result.type = inttype();
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

static void gen_normalize_bool(void) {
	char *lt = newlabel();
	char *le = newlabel();
	emitc("\tcmp ax,0");
	emitc("\tjz %s", lt);
	emitc("\tmov ax,1");
	emitc("\tjmp %s", le);
	emitc("%s:", lt);
	emitc("\tmov ax,0");
	emitc("%s:", le);
}

expr_t gen_logic(expr_t *l, expr_t *r, const char *op) {
	expr_t result;
	gen_rvalue(r);
	gen_normalize_bool();
	emitc("\tmov bx,ax");
	emitc("\tpop ax");
	gen_normalize_bool();
	emitc("\t%s ax,bx", op);
	result.kind = 0;
	result.type = inttype();
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

expr_t gen_neg(expr_t *e) {
	expr_t result;
	int flt = is_float_type(&e->type);
	const char *acc = flt ? "af" : "ax";
	const char *sec = flt ? "bf" : "bx";
	gen_rvalue(e);
	emitc("\tmov %s,%s", sec, acc);
	emitc("\tmov %s,0", acc);
	emitc("\tsub %s,%s", acc, sec);
	result.kind = 0;
	result.type = e->type;
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

expr_t gen_not(expr_t *e) {
	expr_t result;
	char *lt = newlabel();
	char *le = newlabel();
	gen_rvalue(e);
	emitc("\tcmp ax,0");
	emitc("\tjz %s", lt);
	emitc("\tmov ax,0");
	emitc("\tjmp %s", le);
	emitc("%s:", lt);
	emitc("\tmov ax,1");
	emitc("%s:", le);
	result.kind = 0;
	result.type = inttype();
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

expr_t gen_bitnot(expr_t *e) {
	expr_t result;
	gen_rvalue(e);
	emitc("\tnot ax");
	result.kind = 0;
	result.type = e->type;
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

expr_t gen_index(expr_t *base, expr_t *idx) {
	expr_t result;
	ctype_t elemtype = base->type;
	int esz = elemsize(&base->type);
	elemtype.array = 0;
	elemtype.arraylen = 0;
	elemtype.isref = 0;

	gen_rvalue(idx);
	emitc("\tpush ax");

	if(base->kind == 1) {
		emitc("\tpop bx");
		if(base->isref) emitc("\tmov cx,[%s]", base->label);
		else emitc("\tmov cx,%s", base->label);
	} else {
		emitc("\tpop bx");
		emitc("\tpop cx");
	}
	emitc("\tmov ax,%d", esz);
	emitc("\tmul bx,ax");
	emitc("\tadd cx,bx");
	emitc("\tpush cx");

	result.kind = 2;
	result.type = elemtype;
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

expr_t gen_field(expr_t *base, const char *fieldname) {
	expr_t result;
	structdef_t *sd = base->type.sdef;
	field_t *f = find_field(sd, fieldname);

	if(!sd) semerror("acesso a campo em algo que nao eh struct");
	else if(!f) semerror("campo '%s' nao existe na struct '%s'", fieldname, sd->name);

	if(base->kind == 1) {
		if(base->isref) emitc("\tmov cx,[%s]", base->label);
		else emitc("\tmov cx,%s", base->label);
	} else {
		emitc("\tpop cx");
	}
	if(f && f->offset) emitc("\tadd cx,%d", f->offset);
	emitc("\tpush cx");

	result.kind = 2;
	result.type = f ? f->type : inttype();
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

static void emit_string_data(const char *label, const char *s) {
	int i, n = (int)strlen(s);
	int inquote = 0;
	int first = 1;
	fprintf(dataout, "%s:\tdb ", label);
	for(i = 0; i < n; i++) {
		unsigned char c = (unsigned char)s[i];
		if(c >= 32 && c <= 126 && c != '"') {
			if(!inquote) { if(!first) fprintf(dataout, ","); fprintf(dataout, "\""); inquote = 1; }
			fputc(c, dataout);
		} else {
			if(inquote) { fprintf(dataout, "\""); inquote = 0; fprintf(dataout, ","); first = 0; }
			else if(!first) fprintf(dataout, ",");
			fprintf(dataout, "%d", c);
			first = 0;
			continue;
		}
		first = 0;
	}
	if(inquote) fprintf(dataout, "\"");
	if(!first) fprintf(dataout, ",");
	fprintf(dataout, "0\n");
}

expr_t gen_ident(const char *name) {
	expr_t result;
	symbol_t *s = find_symbol(name);
	if(!s || s->kind != SYM_VAR) {
		semerror("variavel '%s' nao declarada", name);
		result.kind = 0;
		result.type.base = T_LONG; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		result.label[0] = 0;
		result.isref = 0;
		emitc("\tmov ax,0");
		return result;
	}
	result.kind = 1;
	result.type = s->type;
	strcpy(result.label, s->label);
	result.isref = s->type.isref;
	return result;
}

expr_t gen_intlit(int v) {
	expr_t result;
	emitc("\tmov ax,%d", v);
	result.kind = 0;
	result.type.base = T_LONG; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

expr_t gen_charlit(int v) {
	expr_t result;
	emitc("\tmov ax,%d", v);
	result.kind = 0;
	result.type.base = T_CHAR; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

expr_t gen_floatlit(double v) {
	expr_t result;
	char label[32];
	union { float f; int i; } conv;

	/* instruction immediates only carry 20 bits in this ISA, nowhere near
	   enough for an arbitrary IEEE-754 bit pattern, so the literal is
	   stored as a data constant (which has no such limit) and loaded
	   through direct addressing instead of "mov af,<bits>". */
	conv.f = (float)v;
	sprintf(label, "K%d", ++strcounter);
	emitd("%s:\tfl %d", label, conv.i);
	emitc("\tmov af,[%s]", label);
	result.kind = 0;
	result.type.base = T_FLOAT; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

expr_t gen_stringlit(const char *s) {
	expr_t result;
	char label[32];
	sprintf(label, "S%d", ++strcounter);
	emit_string_data(label, s);
	emitc("\tmov ax,%s", label);
	result.kind = 0;
	result.type.base = T_CHAR; result.type.sdef = NULL; result.type.array = 1;
	result.type.arraylen = (int)strlen(s) + 1; result.type.isref = 0;
	result.label[0] = 0;
	result.isref = 0;
	return result;
}

expr_t gen_call(const char *name, int argcount) {
	expr_t result;
	symbol_t *f;
	int i;

	result.label[0] = 0;
	result.isref = 0;

	if(strcmp(name, "print") == 0) {
		if(argcount != 1) semerror("print espera 1 argumento");
		emitc("\tpop ax");
		emitc("\tsys print,ax");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "debug") == 0) {
		if(argcount != 1) semerror("debug espera 1 argumento");
		emitc("\tpop ax");
		emitc("\tsys debug,ax");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "halt") == 0) {
		if(argcount != 1) semerror("halt espera 1 argumento");
		emitc("\tpop ax");
		emitc("\tsys halt,ax");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "printf") == 0) {
		if(argcount != 2) semerror("printf espera 2 argumentos");
		emitc("\tpop bx");
		emitc("\tpop ax");
		emitc("\tsys prints,ax");
		emitc("\tmov ax,bx");
		emitc("\tsys printf,ax");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "input") == 0) {
		if(argcount != 1) semerror("input espera 1 argumento (porta)");
		emitc("\tpop bx");
		emitc("\tin ax,bx");
		result.kind = 0;
		result.type.base = T_INT; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "output") == 0) {
		if(argcount != 2) semerror("output espera 2 argumentos (porta, valor)");
		emitc("\tpop ax");
		emitc("\tpop bx");
		emitc("\tout ax,bx");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "delay") == 0) {
		if(argcount != 1) semerror("delay espera 1 argumento");
		emitc("\tpop ax");
		emitc("\tdelay ax");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "getch") == 0) {
		if(argcount != 1) semerror("getch espera 1 argumento (timeout)");
		emitc("\tpop bx");
		emitc("\tsys getch,bx");
		result.kind = 0;
		result.type.base = T_INT; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "syscall") == 0) {
		if(argcount != 2) semerror("syscall espera 2 argumentos (number, param)");
		emitc("\tpop bx");
		emitc("\tpop ax");
		emitc("\tsys ax,bx");
		result.kind = 0;
		result.type.base = T_INT; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "inputpin") == 0) {
		if(argcount != 1) semerror("inputpin espera 1 argumento (pin)");
		emitc("\tpop bx");
		emitc("\tsys inputpin,bx");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "outputpin") == 0) {
		if(argcount != 1) semerror("outputpin espera 1 argumento (pin)");
		emitc("\tpop bx");
		emitc("\tsys outputpin,bx");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "pulluppin") == 0) {
		if(argcount != 1) semerror("pulluppin espera 1 argumento (pin)");
		emitc("\tpop bx");
		emitc("\tsys pullup,bx");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}

	f = find_symbol(name);
	if(!f || f->kind != SYM_FUNC) {
		semerror("funcao '%s' nao declarada", name);
		for(i = 0; i < argcount; i++) emitc("\tpop ax");
		result.kind = 0;
		result.type.base = T_LONG; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(argcount != f->nparams) {
		semerror("numero incorreto de argumentos para '%s' (esperado %d, recebido %d)", name, f->nparams, argcount);
	}
	{
		int n = f->nparams;
		param_t **arr = malloc(sizeof(param_t*) * (n > 0 ? n : 1));
		param_t *p = f->params;
		int idx;
		for(idx = 0; idx < n; idx++) { arr[idx] = p; p = p->next; }
		for(idx = n - 1; idx >= 0; idx--) {
			if(is_float_type(&arr[idx]->type)) {
				emitc("\tpop af");
				emitc("\tmov [%s],af", arr[idx]->label);
			} else {
				emitc("\tpop ax");
				switch(basewidth(&arr[idx]->type)) {
					case 1: emitc("\tmov [%s],ah", arr[idx]->label); break;
					case 2: emitc("\tmov [%s],aw", arr[idx]->label); break;
					default: emitc("\tmov [%s],ax", arr[idx]->label); break;
				}
			}
		}
		free(arr);
		for(i = n; i < argcount; i++) emitc("\tpop ax"); /* discard extras on mismatch */
	}
	emitc("\tcall %s", f->label);
	result.kind = 0;
	result.type = f->type;
	return result;
}

/* ---------- driver ---------- */

int main(int argc, char **argv) {
	char buf[4096];
	size_t n;
	int i;
	char *inname = NULL;
	char *outname = NULL;
	FILE *out;

	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-o") == 0 && i+1 < argc) {
			outname = argv[++i];
		} else {
			if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0) {
				printf( "C: VM Arpia small C compiler (c) 2026 - v0.02\n"
						"Use: c [-o <output>] [<input>]\n"
						"     -o <output> output assembly source, when ommited write to the stdout\n"
						"     <input> input C source, when ommited read from stdin\n\n");
				return(0);
			} else {
				inname = argv[i];
			}
		}
	}
	if(inname) {
		yyin = fopen(inname, "r");
		if(!yyin) {
			fprintf(stderr, "Erro ao abrir arquivo de entrada: %s\n", inname);
			return 1;
		}
	}

	codeout = tmpfile();
	dataout = tmpfile();

	emitc("\tsm20");
	emitc("\tcall F_main");
	emitc("\tsys halt,0");

	yyparse();

	if(!parse_ok) return 1;

	out = stdout;
	if(outname) {
		out = fopen(outname, "w");
		if(!out) {
			fprintf(stderr, "Erro ao abrir arquivo de saida: %s\n", outname);
			return 1;
		}
	}

	fprintf(out, ".name \"programa\"\n\n.code\n");
	rewind(codeout);
	while((n = fread(buf, 1, sizeof(buf), codeout)) > 0) fwrite(buf, 1, n, out);
	fprintf(out, "\n.data\n");
	rewind(dataout);
	while((n = fread(buf, 1, sizeof(buf), dataout)) > 0) fwrite(buf, 1, n, out);
	fprintf(out, "\n.end\n");
	return 0;
}

int yyerror(const char *s) {
	fprintf(stderr, "Erro de sintaxe na linha %d: %s (proximo a '%s')\n", yylineno, s, yytext);
	parse_ok = 0;
	return 0;
}
