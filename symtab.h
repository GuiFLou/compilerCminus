#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include <stdio.h>

/* --------------------------------------------------
   Inserção e consulta
   -------------------------------------------------- */
void st_insert(char *name, int lineno, int loc,
               char *scope, char *typeID, char *typeData);

int  st_lookup(char *name, char *scope);
char *st_lookup_type(char *name, char *scope);
char *st_lookup_typeid(char *name, char *scope);

/* --------------------------------------------------
   Inicialização (zera hash e injeta built‑ins)
   -------------------------------------------------- */
void symtabInit(void);

/* --------------------------------------------------
   Depuração: imprime a tabela formatada
   -------------------------------------------------- */
void printSymTab(FILE *listing);

#endif