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

void emit_cmd_error(Command_error* cmd_err, const cmd_error error, const line* err_substring)
{
    *cmd_err = (Command_error)
              {
                  .line_idx           = cmd_err->line_idx,
                  .err_substring      = *err_substring,
                  .err_id             = error
              };
}

void emit_asm_error(Compiler_errors* compiler_errors, const Command_error* const err, const cmd_error error_id)
{
    if (compiler_errors->errors_amount < MAX_ERRORS_AMOUNT)
    {
        size_t err_idx = compiler_errors->errors_amount;
        compiler_errors->errors[err_idx] =  (Command_error)
                                            {
                                                .line_idx    = err->line_idx,
                                                .err_substring = err->err_substring,
                                                .err_id      = error_id
                                            };
        compiler_errors->errors_amount++;
    }
}

void print_asm_error(const enum asm_error error)
{
    if (error != ASM_NO_ERR)
        LOG_ERROR("Assembler error = %s", asm_errors_strs[error]);
    else
        LOG_INFO("%s", asm_errors_strs[error]);
}

static void print_error_position(const size_t line_idx)
{
    fprintf(stderr, PAINT_TEXT(COLOR_WHITE, "input_file: line %lld: "), line_idx);
}

static void print_cmd_error(Command_error* cmd_err)
{
    assert(cmd_err);

    #define DEF_CMD_ERR(NAME, FORMAT, ...)                                   \
    case NAME:                                                               \
    {                                                                        \
        print_error_position(cmd_err->line_idx);                             \
        fprintf(stderr, PAINT_TEXT(COLOR_LIGHT_RED, "error: "));             \
        fprintf(stderr, FORMAT, cmd_err->err_substring.len, cmd_err->err_substring.start); \
        fprintf(stderr, "\n");                                               \
        break;                                                               \
    }

    switch (cmd_err->err_id)
    {
        #include "cmd_errs.inc"

        default:
        {
            print_error_position(cmd_err->line_idx);
            fprintf(stderr, PAINT_TEXT(COLOR_LIGHT_RED, "undefined error"));
            break;
        }
    }
    #undef DEF_CMD_ERR
}

void print_compiler_errors(Compiler_errors* compiler_errors)
{
    for (size_t i = 0; i < compiler_errors->errors_amount; i++)
        print_cmd_error(compiler_errors->errors + i);
}
