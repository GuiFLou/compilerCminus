/* cgen.c — geração de quádruplas, assembly e binário
 * Varre TODAS as funções (siblings) e emite ALLOC para variáveis. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "cgen.h"
#include "asmgen.h"
#include "encoder.h"
#include "symtab.h"

/* ---------- lista de quadruplas ---------- */
Quadruple *quadList = NULL;
static Quadruple *lastQuad = NULL;

void emit(const char *op,
          const char *arg1,
          const char *arg2,
          const char *result)
{
    Quadruple *q = malloc(sizeof *q);
    if (!q) { perror("emit"); exit(EXIT_FAILURE); }

    strcpy(q->op,     op     ? op     : "");
    strcpy(q->arg1,   arg1   ? arg1   : "");
    strcpy(q->arg2,   arg2   ? arg2   : "");
    strcpy(q->result, result ? result : "");
    q->next = NULL;

    if (!quadList) quadList = q; else lastQuad->next = q;
    lastQuad = q;
}

/* ---------- temporarios e labels ---------- */
static int tempCount = 0;
static int labelCount = 0;

char *newTemp(void)
{
    static char buf[32];
    sprintf(buf, "t%d", tempCount++);
    return strdup(buf);
}

char *newLabel(void)
{
    static char buf[32];
    sprintf(buf, "L%d", labelCount++);
    return strdup(buf);
}

/* ---------- prototipos internos ---------- */
static char *genExp(TreeNode *t);
static void genStmt(TreeNode *t);

static int isDirectVector(const char *name, const char *scope)
{
    char *tid = st_lookup_typeid((char *)name, (char *)(scope ? scope : "global"));
    if (tid && !strcmp(tid, "vector")) return 1;

    tid = st_lookup_typeid((char *)name, "global");
    return tid && !strcmp(tid, "vector");
}

static char *genCallArg(TreeNode *t)
{
    if (t && t->nodekind == ExpK && t->kind.exp == IdK &&
        isDirectVector(t->attr.name, t->attr.scope)) {
        char *dst = newTemp();
        emit("ADDR", t->attr.name, "-", dst);
        return dst;
    }

    return genExp(t);
}

/* ---------- expressoes ---------- */
static char *genExp(TreeNode *t)
{
    if (!t) return "";

    if (t->nodekind == StmtK && t->kind.stmt == callK) {
        if (!strcmp(t->attr.name, "input")) {
            char *dst = newTemp();
            emit("CALL_I", "-", "-", dst);
            return dst;
        }
        else {
            int cnt = 0;
            for (TreeNode *p = t->child[0]; p; p = p->sibling) {
                char *a = genCallArg(p);
                emit("PARAM", a, "-", "-");
                ++cnt;
            }
            char buf[12];
            sprintf(buf, "%d", cnt);
            char *dst = newTemp();
            emit("CALL", t->attr.name, buf, dst);
            return dst;
        }
    }

    switch (t->kind.exp) {
    case ConstK: {
        char lit[32];
        char *dst = newTemp();
        sprintf(lit, "%d", t->attr.val);
        emit("ASSIGN", lit, "-", dst);
        return dst;
    }

    case IdK: {
        char *dst = newTemp();
        emit("LOAD", t->attr.name, "-", dst);
        return dst;
    }

    case vectorK: {
        if (t->child[0] == NULL) {
            char *dst = newTemp();
            emit("LOAD", t->attr.name, "-", dst);
            return dst;
        }

        char *idx = genExp(t->child[0]);
        char *dst = newTemp();
        emit("LOADV", t->attr.name, idx, dst);
        return dst;
    }

    case OpK: {
        char *a = genExp(t->child[0]);
        char *b = genExp(t->child[1]);
        char *dst = newTemp();

        switch (t->attr.op) {
        case PLUS:
            emit("ADD", a, b, dst);
            break;
        case MINUS:
            emit("SUB", a, b, dst);
            break;
        case TIMES:
            emit("MUL", a, b, dst);
            break;
        case OVER:
            emit("DIV", a, b, dst);
            break;
        case EQ:
            emit("EQ", a, b, dst);
            break;
        case NE:
            emit("NEQ", a, b, dst);
            break;
        case LT:
            emit("SLT", a, b, dst);
            break;
        case GT:
            emit("SLT", b, a, dst);
            break;
        case LTE: {
            char *tmp = newTemp();
            char *one = newTemp();
            emit("SLT", b, a, tmp);
            emit("ASSIGN", "1", "-", one);
            emit("SUB", one, tmp, dst);
            break;
        }
        case GTE: {
            char *tmp = newTemp();
            char *one = newTemp();
            emit("SLT", a, b, tmp);
            emit("ASSIGN", "1", "-", one);
            emit("SUB", one, tmp, dst);
            break;
        }
        default:
            emit("NOOP", a, b, dst);
            break;
        }

        return dst;
    }

    default:
        return "";
    }
}

/* ---------- statements ---------- */
static void genStmt(TreeNode *t)
{
    for (; t; t = t->sibling) {
        if (t->nodekind == ExpK) {
            if (t->kind.exp == typeK && t->child[0])
                genStmt(t->child[0]);
            continue;
        }

        switch (t->kind.stmt) {
        case variableK: {
            const char *scope =
                (t->attr.scope && t->attr.scope[0] != '\0') ? t->attr.scope : "-";
            if (t->attr.len > 0) {
                char sizeBuf[16];
                sprintf(sizeBuf, "%d", t->attr.len);
                emit("ALLOC", t->attr.name, scope, sizeBuf);
            }
            else {
                emit("ALLOC", t->attr.name, scope, "-");
            }
            break;
        }

        case functionK: {
            emit("FUN",
                 (t->type == integerK) ? "int" :
                 (t->type == booleanK) ? "bool" : "void",
                 t->attr.name, "-");

            for (TreeNode *p = t->child[0]; p; p = p->sibling) {
                if (p->nodekind == ExpK &&
                    p->kind.exp == typeK &&
                    p->child[0] != NULL) {
                    TreeNode *param = p->child[0];
                    emit("ARG",
                         (p->type == integerK) ? "int" :
                         (p->type == booleanK) ? "bool" : "void",
                         param->attr.name,
                         t->attr.name);
                }
            }

            for (int i = 1; i < MAXCHILDREN; ++i)
                genStmt(t->child[i]);

            emit("END", t->attr.name, "-", "-");
            break;
        }

        case AssignK: {
            char *rhs = genExp(t->child[1]);
            TreeNode *lhs = t->child[0];

            if (lhs->kind.exp == vectorK && lhs->child[0] != NULL) {
                char *idx = genExp(lhs->child[0]);
                emit("STOREV", rhs, idx, lhs->attr.name);
            }
            else {
                emit("STORE", rhs, "-", lhs->attr.name);
            }
            break;
        }

        case returnK: {
            char *retv = t->child[0] ? genExp(t->child[0]) : "-";
            emit("RET", retv, "-", "-");
            break;
        }

        case callK: {
            if (!strcmp(t->attr.name, "input")) {
                char *dst = newTemp();
                emit("CALL_I", "-", "-", dst);
            }
            else if (!strcmp(t->attr.name, "output")) {
                char *arg0 = genExp(t->child[0]);
                emit("PARAM", arg0, "-", "-");
                emit("CALL_O", arg0, "-", "-");
            }
            else {
                int cnt = 0;
                for (TreeNode *p = t->child[0]; p; p = p->sibling) {
                    char *a = genCallArg(p);
                    emit("PARAM", a, "-", "-");
                    ++cnt;
                }
                char buf[12];
                sprintf(buf, "%d", cnt);
                emit("CALL", t->attr.name, buf, "-");
            }
            break;
        }

        case IfK: {
            char *lElse = newLabel();
            char *lEnd = newLabel();
            char *cond = genExp(t->child[0]);
            emit("IFF", cond, lElse, "-");
            genStmt(t->child[1]);
            emit("GOTO", lEnd, "-", "-");
            emit("LAB", lElse, "-", "-");
            if (t->child[2]) genStmt(t->child[2]);
            emit("LAB", lEnd, "-", "-");
            break;
        }

        case whileK: {
            char *lStart = newLabel();
            char *lExit = newLabel();
            emit("LAB", lStart, "-", "-");
            char *cond = genExp(t->child[0]);
            emit("IFF", cond, lExit, "-");
            genStmt(t->child[1]);
            emit("GOTO", lStart, "-", "-");
            emit("LAB", lExit, "-", "-");
            break;
        }

        default:
            break;
        }
    }
}

/* ---------- "main" code generation ---------- */
void codeGen(TreeNode *syntaxTree, const char *base)
{
    quadList = lastQuad = NULL;
    tempCount = 0;
    labelCount = 0;

    
    genStmt(syntaxTree);
    emit("HALT", "-", "-", "-");

    {
        char tm[256];
        char asmf[256];
        char memf[256];

        sprintf(tm, "%s.tm", base);
        sprintf(asmf, "%s.s", base);
        sprintf(memf, "%s.mem", base);

        FILE *ft = fopen(tm, "w");
        printQuads(ft);
        fclose(ft);

        asmGen(quadList, asmf);
        encodeAsm(asmf, memf);
    }
}

/* ---------- dump ---------- */
void printQuads(FILE *out)
{
    int n = 0;
    for (Quadruple *q = quadList; q; q = q->next)
        fprintf(out, "%3d: (%s, %s, %s, %s)\n",
                n++, q->op, q->arg1, q->arg2, q->result);
}
