#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdlib.h>
#include <stdint.h>
#include "assembler_errors.h"

#include "../common.h"

struct Command
{
    cmd_t   cmd_id;
    arg_t   reg_id;
    arg_t   imm_value;
    uint8_t possible_args_bitmask;

    bool    has_ram;
    bool    has_reg;
    bool    has_imm;
    bool    has_label;
};

const ssize_t LABEL_POISON_VALUE = -1;
const size_t MAX_LABEL_NAME_LENGTH = 50;
struct Label
{
    wchar_t name[MAX_LABEL_NAME_LENGTH] = {};
    arg_t cmd_pos = 0;
};

const size_t LABELS_MAX_AMOUNT = 100;
struct Labels
{
    Label  labels_arr[LABELS_MAX_AMOUNT] = {};
    size_t amount = 0;
    size_t final_size = 0;
};

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
