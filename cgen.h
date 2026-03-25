#ifndef _CGEN_H_
#define _CGEN_H_

#include "globals.h"

/* --------------------------------------------------
   Estrutura da quádrupla (op, arg1, arg2, result)
   -------------------------------------------------- */
typedef struct QuadrupleRec {
    char op[10];
    char arg1[32];
    char arg2[32];
    char result[32];
    struct QuadrupleRec *next;
} Quadruple;

/* lista global de quádruplas */
extern Quadruple *quadList;

/* Gera quádruplas + asm + .mem/.txt                        */
void codeGen(TreeNode *syntaxTree, const char *baseName);

/* Emite quádrupla                                          */
void emit(const char *op,
          const char *arg1,
          const char *arg2,
          const char *result);

/* Utilidades                                               */
char *newTemp(void);
char *newLabel(void);
void  printQuads(FILE *out);

#endif