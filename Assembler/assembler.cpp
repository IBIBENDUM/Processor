#include <string.h>
#include <stdio.h>
#include <cwchar>
#include <assert.h>

#include "textlib.h"
#include "assembler.h"
#include "commands.h"


void parse_file_to_commands(File* file)
{
    assert(file);

    tokenize_lines(file);
    const wchar_t delim[] = L" ";
    wchar_t* internal_state = NULL;
    wchar_t* str_ptr = file->buffer;
    wchar_t* op_name = wcstok(str_ptr, delim, &internal_state);
    for (size_t i = 0; i < OPERATION_AMOUNT; i++)
    {
        if (!wcscmp(op_name, OPERATIONS[i].name))
        {
            short command_code = OPERATIONS[i].code & COMMAND_MASK;
            wchar_t* arg_ptr = wcstok(NULL, delim, &internal_state);
            int int_arg = 0;
            if (swscanf(arg_ptr, L"%d", &int_arg))
            {
                command_code |= 0b00100000; //TODO: bitmask shift
            }

            printf("%d %d", command_code, int_arg);
        }
    }
}
