#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#ifndef YYPARSER

#include "Parser.tab.h"

#endif


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 6

extern FILE* source; /* source code text file */
extern FILE* listing; /* listing output text file */
extern FILE* code; /* code text file for TM simulator */

typedef int TokenType;

extern int lineno; /* source line number for listing */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum 
{
    StmtK, ExpK
} NodeKind;

typedef enum
{
    IfK, whileK, AssignK, variableK, functionK, callK, returnK, numberK
} StmtKind;

typedef enum 
{
    OpK, ConstK, IdK, vectorK, vectorIdK, typeK
} ExpKind;

/* ExpType is used for type checking */
typedef enum 
{
    voidK, integerK, booleanK
} ExpType;

#define MAXCHILDREN 3


typedef struct treeNode
   { 
    struct treeNode * child[MAXCHILDREN];
    struct treeNode * sibling;
    int lineno;

    NodeKind nodekind;

    union 
    { 
        StmtKind stmt; 
        ExpKind exp;
    } kind;

    struct 
    { 
        TokenType op;
        int val;
        int len;
        char* name; 
        char* scope;
    } attr;

    ExpType type; /* for type checking of exps */
   } TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

extern int EchoSource;

extern int TraceScan;

extern int TraceParse;

extern int TraceAnalyze;

extern int TraceCode;

extern int Error; 
#endif
