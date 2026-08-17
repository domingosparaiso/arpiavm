/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_C_TAB_H_INCLUDED
# define YY_YY_C_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDENT = 258,                   /* IDENT  */
    STRINGLIT = 259,               /* STRINGLIT  */
    NUMBER = 260,                  /* NUMBER  */
    CHARLIT = 261,                 /* CHARLIT  */
    FLOATNUM = 262,                /* FLOATNUM  */
    KW_CHAR = 263,                 /* KW_CHAR  */
    KW_INT = 264,                  /* KW_INT  */
    KW_LONG = 265,                 /* KW_LONG  */
    KW_FLOAT = 266,                /* KW_FLOAT  */
    KW_VOID = 267,                 /* KW_VOID  */
    KW_STRUCT = 268,               /* KW_STRUCT  */
    KW_IF = 269,                   /* KW_IF  */
    KW_ELSE = 270,                 /* KW_ELSE  */
    KW_WHILE = 271,                /* KW_WHILE  */
    KW_FOR = 272,                  /* KW_FOR  */
    KW_RETURN = 273,               /* KW_RETURN  */
    KW_BREAK = 274,                /* KW_BREAK  */
    KW_CONTINUE = 275,             /* KW_CONTINUE  */
    SHLEQ = 276,                   /* SHLEQ  */
    SHREQ = 277,                   /* SHREQ  */
    PLUSEQ = 278,                  /* PLUSEQ  */
    MINUSEQ = 279,                 /* MINUSEQ  */
    STAREQ = 280,                  /* STAREQ  */
    SLASHEQ = 281,                 /* SLASHEQ  */
    PERCENTEQ = 282,               /* PERCENTEQ  */
    ANDEQ = 283,                   /* ANDEQ  */
    OREQ = 284,                    /* OREQ  */
    XOREQ = 285,                   /* XOREQ  */
    EQ = 286,                      /* EQ  */
    NE = 287,                      /* NE  */
    LE = 288,                      /* LE  */
    GE = 289,                      /* GE  */
    ANDAND = 290,                  /* ANDAND  */
    OROR = 291,                    /* OROR  */
    SHL = 292,                     /* SHL  */
    SHR = 293,                     /* SHR  */
    INCOP = 294,                   /* INCOP  */
    DECOP = 295,                   /* DECOP  */
    LOWER_THAN_ELSE = 296,         /* LOWER_THAN_ELSE  */
    UMINUS = 297,                  /* UMINUS  */
    UNOT = 298,                    /* UNOT  */
    UBITNOT = 299                  /* UBITNOT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 160 "c.y"

	int ival;
	double fval;
	char *sval;
	ctype_t ctype;
	expr_t expr;
	paramnode_t *plist;
	int count;

#line 118 "c.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_C_TAB_H_INCLUDED  */
