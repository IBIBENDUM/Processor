#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdlib.h>
#include <stdint.h>
#include "assembler_errors.h"

const size_t MIN_BYTECODE_CAPACITY = 32;
struct Bytecode
{
    uint8_t* code_array;
    size_t position;
};

/**
 * @brief Convert command file to assembler bytecode
 */
asm_error compile_file(const char* input_file_name, const char* output_file_name, const char* listing_file_name);

bool parse_label(line* op_name, const line* line_ptr);

#endif
