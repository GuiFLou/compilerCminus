/* analyze.c – analisador semântico com suporte a built‑ins input()/output() */

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/******************* utilidades ************************/ 
static void typeError(TreeNode *t, char *message)
{
    fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
    Error = TRUE;
}

static ExpType typeDataToExpType(const char *typeData)
{
    if (!typeData) return integerK;
    if (strncmp(typeData, "void", 4) == 0) return voidK;
    if (strncmp(typeData, "integer", 7) == 0) return integerK;
    if (strncmp(typeData, "boolean", 7) == 0) return booleanK;
    return integerK;
}

static const char *expTypeName(ExpType type)
{
    switch (type) {
    case voidK:
        return "void";
    case booleanK:
        return "boolean";
    case integerK:
    default:
        return "integer";
    }
}

static const char *lookupDeclaredType(TreeNode *t)
{
    const char *declType = st_lookup_type(t->attr.name, t->attr.scope);
    if (!declType)
        declType = st_lookup_type(t->attr.name, "global");
    return declType;
}

/* contador de endereços de variável */
static int location = 0;

/******************* travessia genérica ****************/ 
static void traverse(TreeNode *t,
                     void (*preProc)(TreeNode *),
                     void (*postProc)(TreeNode *))
{
    if (!t) return;
    preProc(t);
    for (int i = 0; i < MAXCHILDREN; ++i)
        traverse(t->child[i], preProc, postProc);
    postProc(t);
    traverse(t->sibling, preProc, postProc);
}

static void nullProc(TreeNode *t) { (void)t; }

/******************* inserção na ST *******************/
static void insertNode(TreeNode *t)
{
    switch (t->nodekind) {
    case StmtK:
        switch (t->kind.stmt) {
        case variableK:
            if (st_lookup(t->attr.name, t->attr.scope) == -1 &&
                st_lookup(t->attr.name, "global") == -1) {
                if (t->attr.len > 0) {
                    st_insert(t->attr.name, t->lineno, location, t->attr.scope,
                              "vector", "integer");
                    location += t->attr.len;
                } else {
                    st_insert(t->attr.name, t->lineno, location++, t->attr.scope,
                              "variable", "integer");
                }
            } else
                typeError(t, "Error 4: Invalid Declaration. Already declared.");
            break;
        case functionK:
            if (st_lookup(t->attr.name, t->attr.scope) == -1 &&
                st_lookup(t->attr.name, "global") == -1)
                st_insert(t->attr.name, t->lineno, location++, t->attr.scope,
                           "function",
                           (t->type == integerK) ? "integer" : "void");
            else
                typeError(t, "Error 4: Invalid Declaration. Already declared.");
            break;
        case callK:
            /* built‑ins input/output já estão na ST global */
            if (st_lookup(t->attr.name, t->attr.scope) == -1 &&
                st_lookup(t->attr.name, "global") == -1)
                typeError(t, "Error 5: Invalid Call. It was not declared.");
            else {
                const char *declType = lookupDeclaredType(t);
                t->type = typeDataToExpType(declType);
                /* apenas anota a linha */
                st_insert(t->attr.name, t->lineno, 0, t->attr.scope, "call",
                          (char *)expTypeName(t->type));
            }
            break;
        case returnK:
            break;
        default:
            break;
        }
        break;
    case ExpK:
        switch (t->kind.exp) {
        case vectorK:
            if (st_lookup(t->attr.name, t->attr.scope) == -1 &&
                st_lookup(t->attr.name, "global") == -1)
                typeError(t, "Error 1: it was not declared");
            else
                st_insert(t->attr.name, t->lineno, 0, t->attr.scope, "vector",
                           "integer");
            break;
        case vectorIdK:
            if (st_lookup(t->attr.name, t->attr.scope) == -1 &&
                st_lookup(t->attr.name, "global") == -1)
                typeError(t, "Error 1: it was not declared");
            else
                st_insert(t->attr.name, t->lineno, 0, t->attr.scope,
                           "vector index", "integer");
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

/******************* construção da ST *****************/
void buildSymtab(TreeNode *syntaxTree)
{
    symtabInit(); /* built‑ins input/output + reset */

    traverse(syntaxTree, insertNode, nullProc);

    if (st_lookup("main", "global") == -1) {
        fprintf(listing, "Error: main was not declared\n");
        Error = TRUE;
    }

    if (TraceAnalyze) {
        fprintf(listing, "\nSymbol table:\n\n");
        printSymTab(listing);
    }
}

/******************* verificação de tipos *************/
static void checkNode(TreeNode *t)
{
    switch (t->nodekind) {
    case StmtK:
        switch (t->kind.stmt) {
        case variableK:
            if (st_lookup(t->attr.name, t->attr.scope) == -1 &&
                st_lookup(t->attr.name, "global") == -1)
                typeError(t, "Error 1: it was not declared");
            break;
        case IfK:
            if (t->child[0]->type == integerK && t->child[1]->type == integerK)
                typeError(t->child[0], "if test is not Boolean");
            break;
        case AssignK: {
            if (t->child[1] &&                     /* nó existe */
                t->child[1]->nodekind == StmtK &&
                t->child[1]->kind.stmt == callK) {

                const char *retType = lookupDeclaredType(t->child[1]);

                if (typeDataToExpType(retType) == voidK)
                    typeError(t->child[1], "assignment of void return");
            }
            break;
        }

        default:
            break;
        }
        break;
    default:
        break;
    }
}

void typeCheck(TreeNode *syntaxTree)
{
    traverse(syntaxTree, nullProc, checkNode);
}
