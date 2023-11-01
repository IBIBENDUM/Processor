#ifndef ASSEMBLER_ERRORS_H
#define ASSEMBLER_ERRORS_H

// BAH: Look for this
#define EMIT_CMD_ERROR_AND_RETURN_IT(CMD_ERROR_PTR, CMD_ERROR_ID)\
    do {\
        const wchar_t* op_name = OPERATIONS[cmd->cmd_id - 1].name;\
        emit_cmd_error(CMD_ERROR_PTR, CMD_ERROR_ID, op_name, wcslen(op_name));\
        return CMD_ERROR_ID;\
    } while (0)

enum cmd_error
{
    #define DEF_CMD_ERR(NAME, ...) NAME,
    #include "cmd_errs.inc"
    #undef DEF_CMD_ERR
};

struct Command_error
{
    size_t line_idx;
    const wchar_t* err_str_ptr;
    int err_str_len;
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
};
#undef DEF_ASM_ERR

#define DEF_ASM_ERR(NAME) #NAME,
const char * const asm_errors_strs[] =
{
    #include "asm_errors.inc"
};
#undef DEF_ASM_ERR

/**
 * @brief Emit command error to commands' errors array
 *
 * @param cmd_err
 * @param error
 * @param source
 * @param len
 */
void emit_cmd_error(Command_error* cmd_err, cmd_error error, const wchar_t* source, size_t len);

/**
 * @brief Emit compiler error to compiler's errors array
 *
 * @param compiler_errors
 * @param err
 * @param error_id
 */
void emit_asm_error(Compiler_errors* compiler_errors, const Command_error* const err, cmd_error error_id);

/**
 * @brief Print assembler errors that not related to command parsing
 *
 * @param error
 */
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