// encoder.c — montador (.s → .txt) de palavras de 32 bits
//
// O .s não contém cabeçalho. Após validar e montar todas as instruções, o
// encoder prefixa no .txt um NOP cujo campo F3 contém o tamanho total:
//   palavra 0        NOP; bits[25:0] = N
//   palavras 1..N-2  corpo proveniente do .s
//   palavra N-1      HLT
// N inclui o NOP inicial e o HLT final.

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encoder.h"

/* Definido em main.c. Some 3072 em cada J/JAL e recusa imagem acima de um setor. */
extern int targetOsUser;

#define F1 '1'
#define F2 '2'
#define F3 '3'
#define FI 'I'

#define MAX_LABELS 256
#define MAX_LABEL_LENGTH 31
#define MAX_MNEMONIC_LENGTH 15
#define MAX_OPERAND_LENGTH 31
#define MAX_LINE_LENGTH 255
#define MAX_F3_ADDRESS 0x03FFFFFFL

enum {
    OP_MULT = 4,
    OP_DIV = 6,
    OP_JUMP = 17,
    OP_JUMPR = 18,
    OP_JAL = 19,
    OP_BEQ = 20,
    OP_BNE = 21,
    OP_MOVE = 22,
    OP_NOP = 23,
    OP_HLT = 24
};

typedef struct {
    const char *mnemonic;
    uint8_t opcode;
    char format;
} OpMap;

typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
} Label;

typedef struct {
    const OpMap *op;
    char operands[3][MAX_OPERAND_LENGTH + 1];
    int operandCount;
    int lineNumber;
} Instruction;

typedef struct {
    Instruction *items;
    size_t count;
    size_t capacity;
} InstructionList;

static const OpMap MAP[] = {
    /* Aritmética */
    {"ADD", 0, F1},       {"ADDI", 1, F2},
    {"SUB", 2, F1},       {"SUBI", 3, F2},
    {"MULT", OP_MULT, F1},{"MUL", OP_MULT, F1},
    {"MULTI", 5, F2},
    {"DIV", OP_DIV, F1},  {"DIVI", 7, F2},
    /* Lógica */
    {"AND", 8, F1},       {"ANDI", 9, F2},
    {"OR", 10, F1},       {"ORI", 11, F2},
    {"NOT", 12, F2},
    /* Deslocamento */
    {"SR", 13, F1},       {"SRL", 13, F1},
    {"SL", 14, F1},       {"SLL", 14, F1},
    /* Memória */
    {"LOAD", 15, F2},     {"LW", 15, F2},
    {"STORE", 16, F2},    {"SW", 16, F2},
    /* Saltos */
    {"JUMP", OP_JUMP, F3},{"J", OP_JUMP, F3},
    {"JUMPR", OP_JUMPR, F1}, {"JR", OP_JUMPR, F1},
    {"JAL", OP_JAL, F3},
    /* Saltos condicionais */
    {"BEQ", OP_BEQ, F2},  {"BNE", OP_BNE, F2},
    /* Transferência e controle */
    {"MOVE", OP_MOVE, F2},
    {"NOP", OP_NOP, F3},  {"HLT", OP_HLT, F3},
    /* Comparação e I/O */
    {"SLT", 25, F1},
    {"IN", 26, FI},       {"OUT", 27, FI},
    {NULL, 0, 0}
};

static Label labels[MAX_LABELS];
static int labelCount;

static int equalsIgnoreCase(const char *left, const char *right)
{
    if (!left || !right)
        return 0;

    while (*left && *right) {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right))
            return 0;
        left++;
        right++;
    }
    return *left == '\0' && *right == '\0';
}

static char *trim(char *text)
{
    char *end;

    while (isspace((unsigned char)*text))
        text++;

    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1]))
        end--;
    *end = '\0';
    return text;
}

static void removeComment(char *line)
{
    char *hash = strchr(line, '#');
    char *slashes = strstr(line, "//");
    char *comment = NULL;

    if (hash && slashes)
        comment = hash < slashes ? hash : slashes;
    else if (hash)
        comment = hash;
    else
        comment = slashes;

    if (comment)
        *comment = '\0';
}

static int isValidLabel(const char *name)
{
    const unsigned char *cursor = (const unsigned char *)name;

    if (!*cursor || !(isalpha(*cursor) || *cursor == '_' || *cursor == '.'))
        return 0;

    for (cursor++; *cursor; cursor++) {
        if (!(isalnum(*cursor) || *cursor == '_' || *cursor == '.'))
            return 0;
    }
    return 1;
}

static int findLabel(const char *name)
{
    int i;

    for (i = 0; i < labelCount; i++) {
        if (equalsIgnoreCase(name, labels[i].name))
            return labels[i].address;
    }
    return -1;
}

static int addLabel(const char *name, int address, int lineNumber)
{
    size_t length = strlen(name);

    if (!isValidLabel(name)) {
        fprintf(stderr, "[encoder] label inválido '%s' (linha %d)\n",
                name, lineNumber);
        return -1;
    }
    if (length > MAX_LABEL_LENGTH) {
        fprintf(stderr, "[encoder] label longo demais '%s' (linha %d)\n",
                name, lineNumber);
        return -1;
    }
    if (findLabel(name) >= 0) {
        fprintf(stderr, "[encoder] label duplicado '%s' (linha %d)\n",
                name, lineNumber);
        return -1;
    }
    if (labelCount >= MAX_LABELS) {
        fprintf(stderr, "[encoder] limite de %d labels excedido (linha %d)\n",
                MAX_LABELS, lineNumber);
        return -1;
    }

    memcpy(labels[labelCount].name, name, length + 1);
    labels[labelCount].address = address;
    labelCount++;
    return 0;
}

static const OpMap *findOp(const char *mnemonic)
{
    const OpMap *op;

    for (op = MAP; op->mnemonic; op++) {
        if (equalsIgnoreCase(mnemonic, op->mnemonic))
            return op;
    }
    return NULL;
}

static int appendInstruction(InstructionList *list,
                             const Instruction *instruction)
{
    Instruction *resized;
    size_t newCapacity;

    if (list->count == list->capacity) {
        newCapacity = list->capacity ? list->capacity * 2 : 32;
        resized = (Instruction *)realloc(
            list->items, newCapacity * sizeof(*list->items));
        if (!resized) {
            perror("[encoder] realloc");
            return -1;
        }
        list->items = resized;
        list->capacity = newCapacity;
    }

    list->items[list->count++] = *instruction;
    return 0;
}

static int copyOperand(char destination[MAX_OPERAND_LENGTH + 1],
                       const char *source, int lineNumber)
{
    size_t length = strlen(source);

    if (length > MAX_OPERAND_LENGTH) {
        fprintf(stderr, "[encoder] operando longo demais '%s' (linha %d)\n",
                source, lineNumber);
        return -1;
    }
    memcpy(destination, source, length + 1);
    return 0;
}

/*
 * Interpreta uma linha uma única vez. Labels são registrados com o endereço
 * final (+1), pois a palavra zero é reservada para o cabeçalho NOP.
 */
static int parseLine(char *line, int lineNumber, int bodyPc,
                     InstructionList *instructions)
{
    Instruction instruction;
    char *cursor;
    char *colon;
    char *token;

    removeComment(line);
    cursor = trim(line);
    if (!*cursor)
        return 0;

    colon = strchr(cursor, ':');
    if (colon) {
        char *label;
        *colon = '\0';
        label = trim(cursor);
        if (addLabel(label, bodyPc + 1, lineNumber) != 0)
            return -1;
        cursor = trim(colon + 1);
        if (!*cursor)
            return 0;
    }

    /* Diretivas não geram palavras, inclusive quando aparecem após label. */
    if (*cursor == '.')
        return 0;

    memset(&instruction, 0, sizeof(instruction));
    instruction.lineNumber = lineNumber;

    token = strtok(cursor, " ,\t\r\n");
    if (!token)
        return 0;
    if (strlen(token) > MAX_MNEMONIC_LENGTH) {
        fprintf(stderr, "[encoder] mnemônico longo demais '%s' (linha %d)\n",
                token, lineNumber);
        return -1;
    }

    instruction.op = findOp(token);
    if (!instruction.op) {
        fprintf(stderr, "[encoder] mnemônico desconhecido '%s' (linha %d)\n",
                token, lineNumber);
        return -1;
    }

    while ((token = strtok(NULL, " ,\t\r\n")) != NULL) {
        if (instruction.operandCount >= 3) {
            fprintf(stderr, "[encoder] operandos em excesso (linha %d)\n",
                    lineNumber);
            return -1;
        }
        if (copyOperand(instruction.operands[instruction.operandCount],
                        token, lineNumber) != 0)
            return -1;
        instruction.operandCount++;
    }

    return appendInstruction(instructions, &instruction);
}

static int parseInteger(const char *text, long minimum, long maximum,
                        long *value, const char *description, int lineNumber)
{
    char *end;
    long parsed;

    if (!text || !*text) {
        fprintf(stderr, "[encoder] %s ausente (linha %d)\n",
                description, lineNumber);
        return -1;
    }

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno == ERANGE || *end != '\0' || parsed < minimum || parsed > maximum) {
        fprintf(stderr, "[encoder] %s inválido '%s' (linha %d)\n",
                description, text, lineNumber);
        return -1;
    }
    *value = parsed;
    return 0;
}

static int parseRegister(const char *text, int *value, int lineNumber)
{
    long number;
    const char *digits;
    int base;
    long maximumAlias;

    if (!text || !*text) {
        fprintf(stderr, "[encoder] registrador ausente (linha %d)\n", lineNumber);
        return -1;
    }

    if (equalsIgnoreCase(text, "$zero"))
        number = 0;
    else if (equalsIgnoreCase(text, "$v0"))
        number = 2;
    else if (equalsIgnoreCase(text, "$gp"))
        number = 28;
    else if (equalsIgnoreCase(text, "$sp"))
        number = 29;
    else if (equalsIgnoreCase(text, "$fp"))
        number = 30;
    else if (equalsIgnoreCase(text, "$ra"))
        number = 31;
    else if (equalsIgnoreCase(text, "$lo"))
        number = 61;
    else if (equalsIgnoreCase(text, "$hi"))
        number = 62;
    else {
        digits = text;
        base = 0;
        maximumAlias = 63;
        if (*digits == '$')
            digits++;

        if (tolower((unsigned char)*digits) == 't') {
            digits++;
            base = 8;
            maximumAlias = 9;
        } else if (tolower((unsigned char)*digits) == 's') {
            digits++;
            base = 16;
            maximumAlias = 7;
        }

        if (parseInteger(digits, 0, maximumAlias, &number,
                         "registrador", lineNumber) != 0)
            return -1;
        number += base;
    }

    *value = (int)number;
    return 0;
}

static int normalizeMemoryOperand(Instruction *instruction)
{
    char memory[MAX_OPERAND_LENGTH + 1];
    char *leftParen;
    char *rightParen;
    char *offset;
    char *base;

    if (instruction->operandCount != 2)
        return 0;

    memcpy(memory, instruction->operands[1], sizeof(memory));
    leftParen = strchr(memory, '(');
    if (!leftParen)
        return 0;
    rightParen = strchr(leftParen + 1, ')');
    if (!rightParen || rightParen[1] != '\0') {
        fprintf(stderr, "[encoder] endereço de memória inválido '%s' (linha %d)\n",
                instruction->operands[1], instruction->lineNumber);
        return -1;
    }

    *leftParen = '\0';
    *rightParen = '\0';
    offset = memory;
    base = leftParen + 1;
    if (!*offset)
        offset = "0";
    if (!*base) {
        fprintf(stderr, "[encoder] registrador-base ausente (linha %d)\n",
                instruction->lineNumber);
        return -1;
    }

    if (copyOperand(instruction->operands[1], base,
                    instruction->lineNumber) != 0 ||
        copyOperand(instruction->operands[2], offset,
                    instruction->lineNumber) != 0)
        return -1;
    instruction->operandCount = 3;
    return 0;
}

static int requireOperandCount(const Instruction *instruction,
                               int minimum, int maximum)
{
    if (instruction->operandCount < minimum ||
        instruction->operandCount > maximum) {
        fprintf(stderr,
                "[encoder] quantidade inválida de operandos para %s "
                "(linha %d; esperado %d%s)\n",
                instruction->op->mnemonic, instruction->lineNumber,
                minimum, minimum == maximum ? "" : " ou mais");
        return -1;
    }
    return 0;
}

static uint32_t makeF1(uint8_t op, int rs, int rt, int rd, int shamt)
{
    return ((uint32_t)op << 26) |
           (((uint32_t)rs & 0x3F) << 20) |
           (((uint32_t)rt & 0x3F) << 14) |
           (((uint32_t)rd & 0x3F) << 8) |
           ((uint32_t)shamt & 0xFF);
}

static uint32_t makeF2(uint8_t op, int rd, int rs, int immediate)
{
    return ((uint32_t)op << 26) |
           (((uint32_t)rd & 0x3F) << 20) |
           (((uint32_t)rs & 0x3F) << 14) |
           ((uint32_t)immediate & 0x3FFF);
}

static uint32_t makeF3(uint8_t op, long address)
{
    return ((uint32_t)op << 26) | ((uint32_t)address & 0x03FFFFFF);
}

static uint32_t makeIO(uint8_t op, int reg)
{
    return ((uint32_t)op << 26) | (((uint32_t)reg & 0x3F) << 20);
}

static int resolveRelativeImmediate(const char *operand, int pc,
                                    int lineNumber, int *immediate)
{
    int address = findLabel(operand);
    long parsed;

    if (address >= 0)
        parsed = address - pc - 1;
    else if (isValidLabel(operand)) {
        fprintf(stderr, "[encoder] label inexistente '%s' (linha %d)\n",
                operand, lineNumber);
        return -1;
    } else if (parseInteger(operand, -8192, 8191, &parsed,
                            "imediato F2", lineNumber) != 0)
        return -1;

    if (parsed < -8192 || parsed > 8191) {
        fprintf(stderr, "[encoder] deslocamento fora de 14 bits: %ld "
                "(linha %d)\n", parsed, lineNumber);
        return -1;
    }
    *immediate = (int)parsed;
    return 0;
}

static int applyUserBase(long *address, int lineNumber);

static int resolveAbsoluteAddress(const char *operand, int lineNumber,
                                  long *address)
{
    int labelAddress = findLabel(operand);
    long parsed;

    if (labelAddress >= 0) {
        *address = labelAddress;
        return applyUserBase(address, lineNumber);
    }
    if (isValidLabel(operand)) {
        fprintf(stderr, "[encoder] label inexistente '%s' (linha %d)\n",
                operand, lineNumber);
        return -1;
    }

    /*
     * Compatibilidade: números em .s antigos endereçam o corpo sem cabeçalho.
     * O NOP inserido na palavra zero desloca esses destinos em uma palavra.
     */
    if (parseInteger(operand, 0, MAX_F3_ADDRESS - 1, &parsed,
                     "endereço absoluto", lineNumber) != 0)
        return -1;
    *address = parsed + 1;
    return applyUserBase(address, lineNumber);
}

static int applyUserBase(long *address, int lineNumber)
{
    if (!targetOsUser)
        return 0;
    if (*address < 0 || *address > (long)MAX_F3_ADDRESS - 3072) {
        fprintf(stderr, "[encoder] J/JAL nao cabe na folha (linha %d)\n",
                lineNumber);
        return -1;
    }
    *address += 3072;
    return 0;
}

static int encodeInstruction(Instruction *instruction, int pc, uint32_t *word)
{
    const OpMap *op = instruction->op;
    int rd = 0;
    int rs = 0;
    int rt = 0;
    int immediate = 0;
    long parsed;
    long address = 0;

    if ((op->opcode == 15 || op->opcode == 16) &&
        normalizeMemoryOperand(instruction) != 0)
        return -1;

    switch (op->format) {
    case F1:
        if (op->opcode == OP_JUMPR) {
            if (requireOperandCount(instruction, 1, 1) != 0 ||
                parseRegister(instruction->operands[0], &rs,
                              instruction->lineNumber) != 0)
                return -1;
            *word = makeF1(op->opcode, rs, 0, 0, 0);
        } else if (op->opcode == OP_MULT || op->opcode == OP_DIV) {
            if (requireOperandCount(instruction, 2, 2) != 0 ||
                parseRegister(instruction->operands[0], &rs,
                              instruction->lineNumber) != 0 ||
                parseRegister(instruction->operands[1], &rt,
                              instruction->lineNumber) != 0)
                return -1;
            *word = makeF1(op->opcode, rs, rt, 0, 0);
        } else if (op->opcode == 13 || op->opcode == 14) {
            /* SR/SL: RD ← RS >> shamt. O terceiro operando é o imediato,
             * não um registrador. O processador lê esse valor em instr[7:0]. */
            long shamt;
            if (requireOperandCount(instruction, 3, 3) != 0 ||
                parseRegister(instruction->operands[0], &rd,
                              instruction->lineNumber) != 0 ||
                parseRegister(instruction->operands[1], &rs,
                              instruction->lineNumber) != 0 ||
                parseInteger(instruction->operands[2], 0, 31, &shamt,
                             "shamt", instruction->lineNumber) != 0)
                return -1;
            *word = makeF1(op->opcode, rs, 0, rd, (int)shamt);
        } else {
            if (requireOperandCount(instruction, 3, 3) != 0 ||
                parseRegister(instruction->operands[0], &rd,
                              instruction->lineNumber) != 0 ||
                parseRegister(instruction->operands[1], &rs,
                              instruction->lineNumber) != 0 ||
                parseRegister(instruction->operands[2], &rt,
                              instruction->lineNumber) != 0)
                return -1;
            *word = makeF1(op->opcode, rs, rt, rd, 0);
        }
        return 0;

    case F2:
        if (op->opcode == OP_MOVE || op->opcode == 12) {
            if (requireOperandCount(instruction, 2, 2) != 0 ||
                parseRegister(instruction->operands[0], &rd,
                              instruction->lineNumber) != 0 ||
                parseRegister(instruction->operands[1], &rs,
                              instruction->lineNumber) != 0)
                return -1;
            *word = makeF2(op->opcode, rd, rs, 0);
            return 0;
        }

        if (requireOperandCount(instruction, 3, 3) != 0 ||
            parseRegister(instruction->operands[0], &rd,
                          instruction->lineNumber) != 0 ||
            parseRegister(instruction->operands[1], &rs,
                          instruction->lineNumber) != 0)
            return -1;

        if (op->opcode == OP_BEQ || op->opcode == OP_BNE) {
            if (resolveRelativeImmediate(instruction->operands[2], pc,
                                         instruction->lineNumber,
                                         &immediate) != 0)
                return -1;
        } else {
            if (parseInteger(instruction->operands[2], -8192, 8191, &parsed,
                             "imediato F2", instruction->lineNumber) != 0)
                return -1;
            immediate = (int)parsed;
        }
        *word = makeF2(op->opcode, rd, rs, immediate);
        return 0;

    case F3:
        if (op->opcode == OP_JUMP || op->opcode == OP_JAL) {
            if (requireOperandCount(instruction, 1, 1) != 0 ||
                resolveAbsoluteAddress(instruction->operands[0],
                                       instruction->lineNumber,
                                       &address) != 0)
                return -1;
        } else {
            if (requireOperandCount(instruction, 0, 0) != 0)
                return -1;
            address = 0;
        }
        *word = makeF3(op->opcode, address);
        return 0;

    case FI:
        if (requireOperandCount(instruction, 1, 1) != 0 ||
            parseRegister(instruction->operands[0], &rd,
                          instruction->lineNumber) != 0)
            return -1;
        *word = makeIO(op->opcode, rd);
        return 0;
    }

    fprintf(stderr, "[encoder] formato desconhecido (linha %d)\n",
            instruction->lineNumber);
    return -1;
}

static int writeWord(FILE *output, uint32_t word)
{
    int bit;

    for (bit = 31; bit >= 0; bit--) {
        if (fputc((word >> bit) & 1U ? '1' : '0', output) == EOF)
            return -1;
    }
    return fputc('\n', output) == EOF ? -1 : 0;
}

static int writeAtomically(const char *destination,
                           const uint32_t *words, size_t wordCount)
{
    size_t pathLength = strlen(destination);
    char *temporaryPath = (char *)malloc(pathLength + 5);
    FILE *output;
    size_t i;
    int result = 0;

    if (!temporaryPath) {
        perror("[encoder] malloc");
        return -2;
    }
    memcpy(temporaryPath, destination, pathLength);
    memcpy(temporaryPath + pathLength, ".tmp", 5);

    output = fopen(temporaryPath, "w");
    if (!output) {
        perror(temporaryPath);
        free(temporaryPath);
        return -2;
    }

    for (i = 0; i < wordCount; i++) {
        if (writeWord(output, words[i]) != 0) {
            perror(temporaryPath);
            result = -2;
            break;
        }
    }
    if (fclose(output) != 0) {
        perror(temporaryPath);
        result = -2;
    }

    if (result == 0 && rename(temporaryPath, destination) != 0) {
        perror(destination);
        result = -2;
    }
    if (result != 0)
        remove(temporaryPath);

    free(temporaryPath);
    return result;
}

int encodeAsm(const char *srcAsm, const char *dstTXT)
{
    FILE *input;
    InstructionList instructions = {NULL, 0, 0};
    char line[MAX_LINE_LENGTH + 2];
    int lineNumber = 0;
    int parseFailed = 0;
    int needHalt;
    size_t programSize;
    uint32_t *words = NULL;
    size_t i;
    int result = 0;

    input = fopen(srcAsm, "r");
    if (!input)
        return -1;

    labelCount = 0;
    while (fgets(line, sizeof(line), input)) {
        size_t length;
        lineNumber++;
        length = strlen(line);
        if (length == MAX_LINE_LENGTH + 1 && line[length - 1] != '\n') {
            int ch;
            while ((ch = fgetc(input)) != '\n' && ch != EOF)
                ;
            fprintf(stderr, "[encoder] linha %d longa demais\n", lineNumber);
            parseFailed = 1;
            continue;
        }
        if (parseLine(line, lineNumber, (int)instructions.count,
                      &instructions) != 0)
            parseFailed = 1;
    }
    if (ferror(input)) {
        perror(srcAsm);
        parseFailed = 1;
    }
    fclose(input);

    if (parseFailed) {
        free(instructions.items);
        return -3;
    }

    needHalt = instructions.count == 0 ||
               instructions.items[instructions.count - 1].op->opcode != OP_HLT;
    programSize = instructions.count + 1U + (needHalt ? 1U : 0U);
    if (targetOsUser && programSize > 256) {
        fprintf(stderr, "[encoder] os-user excede 256 palavras (%zu)\n",
                programSize);
        free(instructions.items);
        return -3;
    }
    if (programSize > (size_t)MAX_F3_ADDRESS) {
        fprintf(stderr, "[encoder] programa excede o limite de 26 bits\n");
        free(instructions.items);
        return -3;
    }

    words = (uint32_t *)malloc(programSize * sizeof(*words));
    if (!words) {
        perror("[encoder] malloc");
        free(instructions.items);
        return -2;
    }

    words[0] = makeF3(OP_NOP, (long)programSize);
    for (i = 0; i < instructions.count; i++) {
        if (encodeInstruction(&instructions.items[i], (int)i + 1,
                              &words[i + 1]) != 0)
            result = -3;
    }
    if (needHalt)
        words[programSize - 1] = makeF3(OP_HLT, 0);

    if (result == 0)
        result = writeAtomically(dstTXT, words, programSize);

    free(words);
    free(instructions.items);
    return result;
}
