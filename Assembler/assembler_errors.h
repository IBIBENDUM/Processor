#ifndef ASSEMBLER_ERRORS_H
#define ASSEMBLER_ERRORS_H

#define EMIT_CMD_ERROR_AND_RETURN_IT(CMD_ERROR_PTR, CMD_ERROR_ID)\
    do {\
        const wchar_t* op_name = OPERATIONS[cmd->cmd_id - 1].name;\
        emit_cmd_error(CMD_ERROR_PTR, CMD_ERROR_ID, op_name, wcslen(op_name));\
        return CMD_ERROR_ID;\
    } while (0)

enum cmd_error
{
    #define DEF_CMD_ERR(NAME, ...) NAME,
    #include "cmd_errs.h"
    #undef DEF_CMD_ERR
};

struct Command_error
{
    size_t line_idx;
    const wchar_t* err_str_ptr;
    int err_str_len;
    cmd_error err_id;
};

const size_t MAX_ERRORS_AMOUNT = 50;
struct Compiler_errors
{
    Command_error errors[MAX_ERRORS_AMOUNT];
    size_t errors_amount;
};

void emit_cmd_error(Command_error* cmd_err, cmd_error error, const wchar_t* source, const size_t len);

void print_error_position(const size_t line_idx);

void print_asm_error(Command_error* cmd_err);

void print_errors(Compiler_errors* compiler_errors);

void emit_asm_error(Compiler_errors* compiler_errors, Command_error* err, cmd_error error_id);

#endif
