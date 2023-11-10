#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "assembler_errors.h"

/**
 * @brief Convert command file to assembler bytecode
 */
asm_error compile_file(const char* input_file_name, const char* output_file_name, const char* listing_file_name);

#endif
