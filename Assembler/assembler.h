#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "../Libs/textlib.h"

#define ASM_ERRORS_LIST\
    DEF_ASM_ERR(ASM_NO_ERR)\
    DEF_ASM_ERR(ASM_INPUT_FILE_READ_ERR)\
    DEF_ASM_ERR(ASM_FILE_OPEN_ERR)\
    DEF_ASM_ERR(ASM_FILE_CLOSE_ERR)\
    DEF_ASM_ERR(ASM_MEM_ALLOC_ERR)\
    DEF_ASM_ERR(ASM_PARSE_ARGS_ERR)\
    DEF_ASM_ERR(ASM_PARSE_LABELS_ERR)\
    DEF_ASM_ERR(ASM_WRITE_ERR)\

#define DEF_ASM_ERR(NAME) NAME,
enum asm_error
{
    ASM_ERRORS_LIST
};
#undef DEF_ASM_ERR

#define DEF_ASM_ERR(NAME) #NAME,
const char * const asm_errors_strs[] = { ASM_ERRORS_LIST };
#undef DEF_ASM_ERR

asm_error file_to_asm(const char* input_file_name, const char* output_file_name);

void print_asm_err(asm_error err_code);

#endif
