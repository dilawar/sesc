/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YYCONF_HOME_DILAWARS_WORK_GITHUB_DILAWAR_SESC_OBJ_LINUX_OBJ_CONFLEX_TAB_HPP_INCLUDED
# define YY_YYCONF_HOME_DILAWARS_WORK_GITHUB_DILAWAR_SESC_OBJ_LINUX_OBJ_CONFLEX_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yyConfdebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    CFDOUBLE = 258,
    CFLONG = 259,
    CFBOOL = 260,
    CFQSTRING = 261,
    CFNAME = 262,
    CFNAMEREF = 263,
    CFSTRNAMEREF = 264,
    CFOB = 265,
    CFCB = 266,
    CFEQUAL = 267,
    CFDDOT = 268,
    CFOP = 269,
    CFCP = 270,
    CFPLUS = 271,
    CFMINUS = 272,
    CFMULT = 273,
    CFDIV = 274,
    CFDIVC = 275,
    CFEXP = 276,
    LE = 277,
    LT = 278,
    GE = 279,
    GT = 280,
    TRICASE = 281
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 31 "/home/dilawars/Work/GITHUB/DILAWAR/sesc/./src/libsuc/conflex.y" /* yacc.c:1921  */

  bool Bool;
  int  Int;
  double Double;
  char *CharPtr;

#line 92 "/home/dilawars/Work/GITHUB/DILAWAR/sesc/obj/Linux_obj/conflex.tab.hpp" /* yacc.c:1921  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yyConflval;

int yyConfparse (void);

#endif /* !YY_YYCONF_HOME_DILAWARS_WORK_GITHUB_DILAWAR_SESC_OBJ_LINUX_OBJ_CONFLEX_TAB_HPP_INCLUDED  */
