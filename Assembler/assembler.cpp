#include <string.h>
#include <stdio.h>
#include <cwchar>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>

#include "../Libs/textlib.h"
#include "../commands.h"
#include "assembler.h"

const int ARG_POISON_VALUE = INT_MAX;

void emit_code(int* code_array, size_t* position, const uint8_t command_code, const int arg)
{
    code_array[(*position)++] = command_code;
    code_array[(*position)++] = arg;
}

void parse_line_to_command(line* line_ptr, int* code_array, size_t* position)
{
    const wchar_t delim[]   = L" ";
    wchar_t* internal_state = NULL;
    wchar_t* op_name        = wcstok(line_ptr->start, delim, &internal_state);

    if (!op_name)
    {
        DEBUG_MSG("Empty line!\n");
        return;
    }

    DEBUG_MSG("command: %ls", line_ptr->start);
    // TODO: Remake on switch case with define
    for (size_t op_id = 0; op_id < OPERATION_AMOUNT; op_id++)
    {
        if (wcscmp(op_name, OPERATIONS[op_id].name) == 0)
        {
            uint8_t command_code = OPERATIONS[op_id].id & COMMAND_MASK;
            wchar_t* arg_ptr = wcstok(NULL, delim, &internal_state);
            int arg = 0; //TODO: add 2nd arg
            int reg_id = 0;

            // TODO: Add syntax errors
            if (!arg_ptr)
            {
                emit_code(code_array, position, command_code, ARG_POISON_VALUE);
                DEBUG_MSG("code: %d\n", command_code, reg_id);
            }
            else if (swscanf(arg_ptr, L"%d", &arg))
            {
                command_code |= ARG_FORMAT_IMMED;
                emit_code(code_array, position, command_code, arg);
                DEBUG_MSG("code: %d %d\n", command_code, arg);
            }
            else if (swscanf(arg_ptr, L"r%cx", &reg_id))
            {
                reg_id += 1 - 'a';
                if (1 <= reg_id && reg_id <= REGS_AMOUNT) // Or loop for id comparison?
                {
                    command_code |= ARG_FORMAT_REG;
                    emit_code(code_array, position, command_code, reg_id);
                    DEBUG_MSG("code: %d %d\n", command_code, reg_id);
                }
            }
            return;
        }
    }
}

int* parse_file_to_commands(File* file, size_t* position)
{
    assert(file);
    tokenize_lines(file);
    int* code_array = (int*) calloc(file->line_amounts * 2, sizeof(int)); // Assumed that there is no more than two args
    for (size_t i = 0; i < file->line_amounts; i++)
    {
        parse_line_to_command(file->lines_ptrs + i, code_array, position);
    }
    return code_array;
}

void write_to_file(const char* output_file_name, int* code_array, const size_t size)
{
    FILE* file_ptr = fopen(output_file_name, "wb");

    fwrite(code_array, sizeof(int), size, file_ptr);

    int fclose_ret_val = fclose(file_ptr);
    file_ptr = NULL;
    HANDLE_ERROR(!fclose_ret_val, "Error at closing file");
}
