/* encoder.h — Interface pública do montador binário (.tm → .txt) */
#ifndef ENCODER_H
#define ENCODER_H

/*
 * Converte um arquivo assembly textual (.tm) para binário textual (.txt).
 * Retorna 0 em sucesso; <0 em erro de I/O ou sintaxe.
 */
int encodeAsm(const char *srcTM, const char *dstTXT);

#endif /* ENCODER_H */
