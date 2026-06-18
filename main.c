/* main.c – CLI aprimorada com suporte a --txt
 * Compilador C‑ → MIPS‑Lite
 * Nilton, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"

/* set NO_PARSE to TRUE to obter apenas scanner */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to obter apenas parser */
#define NO_ANALYZE FALSE
/* set NO_CODE to TRUE to compilar sem gerar código */
#define NO_CODE FALSE

#include "util.h"
#if NO_PARSE
  #include "scan.h"
#else
  #include "parse.h"
  #if !NO_ANALYZE
    #include "analyze.h"
    #if !NO_CODE
      #include "cgen.h"
    #endif
  #endif
#endif

/* ---------------------- variáveis globais ---------------------- */
int lineno = 0;
FILE *source;
FILE *listing;
FILE *code;

/* flags de tracing */
int EchoSource   = TRUE;
int TraceScan    = TRUE;
int TraceParse   = TRUE;
int TraceAnalyze = TRUE;
int TraceCode    = TRUE;

int Error = FALSE;

/* ---------------------- protótipos ----------------------------- */
static void usage(const char *prog) {
    fprintf(stderr,"usage: %s [--txt] <file[.cms]>\n", prog);
}

/* ---------------------- função principal ----------------------- */
int main(int argc, char *argv[]) {
    int outTxt = 0;      /* flag --txt */
    char *inPath = NULL; /* arquivo de entrada */

    /* --- parse de argumentos simples --- */
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--txt") == 0) {
            outTxt = 1;
        } else if (!inPath) {
            inPath = argv[i];
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if (!inPath) {
        usage(argv[0]);
        return 1;
    }

    /* cria nome do arquivo .cms se não tiver extensão */
    char pgm[260];
    strncpy(pgm, inPath, sizeof(pgm) - 1);
    pgm[sizeof(pgm) - 1] = '\0';
    if (strchr(pgm, '.') == NULL) strcat(pgm, ".cms");

    /* abre fonte */
    source = fopen(pgm, "r");
    if (!source) {
        fprintf(stderr, "File %s not found\n", pgm);
        return 1;
    }

    listing = stdout;
    fprintf(listing, "\nC- COMPILATION: %s\n", pgm);

    /* ---------------------- FRONT‑END ---------------------- */
#if NO_PARSE
    while (getToken() != ENDFILE);
#else
    TreeNode *syntaxTree = parse();
    if (TraceParse) {
        fprintf(listing, "\nSyntax tree:\n");
        printTree(syntaxTree);
    }

#if !NO_ANALYZE
    if (!Error) {
        if (TraceAnalyze) fprintf(listing, "\nBuilding Symbol Table...\n");
        buildSymtab(syntaxTree);
        if (TraceAnalyze) fprintf(listing, "\nChecking Types...\n");
        typeCheck(syntaxTree);
        if (TraceAnalyze) fprintf(listing, "\nType Checking Finished\n");
    }

#if !NO_CODE
    if (!Error) {
        /* gera nome‑base (sem extensão) */
        char base[260];
        strncpy(base, pgm, sizeof(base) - 1);
        base[sizeof(base) - 1] = '\0';
        char *dot = strrchr(base, '.');
        if (dot) *dot = '\0';

        /* arquivo .tm para IR/assembly (output da codeGen) */
        char codefile[264];
        snprintf(codefile, sizeof(codefile), "%s.tm", base);
        code = fopen(codefile, "w");
        if (!code) {
            fprintf(stderr, "Unable to open %s\n", codefile);
            return 1;
        }

        /* codeGen deve produzir também <base>.mem */
        codeGen(syntaxTree, base); 
        fclose(code);

        /* se --txt, renomeia <base>.mem → <base>.txt */
        if (outTxt) {
            char memName[264], txtName[264];
            snprintf(memName, sizeof(memName), "%s.mem", base);
            snprintf(txtName, sizeof(txtName), "%s.txt", base);
            if (rename(memName, txtName) != 0) {
                perror("rename .mem → .txt");
            }
        }
    }
#endif /* !NO_CODE */
#endif /* !NO_ANALYZE */
#endif /* !NO_PARSE */

    fclose(source);
    return 0;
}