/* encoder.h — Interface pública do montador binário (.tm → .txt) */
#ifndef ENCODER_H
#define ENCODER_H

/*
 * Converte um arquivo assembly textual (.s) para binário textual (.txt).
 *
 * Ao final da montagem, prefixa um NOP (opcode 010111) cuja palavra de
 * endereço (26 bits) contém N, o tamanho do programa em palavras, inclusive
 * esse NOP e o HLT final. O .s não contém esse cabeçalho.
 *
 * Retorna 0 em sucesso; <0 em erro de I/O ou sintaxe.
 */
int encodeAsm(const char *srcTM, const char *dstTXT);

#endif /* ENCODER_H */
