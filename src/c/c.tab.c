/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "c.y"

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


#line 230 "c.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "c.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENT = 3,                      /* IDENT  */
  YYSYMBOL_STRINGLIT = 4,                  /* STRINGLIT  */
  YYSYMBOL_NUMBER = 5,                     /* NUMBER  */
  YYSYMBOL_CHARLIT = 6,                    /* CHARLIT  */
  YYSYMBOL_FLOATNUM = 7,                   /* FLOATNUM  */
  YYSYMBOL_KW_CHAR = 8,                    /* KW_CHAR  */
  YYSYMBOL_KW_INT = 9,                     /* KW_INT  */
  YYSYMBOL_KW_LONG = 10,                   /* KW_LONG  */
  YYSYMBOL_KW_FLOAT = 11,                  /* KW_FLOAT  */
  YYSYMBOL_KW_VOID = 12,                   /* KW_VOID  */
  YYSYMBOL_KW_STRUCT = 13,                 /* KW_STRUCT  */
  YYSYMBOL_KW_IF = 14,                     /* KW_IF  */
  YYSYMBOL_KW_ELSE = 15,                   /* KW_ELSE  */
  YYSYMBOL_KW_WHILE = 16,                  /* KW_WHILE  */
  YYSYMBOL_KW_FOR = 17,                    /* KW_FOR  */
  YYSYMBOL_KW_RETURN = 18,                 /* KW_RETURN  */
  YYSYMBOL_KW_BREAK = 19,                  /* KW_BREAK  */
  YYSYMBOL_KW_CONTINUE = 20,               /* KW_CONTINUE  */
  YYSYMBOL_SHLEQ = 21,                     /* SHLEQ  */
  YYSYMBOL_SHREQ = 22,                     /* SHREQ  */
  YYSYMBOL_PLUSEQ = 23,                    /* PLUSEQ  */
  YYSYMBOL_MINUSEQ = 24,                   /* MINUSEQ  */
  YYSYMBOL_STAREQ = 25,                    /* STAREQ  */
  YYSYMBOL_SLASHEQ = 26,                   /* SLASHEQ  */
  YYSYMBOL_PERCENTEQ = 27,                 /* PERCENTEQ  */
  YYSYMBOL_ANDEQ = 28,                     /* ANDEQ  */
  YYSYMBOL_OREQ = 29,                      /* OREQ  */
  YYSYMBOL_XOREQ = 30,                     /* XOREQ  */
  YYSYMBOL_EQ = 31,                        /* EQ  */
  YYSYMBOL_NE = 32,                        /* NE  */
  YYSYMBOL_LE = 33,                        /* LE  */
  YYSYMBOL_GE = 34,                        /* GE  */
  YYSYMBOL_ANDAND = 35,                    /* ANDAND  */
  YYSYMBOL_OROR = 36,                      /* OROR  */
  YYSYMBOL_SHL = 37,                       /* SHL  */
  YYSYMBOL_SHR = 38,                       /* SHR  */
  YYSYMBOL_INCOP = 39,                     /* INCOP  */
  YYSYMBOL_DECOP = 40,                     /* DECOP  */
  YYSYMBOL_LOWER_THAN_ELSE = 41,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_42_ = 42,                       /* '|'  */
  YYSYMBOL_43_ = 43,                       /* '^'  */
  YYSYMBOL_44_ = 44,                       /* '&'  */
  YYSYMBOL_45_ = 45,                       /* '<'  */
  YYSYMBOL_46_ = 46,                       /* '>'  */
  YYSYMBOL_47_ = 47,                       /* '+'  */
  YYSYMBOL_48_ = 48,                       /* '-'  */
  YYSYMBOL_49_ = 49,                       /* '*'  */
  YYSYMBOL_50_ = 50,                       /* '/'  */
  YYSYMBOL_51_ = 51,                       /* '%'  */
  YYSYMBOL_UMINUS = 52,                    /* UMINUS  */
  YYSYMBOL_UNOT = 53,                      /* UNOT  */
  YYSYMBOL_UBITNOT = 54,                   /* UBITNOT  */
  YYSYMBOL_55_ = 55,                       /* ';'  */
  YYSYMBOL_56_ = 56,                       /* '{'  */
  YYSYMBOL_57_ = 57,                       /* '}'  */
  YYSYMBOL_58_ = 58,                       /* ','  */
  YYSYMBOL_59_ = 59,                       /* '['  */
  YYSYMBOL_60_ = 60,                       /* ']'  */
  YYSYMBOL_61_ = 61,                       /* '='  */
  YYSYMBOL_62_ = 62,                       /* '('  */
  YYSYMBOL_63_ = 63,                       /* ')'  */
  YYSYMBOL_64_ = 64,                       /* '!'  */
  YYSYMBOL_65_ = 65,                       /* '~'  */
  YYSYMBOL_66_ = 66,                       /* '.'  */
  YYSYMBOL_YYACCEPT = 67,                  /* $accept  */
  YYSYMBOL_program = 68,                   /* program  */
  YYSYMBOL_toplevel_list = 69,             /* toplevel_list  */
  YYSYMBOL_toplevel = 70,                  /* toplevel  */
  YYSYMBOL_struct_def = 71,                /* struct_def  */
  YYSYMBOL_72_1 = 72,                      /* $@1  */
  YYSYMBOL_member_list = 73,               /* member_list  */
  YYSYMBOL_member = 74,                    /* member  */
  YYSYMBOL_member_declarator_list = 75,    /* member_declarator_list  */
  YYSYMBOL_member_declarator = 76,         /* member_declarator  */
  YYSYMBOL_type_spec = 77,                 /* type_spec  */
  YYSYMBOL_global_decl = 78,               /* global_decl  */
  YYSYMBOL_global_declarator_list = 79,    /* global_declarator_list  */
  YYSYMBOL_global_declarator = 80,         /* global_declarator  */
  YYSYMBOL_func_proto = 81,                /* func_proto  */
  YYSYMBOL_func_def = 82,                  /* func_def  */
  YYSYMBOL_83_2 = 83,                      /* $@2  */
  YYSYMBOL_param_list_opt = 84,            /* param_list_opt  */
  YYSYMBOL_param_list = 85,                /* param_list  */
  YYSYMBOL_param = 86,                     /* param  */
  YYSYMBOL_block_item_list = 87,           /* block_item_list  */
  YYSYMBOL_block_item = 88,                /* block_item  */
  YYSYMBOL_local_decl = 89,                /* local_decl  */
  YYSYMBOL_local_declarator_list = 90,     /* local_declarator_list  */
  YYSYMBOL_local_declarator = 91,          /* local_declarator  */
  YYSYMBOL_statement = 92,                 /* statement  */
  YYSYMBOL_93_3 = 93,                      /* @3  */
  YYSYMBOL_94_4 = 94,                      /* @4  */
  YYSYMBOL_95_5 = 95,                      /* @5  */
  YYSYMBOL_96_6 = 96,                      /* @6  */
  YYSYMBOL_97_7 = 97,                      /* @7  */
  YYSYMBOL_98_8 = 98,                      /* @8  */
  YYSYMBOL_opt_expr = 99,                  /* opt_expr  */
  YYSYMBOL_if_head = 100,                  /* if_head  */
  YYSYMBOL_assignment_expr = 101,          /* assignment_expr  */
  YYSYMBOL_expr = 102,                     /* expr  */
  YYSYMBOL_103_9 = 103,                    /* $@9  */
  YYSYMBOL_104_10 = 104,                   /* $@10  */
  YYSYMBOL_105_11 = 105,                   /* $@11  */
  YYSYMBOL_106_12 = 106,                   /* $@12  */
  YYSYMBOL_107_13 = 107,                   /* $@13  */
  YYSYMBOL_108_14 = 108,                   /* $@14  */
  YYSYMBOL_109_15 = 109,                   /* $@15  */
  YYSYMBOL_110_16 = 110,                   /* $@16  */
  YYSYMBOL_111_17 = 111,                   /* $@17  */
  YYSYMBOL_112_18 = 112,                   /* $@18  */
  YYSYMBOL_113_19 = 113,                   /* $@19  */
  YYSYMBOL_114_20 = 114,                   /* $@20  */
  YYSYMBOL_115_21 = 115,                   /* $@21  */
  YYSYMBOL_116_22 = 116,                   /* $@22  */
  YYSYMBOL_117_23 = 117,                   /* $@23  */
  YYSYMBOL_118_24 = 118,                   /* $@24  */
  YYSYMBOL_119_25 = 119,                   /* $@25  */
  YYSYMBOL_120_26 = 120,                   /* $@26  */
  YYSYMBOL_postfix_expr = 121,             /* postfix_expr  */
  YYSYMBOL_call_args_opt = 122,            /* call_args_opt  */
  YYSYMBOL_call_args = 123,                /* call_args  */
  YYSYMBOL_primary_expr = 124              /* primary_expr  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   471

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  58
/* YYNRULES -- Number of rules.  */
#define YYNRULES  138
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  234

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   299


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    64,     2,     2,     2,    51,    44,     2,
      62,    63,    49,    47,    58,    48,    66,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    55,
      45,    61,    46,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    59,     2,    60,    43,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,    42,    57,    65,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    52,    53,    54
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   202,   202,   205,   207,   211,   212,   213,   214,   218,
     218,   222,   223,   227,   231,   232,   236,   237,   241,   242,
     243,   244,   245,   246,   251,   255,   256,   260,   261,   262,
     263,   267,   271,   271,   276,   277,   278,   282,   283,   287,
     288,   291,   293,   297,   298,   302,   306,   307,   311,   312,
     313,   317,   318,   319,   320,   323,   322,   327,   329,   326,
     333,   335,   341,   332,   348,   349,   350,   351,   355,   356,
     360,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   380,   380,   381,   381,   382,   382,   383,
     383,   384,   384,   385,   385,   386,   386,   387,   387,   388,
     388,   389,   389,   390,   390,   391,   391,   392,   392,   393,
     393,   394,   394,   395,   395,   396,   396,   397,   397,   398,
     399,   400,   401,   402,   403,   407,   408,   409,   410,   414,
     415,   419,   420,   424,   425,   426,   427,   428,   429
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENT", "STRINGLIT",
  "NUMBER", "CHARLIT", "FLOATNUM", "KW_CHAR", "KW_INT", "KW_LONG",
  "KW_FLOAT", "KW_VOID", "KW_STRUCT", "KW_IF", "KW_ELSE", "KW_WHILE",
  "KW_FOR", "KW_RETURN", "KW_BREAK", "KW_CONTINUE", "SHLEQ", "SHREQ",
  "PLUSEQ", "MINUSEQ", "STAREQ", "SLASHEQ", "PERCENTEQ", "ANDEQ", "OREQ",
  "XOREQ", "EQ", "NE", "LE", "GE", "ANDAND", "OROR", "SHL", "SHR", "INCOP",
  "DECOP", "LOWER_THAN_ELSE", "'|'", "'^'", "'&'", "'<'", "'>'", "'+'",
  "'-'", "'*'", "'/'", "'%'", "UMINUS", "UNOT", "UBITNOT", "';'", "'{'",
  "'}'", "','", "'['", "']'", "'='", "'('", "')'", "'!'", "'~'", "'.'",
  "$accept", "program", "toplevel_list", "toplevel", "struct_def", "$@1",
  "member_list", "member", "member_declarator_list", "member_declarator",
  "type_spec", "global_decl", "global_declarator_list",
  "global_declarator", "func_proto", "func_def", "$@2", "param_list_opt",
  "param_list", "param", "block_item_list", "block_item", "local_decl",
  "local_declarator_list", "local_declarator", "statement", "@3", "@4",
  "@5", "@6", "@7", "@8", "opt_expr", "if_head", "assignment_expr", "expr",
  "$@9", "$@10", "$@11", "$@12", "$@13", "$@14", "$@15", "$@16", "$@17",
  "$@18", "$@19", "$@20", "$@21", "$@22", "$@23", "$@24", "$@25", "$@26",
  "postfix_expr", "call_args_opt", "call_args", "primary_expr", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-215)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-36)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -215,     2,   185,  -215,  -215,  -215,  -215,  -215,  -215,     0,
    -215,  -215,    11,   -21,   -10,  -215,    21,   -36,   -12,  -215,
    -215,  -215,  -215,    82,    76,   213,    86,   248,    48,  -215,
    -215,    46,   107,   108,    50,    65,  -215,    23,  -215,    19,
    -215,   121,  -215,  -215,    66,    80,   248,    83,  -215,    87,
     -22,  -215,    79,  -215,  -215,  -215,   138,  -215,   121,  -215,
    -215,    85,  -215,   172,  -215,    88,  -215,  -215,  -215,  -215,
     125,   141,   142,   281,    92,    93,    13,    13,   300,  -215,
    -215,  -215,   300,   300,   300,   204,  -215,   153,  -215,   262,
     154,     6,   328,  -215,   300,   300,  -215,   300,  -215,   155,
    -215,  -215,   -55,   -55,  -215,   -55,   235,   167,  -215,  -215,
      27,   173,  -215,  -215,   217,  -215,  -215,  -215,  -215,  -215,
    -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,
    -215,  -215,  -215,  -215,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   230,  -215,   187,   177,
     199,   300,   208,  -215,  -215,  -215,  -215,   259,   300,   204,
    -215,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,  -215,
    -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,   335,
    -215,  -215,  -215,   300,  -215,   207,  -215,   211,  -215,  -215,
     262,   420,   420,   104,   104,   359,    84,   264,   264,   380,
     401,   168,   104,   104,     9,     9,  -215,  -215,  -215,  -215,
    -215,  -215,   300,  -215,  -215,   262,   218,  -215,  -215,   300,
    -215,   209,   262,  -215
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,    18,    19,    20,    21,    22,     0,
       4,     5,     0,     0,     0,     6,    23,    27,    24,    25,
       8,     7,     9,     0,     0,    34,     0,     0,     0,    29,
      30,    22,     0,     0,     0,    36,    37,    27,    26,     0,
      11,     0,    28,    23,    39,    31,     0,     0,    12,    16,
       0,    14,     0,    32,    38,    10,     0,    13,     0,    40,
      41,     0,    15,     0,    17,   133,   137,   134,   135,   136,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
      41,    33,     0,     0,     0,     0,    42,     0,    43,     0,
       0,    82,   124,   125,   129,     0,    57,    68,    64,     0,
      66,    67,   122,   123,   119,   124,     0,     0,   120,   121,
      48,    45,    46,    44,    54,    53,   111,   113,   107,   109,
     115,   117,    99,   101,    95,    97,    93,   103,   105,    83,
      85,    87,    89,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   131,     0,   130,
       0,     0,     0,    69,    65,    52,   138,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    80,
      81,    72,    73,    74,    75,    76,    77,    78,    79,     0,
      71,   127,   128,     0,    70,     0,    60,     0,    50,    47,
       0,   112,   114,   108,   110,   116,   118,   100,   102,    96,
      98,    94,   104,   106,    84,    86,    88,    90,    92,   126,
     132,    58,    68,    49,    56,     0,     0,    59,    61,    68,
      62,     0,     0,    63
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -215,  -215,  -215,  -215,  -215,  -215,  -215,   238,  -215,   231,
       8,  -215,  -215,   267,  -215,  -215,  -215,  -215,  -215,   249,
     214,  -215,  -215,  -215,   137,   -88,  -215,  -215,  -215,  -215,
    -215,  -215,  -214,  -215,   -73,    -4,  -215,  -215,  -215,  -215,
    -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,
    -215,  -215,  -215,  -215,   -71,  -215,  -215,  -215
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,     2,    10,    11,    27,    39,    40,    50,    51,
      33,    13,    18,    19,    14,    15,    60,    34,    35,    36,
      63,    86,    87,   111,   112,    88,   200,   151,   225,   222,
     229,   231,   152,    89,    90,    91,   174,   175,   176,   177,
     178,   171,   169,   170,   167,   168,   172,   173,   163,   164,
     161,   162,   165,   166,    92,   148,   149,    93
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      99,   114,     3,    16,   144,   102,   103,   105,   226,   107,
      12,   146,   105,   105,    17,   230,    65,    66,    67,    68,
      69,   147,   150,    23,   153,    24,    25,     4,     5,     6,
       7,     8,    32,    57,    20,    41,    58,   116,   117,   118,
     119,   120,   121,   122,   123,    21,    26,    41,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   131,   132,
     133,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,    85,   190,   105,   104,    82,    47,    22,   195,   108,
     109,    29,    23,    30,    24,   198,   157,    28,   158,    37,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,    42,   -35,
      43,    44,   224,    45,    85,   116,   117,   118,   119,   120,
     220,   122,   123,    46,    49,    52,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,    53,   227,    55,    59,
     189,   122,   123,    61,   233,    64,    56,   100,   101,   153,
      94,   129,   130,   131,   132,   133,   153,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,    65,    66,    67,    68,    69,
       4,     5,     6,     7,     8,    32,    70,    95,    71,    72,
      73,    74,    75,     4,     5,     6,     7,     8,     9,   116,
     117,   118,   119,    96,    97,   122,   123,   110,   113,   115,
     154,    76,    77,   127,   128,   129,   130,   131,   132,   133,
      78,     4,     5,     6,     7,    31,    32,    79,    80,    81,
     156,   159,   160,   191,    82,   193,    83,    84,    65,    66,
      67,    68,    69,     4,     5,     6,     7,     8,    32,    70,
     192,    71,    72,    73,    74,    75,     4,     5,     6,     7,
       8,    32,   194,   196,   197,    65,    66,    67,    68,    69,
     221,   223,   232,   228,    76,    77,    70,    48,    71,    72,
      73,    74,    75,    78,    65,    66,    67,    68,    69,    62,
      79,    80,   155,    38,   106,    54,   199,    82,     0,    83,
      84,    76,    77,    65,    66,    67,    68,    69,     0,     0,
      78,   129,   130,   131,   132,   133,     0,    79,    80,     0,
      76,    77,     0,     0,    82,     0,    83,    84,     0,    78,
       0,     0,     0,     0,     0,     0,    98,     0,     0,    76,
      77,     0,     0,    82,     0,    83,    84,     0,    78,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,     0,
       0,     0,    82,     0,    83,    84,   116,   117,   118,   119,
     120,   121,   122,   123,     0,     0,     0,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   144,     0,   145,
     116,   117,   118,   119,   146,   219,   122,   123,     0,     0,
       0,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   116,   117,   118,   119,     0,     0,   122,   123,     0,
       0,     0,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   116,   117,   118,   119,     0,     0,   122,   123,
       0,     0,     0,     0,     0,   126,   127,   128,   129,   130,
     131,   132,   133,   118,   119,     0,     0,   122,   123,     0,
       0,     0,     0,     0,     0,   127,   128,   129,   130,   131,
     132,   133
};

static const yytype_int16 yycheck[] =
{
      73,    89,     0,     3,    59,    76,    77,    78,   222,    82,
       2,    66,    83,    84,     3,   229,     3,     4,     5,     6,
       7,    94,    95,    59,    97,    61,    62,     8,     9,    10,
      11,    12,    13,    55,    55,    27,    58,    31,    32,    33,
      34,    35,    36,    37,    38,    55,    58,    39,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    49,    50,
      51,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,    63,   145,   144,    78,    62,    57,    56,   151,    83,
      84,     5,    59,     7,    61,   158,    59,     5,    61,     3,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,    60,    63,
       3,     3,   200,    63,   106,    31,    32,    33,    34,    35,
     193,    37,    38,    58,     3,    59,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    56,   225,    55,    60,
     144,    37,    38,     5,   232,    60,    59,    55,    55,   222,
      62,    47,    48,    49,    50,    51,   229,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    62,    16,    17,
      18,    19,    20,     8,     9,    10,    11,    12,    13,    31,
      32,    33,    34,    62,    62,    37,    38,     3,    55,    55,
      55,    39,    40,    45,    46,    47,    48,    49,    50,    51,
      48,     8,     9,    10,    11,    12,    13,    55,    56,    57,
      63,    58,    15,     3,    62,    58,    64,    65,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      63,    16,    17,    18,    19,    20,     8,     9,    10,    11,
      12,    13,    63,    55,     5,     3,     4,     5,     6,     7,
      63,    60,    63,    55,    39,    40,    14,    39,    16,    17,
      18,    19,    20,    48,     3,     4,     5,     6,     7,    58,
      55,    56,    57,    26,    80,    46,   159,    62,    -1,    64,
      65,    39,    40,     3,     4,     5,     6,     7,    -1,    -1,
      48,    47,    48,    49,    50,    51,    -1,    55,    56,    -1,
      39,    40,    -1,    -1,    62,    -1,    64,    65,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    39,
      40,    -1,    -1,    62,    -1,    64,    65,    -1,    48,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
      -1,    -1,    62,    -1,    64,    65,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    59,    -1,    61,
      31,    32,    33,    34,    66,    60,    37,    38,    -1,    -1,
      -1,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    31,    32,    33,    34,    -1,    -1,    37,    38,    -1,
      -1,    -1,    -1,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    31,    32,    33,    34,    -1,    -1,    37,    38,
      -1,    -1,    -1,    -1,    -1,    44,    45,    46,    47,    48,
      49,    50,    51,    33,    34,    -1,    -1,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    48,    49,
      50,    51
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    68,    69,     0,     8,     9,    10,    11,    12,    13,
      70,    71,    77,    78,    81,    82,     3,     3,    79,    80,
      55,    55,    56,    59,    61,    62,    58,    72,     5,     5,
       7,    12,    13,    77,    84,    85,    86,     3,    80,    73,
      74,    77,    60,     3,     3,    63,    58,    57,    74,     3,
      75,    76,    59,    56,    86,    55,    59,    55,    58,    60,
      83,     5,    76,    87,    60,     3,     4,     5,     6,     7,
      14,    16,    17,    18,    19,    20,    39,    40,    48,    55,
      56,    57,    62,    64,    65,    77,    88,    89,    92,   100,
     101,   102,   121,   124,    62,    62,    62,    62,    55,   101,
      55,    55,   121,   121,   102,   121,    87,   101,   102,   102,
       3,    90,    91,    55,    92,    55,    31,    32,    33,    34,
      35,    36,    37,    38,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    59,    61,    66,   101,   122,   123,
     101,    94,    99,   101,    55,    57,    63,    59,    61,    58,
      15,   117,   118,   115,   116,   119,   120,   111,   112,   109,
     110,   108,   113,   114,   103,   104,   105,   106,   107,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   102,
     101,     3,    63,    58,    63,   101,    55,     5,   101,    91,
      93,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,    60,
     101,    63,    96,    60,    92,    95,    99,    92,    55,    97,
      99,    98,    63,    92
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    67,    68,    69,    69,    70,    70,    70,    70,    72,
      71,    73,    73,    74,    75,    75,    76,    76,    77,    77,
      77,    77,    77,    77,    78,    79,    79,    80,    80,    80,
      80,    81,    83,    82,    84,    84,    84,    85,    85,    86,
      86,    87,    87,    88,    88,    89,    90,    90,    91,    91,
      91,    92,    92,    92,    92,    93,    92,    94,    95,    92,
      96,    97,    98,    92,    92,    92,    92,    92,    99,    99,
     100,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   103,   102,   104,   102,   105,   102,   106,
     102,   107,   102,   108,   102,   109,   102,   110,   102,   111,
     102,   112,   102,   113,   102,   114,   102,   115,   102,   116,
     102,   117,   102,   118,   102,   119,   102,   120,   102,   102,
     102,   102,   102,   102,   102,   121,   121,   121,   121,   122,
     122,   123,   123,   124,   124,   124,   124,   124,   124
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     1,     1,     2,     2,     0,
       7,     1,     2,     3,     1,     3,     1,     4,     1,     1,
       1,     1,     1,     2,     2,     1,     3,     1,     4,     3,
       3,     5,     0,     9,     0,     1,     1,     1,     3,     2,
       4,     0,     2,     1,     2,     2,     1,     3,     1,     4,
       3,     1,     3,     2,     2,     0,     5,     0,     0,     7,
       0,     0,     0,    12,     2,     3,     2,     2,     0,     1,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     2,
       2,     2,     2,     2,     1,     1,     4,     3,     4,     0,
       1,     1,     3,     1,     1,     1,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 9: /* $@1: %empty  */
#line 218 "c.y"
                              { begin_struct((yyvsp[-1].sval)); }
#line 1560 "c.tab.c"
    break;

  case 10: /* struct_def: KW_STRUCT IDENT '{' $@1 member_list '}' ';'  */
#line 218 "c.y"
                                                                        { end_struct(); }
#line 1566 "c.tab.c"
    break;

  case 16: /* member_declarator: IDENT  */
#line 236 "c.y"
                                         { add_field(curdecltype, (yyvsp[0].sval), 0); }
#line 1572 "c.tab.c"
    break;

  case 17: /* member_declarator: IDENT '[' NUMBER ']'  */
#line 237 "c.y"
                                         { add_field(curdecltype, (yyvsp[-3].sval), (yyvsp[-1].ival)); }
#line 1578 "c.tab.c"
    break;

  case 18: /* type_spec: KW_CHAR  */
#line 241 "c.y"
                                         { (yyval.ctype).base=T_CHAR; (yyval.ctype).sdef=NULL; (yyval.ctype).array=0; (yyval.ctype).arraylen=0; (yyval.ctype).isref=0; curdecltype=(yyval.ctype); }
#line 1584 "c.tab.c"
    break;

  case 19: /* type_spec: KW_INT  */
#line 242 "c.y"
                                         { (yyval.ctype).base=T_INT; (yyval.ctype).sdef=NULL; (yyval.ctype).array=0; (yyval.ctype).arraylen=0; (yyval.ctype).isref=0; curdecltype=(yyval.ctype); }
#line 1590 "c.tab.c"
    break;

  case 20: /* type_spec: KW_LONG  */
#line 243 "c.y"
                                         { (yyval.ctype).base=T_LONG; (yyval.ctype).sdef=NULL; (yyval.ctype).array=0; (yyval.ctype).arraylen=0; (yyval.ctype).isref=0; curdecltype=(yyval.ctype); }
#line 1596 "c.tab.c"
    break;

  case 21: /* type_spec: KW_FLOAT  */
#line 244 "c.y"
                                         { (yyval.ctype).base=T_FLOAT; (yyval.ctype).sdef=NULL; (yyval.ctype).array=0; (yyval.ctype).arraylen=0; (yyval.ctype).isref=0; curdecltype=(yyval.ctype); }
#line 1602 "c.tab.c"
    break;

  case 22: /* type_spec: KW_VOID  */
#line 245 "c.y"
                                         { (yyval.ctype).base=T_VOID; (yyval.ctype).sdef=NULL; (yyval.ctype).array=0; (yyval.ctype).arraylen=0; (yyval.ctype).isref=0; curdecltype=(yyval.ctype); }
#line 1608 "c.tab.c"
    break;

  case 23: /* type_spec: KW_STRUCT IDENT  */
#line 246 "c.y"
                                         { (yyval.ctype).base=T_STRUCT; (yyval.ctype).sdef=find_struct((yyvsp[0].sval)); (yyval.ctype).array=0; (yyval.ctype).arraylen=0; (yyval.ctype).isref=0;
	                                    if(!(yyval.ctype).sdef) semerror("struct '%s' nao definido", (yyvsp[0].sval)); curdecltype=(yyval.ctype); }
#line 1615 "c.tab.c"
    break;

  case 27: /* global_declarator: IDENT  */
#line 260 "c.y"
                                         { declare_global(curdecltype, (yyvsp[0].sval)); }
#line 1621 "c.tab.c"
    break;

  case 28: /* global_declarator: IDENT '[' NUMBER ']'  */
#line 261 "c.y"
                                         { declare_global_array(curdecltype, (yyvsp[-3].sval), (yyvsp[-1].ival)); }
#line 1627 "c.tab.c"
    break;

  case 29: /* global_declarator: IDENT '=' NUMBER  */
#line 262 "c.y"
                                         { declare_global_init(curdecltype, (yyvsp[-2].sval), (yyvsp[0].ival)); }
#line 1633 "c.tab.c"
    break;

  case 30: /* global_declarator: IDENT '=' FLOATNUM  */
#line 263 "c.y"
                                         { declare_global_init_float(curdecltype, (yyvsp[-2].sval), (yyvsp[0].fval)); }
#line 1639 "c.tab.c"
    break;

  case 31: /* func_proto: type_spec IDENT '(' param_list_opt ')'  */
#line 267 "c.y"
                                                  { declare_func((yyvsp[-4].ctype), (yyvsp[-3].sval), (yyvsp[-1].plist), 0); }
#line 1645 "c.tab.c"
    break;

  case 32: /* $@2: %empty  */
#line 271 "c.y"
                                                     { start_func((yyvsp[-5].ctype), (yyvsp[-4].sval), (yyvsp[-2].plist)); }
#line 1651 "c.tab.c"
    break;

  case 33: /* func_def: type_spec IDENT '(' param_list_opt ')' '{' $@2 block_item_list '}'  */
#line 272 "c.y"
                               { end_func(); }
#line 1657 "c.tab.c"
    break;

  case 34: /* param_list_opt: %empty  */
#line 276 "c.y"
                                         { (yyval.plist) = NULL; }
#line 1663 "c.tab.c"
    break;

  case 35: /* param_list_opt: KW_VOID  */
#line 277 "c.y"
                                         { (yyval.plist) = NULL; }
#line 1669 "c.tab.c"
    break;

  case 36: /* param_list_opt: param_list  */
#line 278 "c.y"
                                         { (yyval.plist) = (yyvsp[0].plist); }
#line 1675 "c.tab.c"
    break;

  case 37: /* param_list: param  */
#line 282 "c.y"
                                         { (yyval.plist) = (yyvsp[0].plist); }
#line 1681 "c.tab.c"
    break;

  case 38: /* param_list: param_list ',' param  */
#line 283 "c.y"
                                         { (yyval.plist) = append_param((yyvsp[-2].plist), (yyvsp[0].plist)); }
#line 1687 "c.tab.c"
    break;

  case 39: /* param: type_spec IDENT  */
#line 287 "c.y"
                                         { ctype_t t=(yyvsp[-1].ctype); if(t.base==T_STRUCT) t.isref=1; (yyval.plist) = mkparam(t, (yyvsp[0].sval)); }
#line 1693 "c.tab.c"
    break;

  case 40: /* param: type_spec IDENT '[' ']'  */
#line 288 "c.y"
                                         { ctype_t t=(yyvsp[-3].ctype); t.array=1; t.isref=1; (yyval.plist) = mkparam(t, (yyvsp[-2].sval)); }
#line 1699 "c.tab.c"
    break;

  case 48: /* local_declarator: IDENT  */
#line 311 "c.y"
                                         { declare_local(curdecltype, (yyvsp[0].sval)); }
#line 1705 "c.tab.c"
    break;

  case 49: /* local_declarator: IDENT '[' NUMBER ']'  */
#line 312 "c.y"
                                         { declare_local_array(curdecltype, (yyvsp[-3].sval), (yyvsp[-1].ival)); }
#line 1711 "c.tab.c"
    break;

  case 50: /* local_declarator: IDENT '=' assignment_expr  */
#line 313 "c.y"
                                         { symbol_t *s = declare_local(curdecltype, (yyvsp[-2].sval)); gen_rvalue(&(yyvsp[0].expr)); gen_store_symbol(s); }
#line 1717 "c.tab.c"
    break;

  case 53: /* statement: assignment_expr ';'  */
#line 319 "c.y"
                                         { gen_rvalue(&(yyvsp[-1].expr)); }
#line 1723 "c.tab.c"
    break;

  case 54: /* statement: if_head statement  */
#line 321 "c.y"
            { emitc("%s:", (yyvsp[-1].sval)); }
#line 1729 "c.tab.c"
    break;

  case 55: /* @3: %empty  */
#line 323 "c.y"
            { char *le=newlabel(); emitc("\tjmp %s", le); emitc("%s:", (yyvsp[-2].sval)); (yyval.sval)=le; }
#line 1735 "c.tab.c"
    break;

  case 56: /* statement: if_head statement KW_ELSE @3 statement  */
#line 325 "c.y"
            { emitc("%s:", (yyvsp[-1].sval)); }
#line 1741 "c.tab.c"
    break;

  case 57: /* @4: %empty  */
#line 327 "c.y"
            { char *l=newlabel(); emitc("%s:", l); (yyval.sval)=l; }
#line 1747 "c.tab.c"
    break;

  case 58: /* @5: %empty  */
#line 329 "c.y"
            { gen_rvalue(&(yyvsp[-1].expr)); char *le=newlabel(); emitc("\tcmp ax,0"); emitc("\tjz %s", le); push_loop((yyvsp[-2].sval), le); (yyval.sval)=le; }
#line 1753 "c.tab.c"
    break;

  case 59: /* statement: KW_WHILE '(' @4 assignment_expr ')' @5 statement  */
#line 331 "c.y"
            { emitc("\tjmp %s", (yyvsp[-4].sval)); emitc("%s:", (yyvsp[-1].sval)); pop_loop(); }
#line 1759 "c.tab.c"
    break;

  case 60: /* @6: %empty  */
#line 333 "c.y"
            { char *l=newlabel(); emitc("%s:", l); (yyval.sval)=l; }
#line 1765 "c.tab.c"
    break;

  case 61: /* @7: %empty  */
#line 335 "c.y"
            { char *le=newlabel(); char *lc=newlabel();
	      if((yyvsp[-1].count)) { emitc("\tcmp ax,0"); emitc("\tjz %s", le); }
	      push_loop(lc, le);
	      begin_capture();
	      (yyval.sval)=le; }
#line 1775 "c.tab.c"
    break;

  case 62: /* @8: %empty  */
#line 341 "c.y"
            { (yyval.sval) = end_capture(); }
#line 1781 "c.tab.c"
    break;

  case 63: /* statement: KW_FOR '(' opt_expr ';' @6 opt_expr ';' @7 opt_expr @8 ')' statement  */
#line 343 "c.y"
            { emitc("%s:", loop_cont_top());
	      emitc("%s", (yyvsp[-2].sval));
	      emitc("\tjmp %s", (yyvsp[-7].sval));
	      emitc("%s:", (yyvsp[-4].sval));
	      pop_loop(); }
#line 1791 "c.tab.c"
    break;

  case 64: /* statement: KW_RETURN ';'  */
#line 348 "c.y"
                                         { gen_return_void(); }
#line 1797 "c.tab.c"
    break;

  case 65: /* statement: KW_RETURN assignment_expr ';'  */
#line 349 "c.y"
                                         { gen_rvalue(&(yyvsp[-1].expr)); gen_return_value(); }
#line 1803 "c.tab.c"
    break;

  case 66: /* statement: KW_BREAK ';'  */
#line 350 "c.y"
                                         { gen_break(); }
#line 1809 "c.tab.c"
    break;

  case 67: /* statement: KW_CONTINUE ';'  */
#line 351 "c.y"
                                         { gen_continue(); }
#line 1815 "c.tab.c"
    break;

  case 68: /* opt_expr: %empty  */
#line 355 "c.y"
                                         { (yyval.count) = 0; }
#line 1821 "c.tab.c"
    break;

  case 69: /* opt_expr: assignment_expr  */
#line 356 "c.y"
                                         { gen_rvalue(&(yyvsp[0].expr)); (yyval.count) = 1; }
#line 1827 "c.tab.c"
    break;

  case 70: /* if_head: KW_IF '(' assignment_expr ')'  */
#line 361 "c.y"
            { gen_rvalue(&(yyvsp[-1].expr)); char *l=newlabel(); emitc("\tcmp ax,0"); emitc("\tjz %s", l); (yyval.sval)=l; }
#line 1833 "c.tab.c"
    break;

  case 71: /* assignment_expr: postfix_expr '=' assignment_expr  */
#line 365 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "="); }
#line 1839 "c.tab.c"
    break;

  case 72: /* assignment_expr: postfix_expr PLUSEQ assignment_expr  */
#line 366 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "add"); }
#line 1845 "c.tab.c"
    break;

  case 73: /* assignment_expr: postfix_expr MINUSEQ assignment_expr  */
#line 367 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "sub"); }
#line 1851 "c.tab.c"
    break;

  case 74: /* assignment_expr: postfix_expr STAREQ assignment_expr  */
#line 368 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "mul"); }
#line 1857 "c.tab.c"
    break;

  case 75: /* assignment_expr: postfix_expr SLASHEQ assignment_expr  */
#line 369 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "/"); }
#line 1863 "c.tab.c"
    break;

  case 76: /* assignment_expr: postfix_expr PERCENTEQ assignment_expr  */
#line 370 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "%"); }
#line 1869 "c.tab.c"
    break;

  case 77: /* assignment_expr: postfix_expr ANDEQ assignment_expr  */
#line 371 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "and"); }
#line 1875 "c.tab.c"
    break;

  case 78: /* assignment_expr: postfix_expr OREQ assignment_expr  */
#line 372 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "or"); }
#line 1881 "c.tab.c"
    break;

  case 79: /* assignment_expr: postfix_expr XOREQ assignment_expr  */
#line 373 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "xor"); }
#line 1887 "c.tab.c"
    break;

  case 80: /* assignment_expr: postfix_expr SHLEQ assignment_expr  */
#line 374 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "shl"); }
#line 1893 "c.tab.c"
    break;

  case 81: /* assignment_expr: postfix_expr SHREQ assignment_expr  */
#line 375 "c.y"
                                                  { (yyval.expr) = gen_assign(&(yyvsp[-2].expr), &(yyvsp[0].expr), "shr"); }
#line 1899 "c.tab.c"
    break;

  case 82: /* assignment_expr: expr  */
#line 376 "c.y"
                                                  { (yyval.expr) = (yyvsp[0].expr); }
#line 1905 "c.tab.c"
    break;

  case 83: /* $@9: %empty  */
#line 380 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 1911 "c.tab.c"
    break;

  case 84: /* expr: expr '+' $@9 expr  */
#line 380 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "add"); }
#line 1917 "c.tab.c"
    break;

  case 85: /* $@10: %empty  */
#line 381 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 1923 "c.tab.c"
    break;

  case 86: /* expr: expr '-' $@10 expr  */
#line 381 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "sub"); }
#line 1929 "c.tab.c"
    break;

  case 87: /* $@11: %empty  */
#line 382 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 1935 "c.tab.c"
    break;

  case 88: /* expr: expr '*' $@11 expr  */
#line 382 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "mul"); }
#line 1941 "c.tab.c"
    break;

  case 89: /* $@12: %empty  */
#line 383 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 1947 "c.tab.c"
    break;

  case 90: /* expr: expr '/' $@12 expr  */
#line 383 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "div"); }
#line 1953 "c.tab.c"
    break;

  case 91: /* $@13: %empty  */
#line 384 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 1959 "c.tab.c"
    break;

  case 92: /* expr: expr '%' $@13 expr  */
#line 384 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "mod"); }
#line 1965 "c.tab.c"
    break;

  case 93: /* $@14: %empty  */
#line 385 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 1971 "c.tab.c"
    break;

  case 94: /* expr: expr '&' $@14 expr  */
#line 385 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "and"); }
#line 1977 "c.tab.c"
    break;

  case 95: /* $@15: %empty  */
#line 386 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 1983 "c.tab.c"
    break;

  case 96: /* expr: expr '|' $@15 expr  */
#line 386 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "or"); }
#line 1989 "c.tab.c"
    break;

  case 97: /* $@16: %empty  */
#line 387 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 1995 "c.tab.c"
    break;

  case 98: /* expr: expr '^' $@16 expr  */
#line 387 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "xor"); }
#line 2001 "c.tab.c"
    break;

  case 99: /* $@17: %empty  */
#line 388 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 2007 "c.tab.c"
    break;

  case 100: /* expr: expr SHL $@17 expr  */
#line 388 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "shl"); }
#line 2013 "c.tab.c"
    break;

  case 101: /* $@18: %empty  */
#line 389 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 2019 "c.tab.c"
    break;

  case 102: /* expr: expr SHR $@18 expr  */
#line 389 "c.y"
                                                          { (yyval.expr) = gen_binop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "shr"); }
#line 2025 "c.tab.c"
    break;

  case 103: /* $@19: %empty  */
#line 390 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 2031 "c.tab.c"
    break;

  case 104: /* expr: expr '<' $@19 expr  */
#line 390 "c.y"
                                                          { (yyval.expr) = gen_relop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "jlt"); }
#line 2037 "c.tab.c"
    break;

  case 105: /* $@20: %empty  */
#line 391 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 2043 "c.tab.c"
    break;

  case 106: /* expr: expr '>' $@20 expr  */
#line 391 "c.y"
                                                          { (yyval.expr) = gen_relop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "jgt"); }
#line 2049 "c.tab.c"
    break;

  case 107: /* $@21: %empty  */
#line 392 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 2055 "c.tab.c"
    break;

  case 108: /* expr: expr LE $@21 expr  */
#line 392 "c.y"
                                                          { (yyval.expr) = gen_relop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "jle"); }
#line 2061 "c.tab.c"
    break;

  case 109: /* $@22: %empty  */
#line 393 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 2067 "c.tab.c"
    break;

  case 110: /* expr: expr GE $@22 expr  */
#line 393 "c.y"
                                                          { (yyval.expr) = gen_relop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "jge"); }
#line 2073 "c.tab.c"
    break;

  case 111: /* $@23: %empty  */
#line 394 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 2079 "c.tab.c"
    break;

  case 112: /* expr: expr EQ $@23 expr  */
#line 394 "c.y"
                                                          { (yyval.expr) = gen_relop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "jz"); }
#line 2085 "c.tab.c"
    break;

  case 113: /* $@24: %empty  */
#line 395 "c.y"
                   { push_acc(&(yyvsp[-1].expr).type); }
#line 2091 "c.tab.c"
    break;

  case 114: /* expr: expr NE $@24 expr  */
#line 395 "c.y"
                                                          { (yyval.expr) = gen_relop(&(yyvsp[-3].expr), &(yyvsp[0].expr), "jnz"); }
#line 2097 "c.tab.c"
    break;

  case 115: /* $@25: %empty  */
#line 396 "c.y"
                      { push_acc(&(yyvsp[-1].expr).type); }
#line 2103 "c.tab.c"
    break;

  case 116: /* expr: expr ANDAND $@25 expr  */
#line 396 "c.y"
                                                          { (yyval.expr) = gen_logic(&(yyvsp[-3].expr), &(yyvsp[0].expr), "and"); }
#line 2109 "c.tab.c"
    break;

  case 117: /* $@26: %empty  */
#line 397 "c.y"
                    { push_acc(&(yyvsp[-1].expr).type); }
#line 2115 "c.tab.c"
    break;

  case 118: /* expr: expr OROR $@26 expr  */
#line 397 "c.y"
                                                          { (yyval.expr) = gen_logic(&(yyvsp[-3].expr), &(yyvsp[0].expr), "or"); }
#line 2121 "c.tab.c"
    break;

  case 119: /* expr: '-' expr  */
#line 398 "c.y"
                                                         { (yyval.expr) = gen_neg(&(yyvsp[0].expr)); }
#line 2127 "c.tab.c"
    break;

  case 120: /* expr: '!' expr  */
#line 399 "c.y"
                                                         { (yyval.expr) = gen_not(&(yyvsp[0].expr)); }
#line 2133 "c.tab.c"
    break;

  case 121: /* expr: '~' expr  */
#line 400 "c.y"
                                                         { (yyval.expr) = gen_bitnot(&(yyvsp[0].expr)); }
#line 2139 "c.tab.c"
    break;

  case 122: /* expr: INCOP postfix_expr  */
#line 401 "c.y"
                                                         { (yyval.expr) = gen_assign(&(yyvsp[0].expr), NULL, "preinc"); }
#line 2145 "c.tab.c"
    break;

  case 123: /* expr: DECOP postfix_expr  */
#line 402 "c.y"
                                                         { (yyval.expr) = gen_assign(&(yyvsp[0].expr), NULL, "predec"); }
#line 2151 "c.tab.c"
    break;

  case 124: /* expr: postfix_expr  */
#line 403 "c.y"
                                                         { gen_rvalue(&(yyvsp[0].expr)); (yyval.expr) = (yyvsp[0].expr); }
#line 2157 "c.tab.c"
    break;

  case 126: /* postfix_expr: postfix_expr '[' expr ']'  */
#line 408 "c.y"
                                         { (yyval.expr) = gen_index(&(yyvsp[-3].expr), &(yyvsp[-1].expr)); }
#line 2163 "c.tab.c"
    break;

  case 127: /* postfix_expr: postfix_expr '.' IDENT  */
#line 409 "c.y"
                                         { (yyval.expr) = gen_field(&(yyvsp[-2].expr), (yyvsp[0].sval)); }
#line 2169 "c.tab.c"
    break;

  case 128: /* postfix_expr: IDENT '(' call_args_opt ')'  */
#line 410 "c.y"
                                         { (yyval.expr) = gen_call((yyvsp[-3].sval), (yyvsp[-1].count)); }
#line 2175 "c.tab.c"
    break;

  case 129: /* call_args_opt: %empty  */
#line 414 "c.y"
                                         { (yyval.count) = 0; }
#line 2181 "c.tab.c"
    break;

  case 130: /* call_args_opt: call_args  */
#line 415 "c.y"
                                         { (yyval.count) = (yyvsp[0].count); }
#line 2187 "c.tab.c"
    break;

  case 131: /* call_args: assignment_expr  */
#line 419 "c.y"
                                         { gen_rvalue(&(yyvsp[0].expr)); push_acc(&(yyvsp[0].expr).type); (yyval.count) = 1; }
#line 2193 "c.tab.c"
    break;

  case 132: /* call_args: call_args ',' assignment_expr  */
#line 420 "c.y"
                                         { gen_rvalue(&(yyvsp[0].expr)); push_acc(&(yyvsp[0].expr).type); (yyval.count) = (yyvsp[-2].count) + 1; }
#line 2199 "c.tab.c"
    break;

  case 133: /* primary_expr: IDENT  */
#line 424 "c.y"
                                         { (yyval.expr) = gen_ident((yyvsp[0].sval)); }
#line 2205 "c.tab.c"
    break;

  case 134: /* primary_expr: NUMBER  */
#line 425 "c.y"
                                         { (yyval.expr) = gen_intlit((yyvsp[0].ival)); }
#line 2211 "c.tab.c"
    break;

  case 135: /* primary_expr: CHARLIT  */
#line 426 "c.y"
                                         { (yyval.expr) = gen_charlit((yyvsp[0].ival)); }
#line 2217 "c.tab.c"
    break;

  case 136: /* primary_expr: FLOATNUM  */
#line 427 "c.y"
                                         { (yyval.expr) = gen_floatlit((yyvsp[0].fval)); }
#line 2223 "c.tab.c"
    break;

  case 137: /* primary_expr: STRINGLIT  */
#line 428 "c.y"
                                         { (yyval.expr) = gen_stringlit((yyvsp[0].sval)); }
#line 2229 "c.tab.c"
    break;

  case 138: /* primary_expr: '(' assignment_expr ')'  */
#line 429 "c.y"
                                         { (yyval.expr) = (yyvsp[-1].expr); }
#line 2235 "c.tab.c"
    break;


#line 2239 "c.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 432 "c.y"

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
		emitc("\tsys 5,bx");
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
		emitc("\tsys 6,bx");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "outputpin") == 0) {
		if(argcount != 1) semerror("outputpin espera 1 argumento (pin)");
		emitc("\tpop bx");
		emitc("\tsys 7,bx");
		result.kind = 0;
		result.type.base = T_VOID; result.type.sdef = NULL; result.type.array = 0; result.type.arraylen = 0; result.type.isref = 0;
		return result;
	}
	if(strcmp(name, "pulluppin") == 0) {
		if(argcount != 1) semerror("pulluppin espera 1 argumento (pin)");
		emitc("\tpop bx");
		emitc("\tsys 8,bx");
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
			inname = argv[i];
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
