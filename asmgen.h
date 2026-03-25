
#ifndef _ASMGEN_H_
#define _ASMGEN_H_

#include "globals.h"
#include "cgen.h"

/* Gera código assembly MIPS-Lite a partir da lista de quádruplas.
 * Salva o resultado em um arquivo .s.
 */
void asmGen(Quadruple *quadList, const char *asmFile);

#endif
