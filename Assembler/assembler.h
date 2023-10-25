#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "../Libs/textlib.h"

enum asm_error
{
    NO_ERR,
    FILE_OPEN_ERR,
    FILE_CLOSE_ERR,
    TOO_MANY_ARGS_ERR
};

asm_error text_to_asm(File* input_file, const char* output_file_name);

void print_asm_err(asm_error err_code);

#endif
