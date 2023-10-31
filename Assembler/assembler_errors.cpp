#include <string.h>
#include <stdio.h>
#include <cwchar>
#include <assert.h>

#include "../Libs/textlib.h"
#include "../Libs/colors.h"
#include "../Libs/logs.h"
#include "../common.h"
#include "assembler.h"
#include "assembler_errors.h"

void emit_cmd_error(Command_error* cmd_err, cmd_error error, const wchar_t* source, const size_t len)
{
    cmd_err->err_str_ptr = source;
    cmd_err->err_str_len = (int) len;
    cmd_err->err_id      = error;
}

void print_error_position(const size_t line_idx)
{
    fprintf(stderr, PAINT_TEXT(COLOR_WHITE, "input_file: line %lld: "), line_idx);
}

void print_asm_error(Command_error* cmd_err)
{
    assert(cmd_err);

    #define DEF_CMD_ERR(NAME, FORMAT, ...)\
    case NAME:\
    {\
        print_error_position(cmd_err->line_idx);\
        fprintf(stderr, PAINT_TEXT(COLOR_LIGHT_RED, "error: "));\
        fprintf(stderr, FORMAT, cmd_err->err_str_len, cmd_err->err_str_ptr);\
        fprintf(stderr, "\n");\
        break;\
    }

    switch (cmd_err->err_id)
    {
        #include "cmd_errs.h"

        default:
        {
            print_error_position(cmd_err->line_idx);
            fprintf(stderr, PAINT_TEXT(COLOR_LIGHT_RED, "undefined error"));
            break;
        }
    }
    #undef DEF_CMD_ERR
}

void print_errors(Compiler_errors* compiler_errors)
{
    for (size_t i = 0; i < compiler_errors->errors_amount; i++)
        print_asm_error(compiler_errors->errors + i);
}

void emit_asm_error(Compiler_errors* compiler_errors, Command_error* err, cmd_error error_id)
{
    if (compiler_errors->errors_amount < MAX_ERRORS_AMOUNT)
    {
        size_t err_idx = compiler_errors->errors_amount;

        compiler_errors->errors[err_idx].line_idx    = err->line_idx;
        compiler_errors->errors[err_idx].err_str_ptr = err->err_str_ptr;
        compiler_errors->errors[err_idx].err_str_len = err->err_str_len;
        compiler_errors->errors[err_idx].err_id      = error_id;
        compiler_errors->errors_amount++;
    }
}
