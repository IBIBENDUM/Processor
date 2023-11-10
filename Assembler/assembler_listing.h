#ifndef ASM_LISTING_H
#define ASM_LISTING_H

#include "../Libs/textlib.h"

#include "assembler.h"
#include "assembler_errors.h"

asm_error generate_listing(const char* listing_file_name, const File* input_file, const char* output_file_name, Bytecode* bytecode);

#endif
