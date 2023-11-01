#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "assembler_errors.h"

/**
 * @brief Parse command file to assembler bytecode
 *
 * @param input_file_name
 * @param output_file_name
 * @return asm_error
 */
asm_error file_to_asm(const char* input_file_name, const char* output_file_name);

#endif
