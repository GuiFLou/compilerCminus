/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the C‑ compiler  */
/* (chained hash table)                             */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* ===================== CONFIG ===================== */
#define SIZE   211   /* hash table size             */
#define SHIFT    4   /* left‑shift in hash function */

/* ==================== Data Types ================== */

typedef struct LineListRec {
    int lineno;
    struct LineListRec *next;
} *LineList;

typedef struct BucketListRec {
    char *name;
    LineList lines;
    int  memloc;      /* mem address */
    char *scope;
    char *typeID;
    char *typeData;
    struct BucketListRec *next;
} *BucketList;

/* ================= Internal State ================= */
static BucketList hashTable[SIZE];
static int location = 0;  /* next memory location */

/* ================= Hash Function ================== */
static int hash(const char *name, const char *scope) {
    unsigned int temp = 0;
    for (const char *p = name; *p; ++p)
        temp = ((temp << SHIFT) + *p) % SIZE;
    for (const char *p = scope; *p; ++p)
        temp = ((temp << SHIFT) + *p) % SIZE;
    return (int)temp;
}

/* =============== Built‑in Declarations ============ */
/* These functions are always inserted into global scope ("") */
static void insertBuiltIns(void) {
    /* int input(void)  */
    st_insert("input", 0, location++, "global", "function", "integer");
    /* void output(int) */
    st_insert("output",0, location++, "global", "function", "void(int)");
}

/* ================ Public Interface ================ */

void symtabInit(void) {
    memset(hashTable, 0, sizeof(hashTable));
    insertBuiltIns();
}

void st_insert(char *name, int lineno, int loc,
               char *scope, char *typeID, char *typeData) {
    int h = hash(name, scope);
    BucketList l = hashTable[h];
    while (l && (strcmp(name, l->name) || strcmp(scope, l->scope)))
        l = l->next;

    if (!l) { /* first time */
        l = (BucketList)malloc(sizeof *l);
        l->name  = name;
        l->scope = scope;
        l->typeID = typeID;
        l->typeData = typeData;
        l->memloc = loc;
        l->lines = (LineList)malloc(sizeof(struct LineListRec));
        l->lines->lineno = lineno;
        l->lines->next = NULL;
        l->next = hashTable[h];
        hashTable[h] = l;
    } else {  /* already present – append new line */
        LineList t = l->lines;
        while (t->next) t = t->next;
        t->next = (LineList)malloc(sizeof *t);
        t->next->lineno = lineno;
        t->next->next = NULL;
    }
}

int st_lookup(char *name, char *scope) {
    int h = hash(name, scope);
    BucketList l = hashTable[h];
    while (l && (strcmp(name, l->name) || strcmp(scope, l->scope)))
        l = l->next;
    return l ? l->memloc : -1;
}

char *st_lookup_type(char *name, char *scope) {
    int h = hash(name, scope);
    BucketList l = hashTable[h];
    while (l && (strcmp(name, l->name) || strcmp(scope, l->scope)))
        l = l->next;
    return l ? l->typeData : NULL;
}

char *st_lookup_typeid(char *name, char *scope) {
    int h = hash(name, scope);
    BucketList l = hashTable[h];
    while (l && (strcmp(name, l->name) || strcmp(scope, l->scope)))
        l = l->next;
    return l ? l->typeID : NULL;
}

void printSymTab(FILE *listing) {
    fprintf(listing,
            "Location    Name       Scope            TypeID         TypeData     Line Numbers\n");
    fprintf(listing,
            "---------  ------     -------           -------        --------     ------------\n");
    for (int i = 0; i < SIZE; ++i) {
        if (!hashTable[i]) continue;
        for (BucketList l = hashTable[i]; l; l = l->next) {
            fprintf(listing, "%-8d  %-14s %-14s  %-14s  %-14s  ",
                    l->memloc, l->name, l->scope, l->typeID, l->typeData);
            for (LineList t = l->lines; t; t = t->next)
                fprintf(listing, "%4d ", t->lineno);
            fputc('\n', listing);
        }
    }
}