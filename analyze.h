#ifndef _ANALYZE_H_
#define _ANALYZE_H_
#include "globals.h"

/* A função buildSymtab constrói a tabela de símbolos pela passagem de pré-ordem da árvore sintática */
void buildSymtab(TreeNode *);

/* O procedimento typeCheck executa a verificação de tipo pela passagem da árvore sintática de pós-ordem */
void typeCheck(TreeNode *);

#endif