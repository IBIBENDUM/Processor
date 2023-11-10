#ifndef ASSEMBLER_ERRORS_H
#define ASSEMBLER_ERRORS_H

#include <stdlib.h>

#include "../Libs/textlib.h"

// Emit command error code and return error code
#define EMIT_CMD_ERROR_AND_RETURN_IT(CMD_ERROR_PTR, CMD_ERROR_ID)       \
    do {                                                                \
        wchar_t* op_name = (wchar_t*) OPERATIONS[cmd->cmd_id - 1].name; \
        line err_substring = {op_name, wcslen(op_name)};                \
        emit_cmd_error(CMD_ERROR_PTR, CMD_ERROR_ID, &err_substring);    \
                                                                        \
        return CMD_ERROR_ID;                                            \
    } while (0)

enum cmd_error
{
    #define DEF_CMD_ERR(NAME, ...) NAME,
    #include "cmd_errs.inc"
    // There is undef inside "cmd_errs.inc"
};

struct Command_error
{
    size_t line_idx;
    line err_substring;
    cmd_error err_id;
};

const size_t MAX_ERRORS_AMOUNT = 50; ///< if there are more errors, they will not be recorded
struct Compiler_errors
{
    Command_error errors[MAX_ERRORS_AMOUNT];
    size_t errors_amount;
};

#define DEF_ASM_ERR(NAME) NAME,
enum asm_error
{
    #include "asm_errors.inc"
    // There is undef inside "asm_errors.inc"
};

#define DEF_ASM_ERR(NAME) #NAME,
const char * const asm_errors_strs[] =
{
    #include "asm_errors.inc"
    // There is undef inside "asm_errors.inc"
};

void emit_cmd_error(Command_error* cmd_err, cmd_error error, const line* err_substring);

void emit_asm_error(Compiler_errors* compiler_errors, const Command_error* const err, cmd_error error_id);

void print_asm_error(enum asm_error error);

/**
 * @brief Print compilation file errors
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * input_file: line 7: error: command 'p0p' wasn't declared
 * input_file: line 12: error: no previous 'print' declaration
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * @param compiler_errors
 */
void print_compiler_errors(Compiler_errors* compiler_errors);

#endif
