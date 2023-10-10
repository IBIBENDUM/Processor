#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "../Libs/textlib.h"

void parse_file_to_commands(File* file);

void write_to_file(const char* output_file_name, int* code_array, const size_t size);

int* parse_file_to_commands(File* file, size_t* position);

#endif
