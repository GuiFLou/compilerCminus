/* asmgen.c — gera assembly para a ISA F1/F2/F3 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "cgen.h"
#include "asmgen.h"
#include "symtab.h"

/* ---------------- Registradores temporários ---------------- */
static const char *T[] = {
    "$t0","$t1","$t2","$t3","$t4",
    "$t5","$t6","$t7","$t8","$t9"
};
#define NT 10
#define MAX_PARAMS 16
static int nxtT = 0;
static int asmLabNum = 0;
static int currentArgIndex = 0;  /* índice do parâmetro atual dentro da função */
static int currentFunctionArgCount = 0;
static const char *currentFunction = "global";
static int currentFunctionEndReachable = 1;
static int mainExitEmitted = 0;
static const char *paramNames[MAX_PARAMS];
static int paramCount = 0;
/* Palavras empilhadas pelo callee desde a entrada (para offset de parâmetros em $sp) */
static int stackDelta = 0;
static const char *nextT(void) {
    const char *r = T[nxtT];
    nxtT = (nxtT + 1) % NT;
    return r;
}

/* Retorna um temporário que não seja avoid1 nem avoid2 (evita sobrescrever valor/índice em LOADV/STOREV) */
static const char *nextTExcept(const char *avoid1, const char *avoid2)
{
    const char *r;
    int n = NT;
    do {
        r = nextT();
        if ((!avoid1 || strcmp(r, avoid1) != 0) && (!avoid2 || strcmp(r, avoid2) != 0))
            return r;
        n--;
    } while (n > 0);
    return r;
}

/* mapeia "tN" → "$tK"  |  senão devolve o literal */
static const char *mapT(const char *name)
{
    if (name[0] != 't') return name;
    int id = atoi(name + 1);
    return T[id % NT];
}

static int isNumericLiteral(const char *s)
{
    if (!s || !*s) return 0;
    if (*s == '-' || *s == '+') ++s;
    if (!*s) return 0;
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        ++s;
    }
    return 1;
}

static int lookupMemloc(const char *name)
{
    int loc;

    if (!name || !*name) return -1;
    if (isNumericLiteral(name)) return atoi(name);

    loc = st_lookup((char *)name, (char *)currentFunction);
    if (loc >= 0) return loc;

    return st_lookup((char *)name, "global");
}

static void resolveMemOperand(const char *name, char *buf, size_t bufSize)
{
    int loc = lookupMemloc(name);
    if (loc >= 0) snprintf(buf, bufSize, "%d", loc);
    else snprintf(buf, bufSize, "%s", name);
}

/* Retorna índice do parâmetro (0..paramCount-1) ou -1 se não for parâmetro da função atual */
static int getParamIndex(const char *name)
{
    int i;
    if (!name || paramCount <= 0) return -1;
    for (i = 0; i < paramCount; i++)
        if (paramNames[i] && strcmp(paramNames[i], name) == 0)
            return i;
    return -1;
}

static int countFunctionArgs(const Quadruple *q)
{
    int count = 0;
    const Quadruple *qq = q;

    while (qq && strcmp(qq->op, "END")) {
        if (!strcmp(qq->op, "ARG"))
            count++;
        qq = qq->next;
    }

    return count;
}

/* ---------------- Pilha de argumentos ---------------- */
static int argBytes = 0;
static void pushArg(FILE *o, const char *r)
{
    if (!r || r[0] == '\0') return;

    fprintf(o,"    addi $sp,$sp,-1\n");
    fprintf(o,"    sw   %s,0($sp)\n", r);
    argBytes += 1;
    stackDelta += 1;
}

static void popArgCount(FILE *o, int count)
{
    if (count <= 0) return;
    if (count > argBytes) count = argBytes;
    if (count <= 0) return;

    stackDelta -= count;
    fprintf(o, "    addi $sp,$sp,%d\n", count);
    argBytes -= count;
}

static void popArgs(FILE *o)
{
    popArgCount(o, argBytes);
}

static void emitFunctionExit(FILE *o, const char *retReg)
{
    if (retReg && retReg[0] != '\0' && strcmp(retReg, "-"))
        fprintf(o, "    add  $v0,%s,$zero\n", retReg);

    if (!strcmp(currentFunction, "main")) {
        fprintf(o, "    hlt\n");
        return;
    }

    fprintf(o, "    lw   $ra,0($sp)\n");
    fprintf(o, "    addi $sp,$sp,1\n");
    fprintf(o, "    jr   $ra\n");
}

static int findLabelInFunction(Quadruple **body, int bodyCount, const char *label)
{
    int i;

    if (!label || !*label) return -1;

    for (i = 0; i < bodyCount; i++) {
        if (!strcmp(body[i]->op, "LAB") && !strcmp(body[i]->arg1, label))
            return i;
    }

    return -1;
}

static int getFlowSuccessors(Quadruple **body, int bodyCount, int idx, int succ[2])
{
    Quadruple *qq = body[idx];
    int n = 0;

    succ[0] = -1;
    succ[1] = -1;

    if (!strcmp(qq->op, "RET") || !strcmp(qq->op, "END") || !strcmp(qq->op, "HALT"))
        return 0;

    if (!strcmp(qq->op, "GOTO")) {
        int target = findLabelInFunction(body, bodyCount, qq->arg1);
        if (target >= 0) succ[n++] = target;
        return n;
    }

    if (!strcmp(qq->op, "IFF") || !strcmp(qq->op, "BNE")) {
        int target;

        if (idx + 1 < bodyCount)
            succ[n++] = idx + 1;

        target = findLabelInFunction(body, bodyCount, qq->arg2);
        if (target >= 0 && (n == 0 || succ[0] != target))
            succ[n++] = target;

        return n;
    }

    if (idx + 1 < bodyCount)
        succ[n++] = idx + 1;

    return n;
}

static int functionEndIsReachable(Quadruple *funQuad)
{
    Quadruple *qq;
    Quadruple **body;
    int *visited;
    int *stack;
    int bodyCount = 0;
    int endIdx = -1;
    int top = 0;
    int i;

    if (!funQuad || !funQuad->next) return 0;

    for (qq = funQuad->next; qq; qq = qq->next) {
        bodyCount++;
        if (!strcmp(qq->op, "END"))
            break;
    }

    if (bodyCount == 0 || !qq || strcmp(qq->op, "END"))
        return 0;

    body = (Quadruple **)malloc((size_t)bodyCount * sizeof(*body));
    visited = (int *)calloc((size_t)bodyCount, sizeof(*visited));
    stack = (int *)malloc((size_t)bodyCount * sizeof(*stack));
    if (!body || !visited || !stack) {
        perror("functionEndIsReachable");
        free(body);
        free(visited);
        free(stack);
        exit(EXIT_FAILURE);
    }

    qq = funQuad->next;
    for (i = 0; i < bodyCount; i++, qq = qq->next) {
        body[i] = qq;
        if (!strcmp(qq->op, "END"))
            endIdx = i;
    }

    stack[top++] = 0;
    while (top > 0) {
        int idx = stack[--top];
        int succ[2];
        int nSucc;
        int j;

        if (idx < 0 || idx >= bodyCount || visited[idx])
            continue;

        visited[idx] = 1;
        if (idx == endIdx)
            break;

        nSucc = getFlowSuccessors(body, bodyCount, idx, succ);
        for (j = 0; j < nSucc; j++) {
            if (succ[j] >= 0 && succ[j] < bodyCount && !visited[succ[j]])
                stack[top++] = succ[j];
        }
    }

    i = (endIdx >= 0) ? visited[endIdx] : 0;
    free(body);
    free(visited);
    free(stack);
    return i;
}

/* ============================================================ */
void asmGen(Quadruple *q, const char *asmFile)
{
    FILE *o = fopen(asmFile, "w");
    if (!o) {
        perror(asmFile);
        exit(EXIT_FAILURE);
    }

    fprintf(o, "# Assembly gerado automaticamente\n.text\n.globl main\n");

    while (q) {
        const char *op = q->op;

        /* ---------- Labels ---------- */
        if (!strcmp(op, "LAB")) {
            fprintf(o, "%s:\n", q->arg1);
            q = q->next;
            continue;
        }

        /* ---------- Função ---------- */
        if(!strcmp(op,"FUN")){
            fprintf(o,"%s:\n", q->arg2);   /* arg2 = nome da função */
            currentArgIndex = 0;
            currentFunctionArgCount = countFunctionArgs(q->next);
            paramCount = currentFunctionArgCount;
            currentFunction = q->arg2;
            currentFunctionEndReachable = functionEndIsReachable(q);
            argBytes = 0;
            stackDelta = 0;
            memset((void *)paramNames, 0, sizeof(paramNames));
            /* Prologue: salva $ra em offset fixo do frame (0($sp)); parâmetros ficam em 1($sp)... */
            fprintf(o, "    addi $sp,$sp,-1\n");
            fprintf(o, "    sw   $ra,0($sp)\n");
            stackDelta = 1;
            q = q->next; continue;
        }

        /* ---------- Parâmetro de função: só registrar nome; acesso via $sp ---------- */
        if (!strcmp(op, "ARG")) {
            if (currentArgIndex < MAX_PARAMS)
                paramNames[currentArgIndex] = q->arg2;  /* arg2 = nome do parâmetro */
            currentArgIndex++;
            q = q->next;
            continue;
        }

        /* ponteiros de registradores ------------------------ */
        const char *r1 = mapT(q->arg1);
        const char *r2 = mapT(q->arg2);
        const char *rd = mapT(q->result);

        /* ---------- ALU (R‑type) ---------- */
        if (!strcmp(op, "ADD"))       fprintf(o, "    add  %s,%s,%s\n", rd, r1, r2);
        else if (!strcmp(op, "SUB"))  fprintf(o, "    sub  %s,%s,%s\n", rd, r1, r2);
        /* MUL/DIV: escreve em High/Low; copiamos Low para rd */
        else if (!strcmp(op, "MUL")) {
            fprintf(o, "    mult %s,%s\n", r1, r2);
            fprintf(o, "    move %s,$lo\n", rd);
        }
        else if (!strcmp(op, "DIV")) {
            fprintf(o, "    div  %s,%s\n", r1, r2);
            fprintf(o, "    move %s,$lo\n", rd);
        }
        else if (!strcmp(op, "AND"))  fprintf(o, "    and  %s,%s,%s\n", rd, r1, r2);
        else if (!strcmp(op, "OR"))   fprintf(o, "    or   %s,%s,%s\n", rd, r1, r2);
        else if (!strcmp(op, "XOR"))  fprintf(o, "    xor  %s,%s,%s\n", rd, r1, r2);
        else if (!strcmp(op, "SR"))   fprintf(o, "    sr   %s,%s,%s\n", rd, r1, r2);
        else if (!strcmp(op, "SL"))   fprintf(o, "    sl   %s,%s,%s\n", rd, r1, r2);
        else if (!strcmp(op, "SLT"))  fprintf(o, "    slt  %s,%s,%s\n", rd, r1, r2);
        else if (!strcmp(op, "NOT"))  fprintf(o, "    not  %s,%s\n", rd, r1);
        else if (!strcmp(op, "MOVE")) fprintf(o, "    move %s,%s\n", rd, r1);
        /* Comparações: resultado 0/1 em rd */
        else if (!strcmp(op, "EQUAL") || !strcmp(op, "EQ")) {
            int lab = asmLabNum++;
            fprintf(o, "    sub  %s,%s,%s\n", rd, r1, r2);
            fprintf(o, "    beq  %s,$zero,.L_eq_%d\n", rd, lab);
            fprintf(o, "    addi %s,$zero,0\n", rd);
            fprintf(o, "    j    .L_eq_%d\n", lab + 1);
            fprintf(o, ".L_eq_%d:\n", lab);
            fprintf(o, "    addi %s,$zero,1\n", rd);
            fprintf(o, ".L_eq_%d:\n", lab + 1);
            asmLabNum++;
        }
        else if (!strcmp(op, "NEQ")) {
            int lab = asmLabNum++;
            fprintf(o, "    sub  %s,%s,%s\n", rd, r1, r2);
            fprintf(o, "    beq  %s,$zero,.L_ne_%d\n", rd, lab);
            fprintf(o, "    addi %s,$zero,1\n", rd);
            fprintf(o, "    j    .L_ne_%d\n", lab + 1);
            fprintf(o, ".L_ne_%d:\n", lab);
            fprintf(o, "    addi %s,$zero,0\n", rd);
            fprintf(o, ".L_ne_%d:\n", lab + 1);
            asmLabNum++;
        }
        else if (!strcmp(op, "LT"))   fprintf(o, "    slt  %s,%s,%s\n", rd, r1, r2);
        else if (!strcmp(op, "GT"))   fprintf(o, "    slt  %s,%s,%s\n", rd, r2, r1);
        else if (!strcmp(op, "LTE")) {
            const char *tmp = nextT();
            fprintf(o, "    slt  %s,%s,%s\n", tmp, r2, r1);
            fprintf(o, "    addi %s,$zero,1\n", rd);
            fprintf(o, "    sub  %s,%s,%s\n", rd, rd, tmp);
        }
        else if (!strcmp(op, "GTE")) {
            const char *tmp = nextT();
            fprintf(o, "    slt  %s,%s,%s\n", tmp, r1, r2);
            fprintf(o, "    addi %s,$zero,1\n", rd);
            fprintf(o, "    sub  %s,%s,%s\n", rd, rd, tmp);
        }

        /* ---------- Imediatos (I‑type) ---------- */
        else if (!strcmp(op, "ADDI")) fprintf(o, "    addi %s,%s,%s\n", rd, r1, q->arg2);
        else if (!strcmp(op, "ANDI")) fprintf(o, "    andi %s,%s,%s\n", rd, r1, q->arg2);
        else if (!strcmp(op, "ORI"))  fprintf(o, "    ori  %s,%s,%s\n", rd, r1, q->arg2);
        /* SUBI/MULI/DIVI não são comuns em MIPS‑like; se existir opcode próprio, adicione aqui. */

        /* ---------- Atribuição pura ---------- */
        else if (!strcmp(op, "ASSIGN")) {
            if (q->arg1[0] == 't' || q->arg1[0] == '$')
                fprintf(o, "    add  %s,%s,$zero\n", rd, r1);
            else
                fprintf(o, "    addi %s,$zero,%s\n", rd, q->arg1);
        }

        /* ---------- Memória ---------- */
        else if (!strcmp(op, "LOAD")) {
            int pidx = getParamIndex(q->arg1);
            if (pidx >= 0) {
                int off = (paramCount - 1 - pidx) + stackDelta;
                fprintf(o, "    lw   %s,%d($sp)\n", rd, off);
            } else {
                char memOp[32];
                resolveMemOperand(q->arg1, memOp, sizeof(memOp));
                fprintf(o, "    lw   %s,%s($gp)\n", rd, memOp);
            }
        }
        else if (!strcmp(op, "STORE")) {
            int pidx = getParamIndex(q->result);
            if (pidx >= 0) {
                int off = (paramCount - 1 - pidx) + stackDelta;
                fprintf(o, "    sw   %s,%d($sp)\n", r1, off);
            } else {
                char memOp[32];
                resolveMemOperand(q->result, memOp, sizeof(memOp));
                fprintf(o, "    sw   %s,%s($gp)\n", r1, memOp);
            }
        }

        /* Vetor: base em $sp (se parâmetro) ou $gp+memloc */
        else if (!strcmp(op, "LOADV")) {
            const char *arrName = q->arg1;
            const char *idxReg = mapT(q->arg2);
            const char *baseReg = nextTExcept(idxReg, rd);
            const char *addrReg = nextTExcept(idxReg, baseReg);
            int pidx = getParamIndex(arrName);
            if (pidx >= 0) {
                int off = (paramCount - 1 - pidx) + stackDelta;
                fprintf(o, "    lw   %s,%d($sp)\n", baseReg, off);
            } else {
                int loc = lookupMemloc(arrName);
                if (loc >= 0)
                    fprintf(o, "    addi %s,$gp,%d\n", baseReg, loc);
                else
                    fprintf(o, "    # LOADV base %s not found\n", arrName);
            }
            fprintf(o, "    add  %s,%s,%s\n", addrReg, baseReg, idxReg);
            fprintf(o, "    lw   %s,0(%s)\n", rd, addrReg);
        }
        else if (!strcmp(op, "STOREV")) {
            const char *arrName = q->result;
            const char *idxReg = mapT(q->arg2);
            const char *baseReg = nextTExcept(r1, idxReg);
            const char *addrReg = nextTExcept(r1, baseReg);
            int pidx = getParamIndex(arrName);
            if (pidx >= 0) {
                int off = (paramCount - 1 - pidx) + stackDelta;
                fprintf(o, "    lw   %s,%d($sp)\n", baseReg, off);
            } else {
                int loc = lookupMemloc(arrName);
                if (loc >= 0)
                    fprintf(o, "    addi %s,$gp,%d\n", baseReg, loc);
                else
                    fprintf(o, "    # STOREV base %s not found\n", arrName);
            }
            fprintf(o, "    add  %s,%s,%s\n", addrReg, baseReg, idxReg);
            fprintf(o, "    sw   %s,0(%s)\n", r1, addrReg);
        }

        /* Endereço de vetor/variável */
        else if (!strcmp(op, "ADDR")) {
            int loc = lookupMemloc(q->arg1);
            if (loc >= 0)
                fprintf(o, "    addi %s,$gp,%d\n", rd, loc);
            else
                fprintf(o, "    # ADDR %s: symbol not found\n", q->arg1);
        }

        /* ---------- Controle de fluxo ---------- */
        else if (!strcmp(op, "IFF"))  fprintf(o, "    beq  %s,$zero,%s\n", r1, q->arg2);
        else if (!strcmp(op, "BNE"))  fprintf(o, "    bne  %s,$zero,%s\n", r1, q->arg2);
        else if (!strcmp(op, "GOTO")) fprintf(o, "    j    %s\n", q->arg1);

        /* ---------- Funções ---------- */
        else if (!strcmp(op, "PARAM")) {
            pushArg(o, r1);
        }
        else if(!strcmp(op,"ARGS"))
            fprintf(o,"    # %s\n", op);
        else if(!strcmp(op,"END")) {
            fprintf(o,"    # END %s\n", currentFunction);
            if (currentFunctionEndReachable) {
                emitFunctionExit(o, NULL);
                if (!strcmp(currentFunction, "main"))
                    mainExitEmitted = 1;
            }
            currentFunction = "global";
            currentFunctionEndReachable = 1;
            currentArgIndex = 0;
            currentFunctionArgCount = 0;
            paramCount = 0;
            argBytes = 0;
            stackDelta = 0;
        }
        else if(!strcmp(op,"ALLOC")) {
            q = q->next;
            continue;
        }
        else if(!strcmp(op,"NOOP"))
            fprintf(o,"    nop\n");
        else if (!strcmp(op, "CALL")) {
            fprintf(o, "    jal  %s\n", q->arg1);
            popArgs(o);
            if (q->result[0] == 't')
                fprintf(o, "    add  %s,$v0,$zero\n", rd);
        }
        else if (!strcmp(op, "CALL_I")) fprintf(o, "    in   %s\n", rd);
        else if (!strcmp(op, "CALL_O")) {
            fprintf(o, "    out  %s\n", r1);
            popArgCount(o, 1);
        }

        else if (!strcmp(op, "RET")) {
            emitFunctionExit(o, strcmp(q->arg1, "-") ? r1 : NULL);
            if (!strcmp(currentFunction, "main"))
                mainExitEmitted = 1;
        }

        /* ---------- Fim de programa ---------- */
        else if (!strcmp(op, "HALT")) {
            if (!mainExitEmitted)
                fprintf(o, "    hlt\n");
        }

        /* ---------- NOP explícito ---------- */
        else if (!strcmp(op, "NOP"))   fprintf(o, "    nop\n");

        /* ---------- Não reconhecido ---------- */
        else {
            fprintf(o, "    # op %s nao implementado\n", op);
        }

        q = q->next;
    }

    fclose(o);
}
