#include <time.h>
#include <stdio.h>
#include <assert.h>

#include "../Libs/textlib.h"
#include "../Libs/logs.h"
#include "../Libs/time_utils.h"
#include "../common.h"

#include "assembler.h"
#include "assembler_listing.h"
#include "assembler_errors.h"

const size_t BYTECODE_PADDING = 15;
const size_t LINE_IDX_PADDING = 5;
const size_t AMOUNT_OF_LEADING_ZEROES = 4;
const size_t SPACE_AMOUNT_AFTER_POSITION = 6;

static char* format_bytecode_string(Bytecode* bytecode)
{
    static char buffer[16] = "";

    const uint8_t* cmd_ptr = bytecode->code_array + bytecode->position;
    size_t buffer_pos = sprintf(buffer, "%0*llX", AMOUNT_OF_LEADING_ZEROES, *(cmd_t*)cmd_ptr);
    bytecode->position += sizeof(cmd_t);

    if ((*(cmd_t*)cmd_ptr & ARG_IMM_MASK) || (*(cmd_t*)cmd_ptr & ARG_REG_MASK))
    {
        sprintf(buffer + buffer_pos, " %0*llX", AMOUNT_OF_LEADING_ZEROES, *(arg_t*)(cmd_ptr + sizeof(cmd_t)));
        bytecode->position += sizeof(arg_t);
    }

    return buffer;
}

static void print_listing_head(FILE* file_ptr, const File* input_file, const char* output_file_name)
{
    assert(file_ptr);
    assert(input_file);
    assert(output_file_name);

    fprintf(file_ptr, "══════════════════════════════════════════════\n");

    fprintf(file_ptr, "  Date: %s\n", get_current_date_str());
    fprintf(file_ptr, "  Time: %s\n", get_current_time_str());
    fprintf(file_ptr, " Input: %s\n", input_file->file_name);
    fprintf(file_ptr, "Output: %s\n", output_file_name);

    fprintf(file_ptr, "══════════════════════════════════════════════\n");
}

asm_error generate_listing(const char* listing_file_name, const File* input_file, const char* output_file_name, Bytecode* bytecode)
{
    assert(listing_file_name);
    assert(input_file);
    assert(output_file_name);
    assert(bytecode);

    FILE* file_ptr = fopen(listing_file_name, "w");
    if (!file_ptr)
        return ASM_FILE_OPEN_ERR;

    print_listing_head(file_ptr, input_file, output_file_name);

    bytecode->position = 0;
    for (size_t i = 0; i < input_file->line_amount; i++)
    {
        // Print line index
        fprintf(file_ptr, "%-*d", LINE_IDX_PADDING, i + 1);
        const size_t line_len = get_line_len(input_file->lines_ptrs[i].start);

        // Skip line if empty
        if (line_len == 1)
        {
            fprintf(file_ptr, "\n");
            continue;
        }
        // Print position
        fprintf(file_ptr, "%0*llX", AMOUNT_OF_LEADING_ZEROES, bytecode->position);

        fprintf(file_ptr, "%*s", SPACE_AMOUNT_AFTER_POSITION, "");

        // Parse label in line
        line* line_ptr = &input_file->lines_ptrs[i];

        size_t op_name_len = 0;
        wchar_t* op_name_str = get_word(line_ptr->start, &op_name_len);
        line op_name =  {.start = op_name_str, .len = op_name_len};

        if (parse_label(&op_name, line_ptr))
        {
            fprintf(file_ptr, "%*s", BYTECODE_PADDING, ""); // Add padding
            fprintf(file_ptr, "%ls\n", move_to_non_space_sym(input_file->lines_ptrs[i].start));
            continue;
        }
        // Print bytecode
        fprintf(file_ptr, "%-*s", BYTECODE_PADDING, format_bytecode_string(bytecode));

        // Print original line without leading spaces
        fprintf(file_ptr, "%ls\n", move_to_non_space_sym(input_file->lines_ptrs[i].start));
    }

    const int f_close_ret_val = fclose(file_ptr);
    file_ptr = NULL;

    if (f_close_ret_val)
        return ASM_FILE_CLOSE_ERR;

    return ASM_NO_ERR;
}
