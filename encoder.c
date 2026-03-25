// encoder.c — montador (.tm → .txt) 32 bits*

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "encoder.h"

/* ——— utilidades ——— */
static int mapReg(const char *r){
    if(!r||!*r) return 0;
    if(!strcasecmp(r,"$zero")) return 0;
    if(!strcasecmp(r,"$sp"))   return 29;
    if(!strcasecmp(r,"$gp"))   return 28;
    if(!strcasecmp(r,"$ra"))   return 31;
    if(!strcasecmp(r,"$hi"))   return 62;   /* High: resto (Div), parte alta (Mult) */
    if(!strcasecmp(r,"$lo"))   return 61;   /* Low: quociente (Div), parte baixa (Mult) */
    if(r[0]=='$'&&r[1]=='t') return 8  + atoi(r+2);   /* $t0‑$t9 */
    if(r[0]=='$'&&r[1]=='s') return 16 + atoi(r+2);   /* $s0‑$s7 */
    if(r[0]=='$') return atoi(r+1);
    return atoi(r);            /* fallback numérico */
}

typedef struct {const char *mn; uint8_t op; char fmt;} Map;
#define F1 '1'
#define F2 '2'
#define F3 '3'
#define FI 'I'
static const Map MAP[] = {
    /* Aritmética */
    {"ADD",  0b000000,F1},{"ADDI", 0b000001,F2},
    {"SUB",  0b000010,F1},{"SUBI", 0b000011,F2},
    {"MULT", 0b000100,F1},{"mul", 0b000100,F1},{"mult", 0b000100,F1},
    {"MULTI",0b000101,F2},
    {"DIV",  0b000110,F1},{"DIVI", 0b000111,F2},
    /* Lógica */
    {"AND",   0b001000,F1},{"ANDI", 0b001001,F2},
    {"OR",    0b001010,F1},{"ORI",  0b001011,F2},
    {"NOT",   0b001100,F2},
    /* Deslocamento */
    {"SR",    0b001101,F1}, {"srl", 0b001101,F1}, {"SRL", 0b001101,F1},
    {"SL",    0b001110,F1}, {"sll", 0b001110,F1}, {"SLL", 0b001110,F1},
    /* Memória */
    {"LOAD",  0b001111,F2}, {"lw", 0b001111,F2},
    {"STORE", 0b010000,F2}, {"sw", 0b010000,F2},
    /* Saltos */
    {"JUMP", 0b010001, F3}, {"j",   0b010001, F3},
    {"JUMPR",0b010010, F1}, {"jr",  0b010010, F1},
    {"JAL",  0b010011, F3},
    /* Saltos cond. */
    {"BEQ",   0b010100,F2},{"BNE",  0b010101,F2},
    /* Transferência */
    {"MOVE",  0b010110,F2},
    /* Controle */
    {"NOP",   0b010111,F3},{"HLT",  0b011000,F3},
    /* Comparação */
    {"SLT",   0b011001,F1},
    /* I/O */
    {"IN",    0b011010,FI},{"OUT",  0b011011,FI},
    {NULL,0,0}
};

static const Map *findOp(const char *mn){
    for(const Map *m=MAP;m->mn;++m)
        if(!strcasecmp(mn,m->mn)) return m;
    return NULL;
}

/* ——— estrutura para labels (hash linear simples) ——— */
#define MAXLBL 256
static struct {char name[32]; int addr;} lbls[MAXLBL];
static int lblCnt;
static int addLabel(const char *n,int a){
    if(lblCnt>=MAXLBL) return -1;
    strncpy(lbls[lblCnt].name,n,31); lbls[lblCnt].addr=a; return lblCnt++;
}
static int findLabel(const char *n){
    for(int i=0;i<lblCnt;i++) if(!strcasecmp(n,lbls[i].name)) return lbls[i].addr;
    return -1;
}

/* ——— monta uma palavra 32 bits ——— */
static uint32_t makeF1(uint8_t op,int rd,int rs,int rt){
    return (op<<26)|(rd<<21)|(rs<<16)|(rt<<11);
}
static uint32_t makeF2(uint8_t op,int rt,int rs,int imm){
    return (op<<26)|(rt<<21)|(rs<<16)|((uint16_t)imm & 0xFFFF);
}
static uint32_t makeF3(uint8_t op,int addr){
    return (op<<26)|((uint32_t)addr & 0x03FFFFFF);
}
static uint32_t makeIO(uint8_t op,int reg){
    return (op<<26)|(reg<<20);
}

/* ——— implementação principal ——— */
int encodeAsm(const char *srcTM,const char *dstTXT){
    FILE *fin=fopen(srcTM,"r"); if(!fin) return -1;
    /* 1ª passagem — rótulos */
    lblCnt=0; int pc=0; char line[128];
    while(fgets(line,sizeof line,fin)){
        char *p=line; while(isspace(*p)) ++p;
        if(*p=='#'||*p=='/'&&p[1]=='/') continue; /* comentário */
        if(*p=='.'||*p=='\0' || *p=='\n') continue; /* diretiva/vazia */
        char *col=strchr(p,':');
        if(col){ *col='\0'; addLabel(p,pc); p=col+1; while(isspace(*p)) ++p; }
        if(*p=='\0'||*p=='\n') continue; /* linha só com label */
        pc++; /* conta instrução */
    }
    rewind(fin); pc=0;
    FILE *fout=fopen(dstTXT,"w"); if(!fout){fclose(fin);return -2;}

    while(fgets(line,sizeof line,fin)){
        char *p=line; while(isspace(*p)) ++p;
        if(*p=='#'||*p=='/'&&p[1]=='/') continue;
        if(*p=='.'||*p=='\0'||*p=='\n') continue;
        /* corta comentário inline */
        char *cmt=strstr(p,"//"); if(cmt) *cmt='\0'; cmt=strchr(p,'#'); if(cmt) *cmt='\0';
        /* label inline */
        char *col=strchr(p,':'); if(col){ p=col+1; while(isspace(*p)) ++p; }
        if(*p=='\0'||*p=='\n') continue;

        char *tok=strtok(p," ,\t\r\n"); if(!tok) continue;
        const Map *m=findOp(tok);
        if(!m){ fprintf(stderr,"[encoder] mnemônico desconhecido: %s (linha %d)\n",tok,pc+1); continue; }
        char *op1=strtok(NULL," ,\t\r\n");
        char *op2=strtok(NULL," ,\t\r\n");
        char *op3=strtok(NULL," ,\t\r\n");

        if (op2) {
            if (op2[0] == '(') {              /* caso (rs) */
                ++op2;                        /* pula '('   */
                char *rp = strchr(op2, ')');  /* corta ')'  */
                if (rp) *rp = '\0';
            } else {
                char *lp = strchr(op2, '(');  /* caso imm(rs) */
                if (lp) {
                    char *rs = lp + 1;        /* começa após '(' */
                    char *rp = strchr(rs, ')');
                    if (rp) *rp = '\0';
                    *lp = '\0';               /* termina string do imm em op2 */
                    if (!op3) op3 = op2;      /* deslocamento imediato        */
                    op2 = rs;                 /* agora op2 = registrador base */
                }
            }
        }

        uint32_t word=0;
        switch(m->fmt){
            case F1:{
                /* Div e Mult: dois operandos (RS, RT); resultado em High/Low; RD ignorado */
                int rdF1 = mapReg(op1), rsF1 = mapReg(op2), rtF1 = op3 ? mapReg(op3) : 0;
                if (!op3 && (m->op == 0b000110 || m->op == 0b000100)) /* DIV ou MULT */
                    word = makeF1(m->op, 0, rdF1, rsF1);  /* RD=0, RS=op1, RT=op2 */
                else
                    word = makeF1(m->op, rdF1, rsF1, rtF1);
                break; }
            case F2:{
                int imm=0; if(op3){ if(isalpha(*op3)) imm=findLabel(op3)-pc-1; else imm=atoi(op3);} /* label relativo */
                word=makeF2(m->op, mapReg(op1), mapReg(op2), imm); break; }
            case F3:{
                int addr=0; if(op1){ addr=isalpha(*op1)?findLabel(op1):atoi(op1);} word=makeF3(m->op,addr); break; }
            case FI:{
                word=makeIO(m->op, mapReg(op1)); break; }
        }
        for(int b=31;b>=0;b--) fputc((word>>b)&1?'1':'0',fout);
        fputc('\n',fout);
        pc++;
    }

    fclose(fin); fclose(fout);
    return 0;
}
