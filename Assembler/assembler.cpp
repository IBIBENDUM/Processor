#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwchar>
#include <assert.h>
#include <malloc.h>

#include "../Libs/textlib.h"
#include "../Libs/colors.h"
#include "../Libs/logs.h"
#include "../Libs/utils.h"
#include "../common.h"
#include "assembler.h"
#include "assembler_errors.h"

const size_t MAX_ERR_STR_LENGTH = 25; ///< Length of error string argument (e.g. label name)

// NOTE: Sometimes you can immediately match
//       binary representation of a command with
//       binary representation of underlying struct
//       that represents it, like so (you might want to look into bitfields):

// struct Command_ {
//     arg_t cmd_id : 5;

//     union {
//         arg_t reg_id : 3;
//         arg_t imm : 16;
//     }
// };  

struct Command
{
    arg_t cmd_id;
    arg_t reg_id;
    arg_t imm; // TODO: naming, imm_value?
    uint8_t args_bitmask;

    bool has_ram;
    bool has_reg;
    bool has_imm;
    bool has_label;
};

const size_t MAX_LABEL_NAME_LENGTH = 50;
struct Label
{
    arg_t op_id = 0;
    wchar_t name[MAX_LABEL_NAME_LENGTH] = {};
};

const size_t LABELS_MAX_AMOUNT = 50;
struct Labels
{
    // TODO: Don't you have a generic "array"?
    Label labels_arr[LABELS_MAX_AMOUNT] = {};
    size_t amount = 0;

    size_t final_size = 0;
};

static bool check_args_correctness(const int cmd_code, const Command* cmd)
{
    for (size_t i = 0; i < ARGS_COMBINATIONS_AMOUNT; i++)
    {
        if (cmd->args_bitmask & (1 << i))
        {
            if ((cmd_code & (~ID_MASK)) == args_combinations_arr[i])
            {
                return true;
            }
        }
    }
    return false;
}

// TODO: Move macro closer to usage
// Also, maybe this can be a function if you make a struct out of
// code array and position, which seems like a logical solution.

// TODO: indentation?
#define EMIT_BYTECODE(CODE, TYPE)                       \
    do {                                                \
    *(TYPE*)((uint8_t*) code_array + *position) = CODE; \
    *position += sizeof(TYPE);                          \
    } while (0)

// TODO: emit_command?
static cmd_error emit_code(const Command* const cmd, Command_error* const cmd_err, void* code_array, size_t* const position)
{
    assert(code_array);
    assert(cmd);
    assert(position);

    int cmd_code = cmd->cmd_id | (ARG_IMM_MASK * cmd->has_imm) | (ARG_REG_MASK * cmd->has_reg) | (ARG_RAM_MASK * cmd->has_ram);

    if (!check_args_correctness(cmd_code, cmd))
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_WRONG_ARG_ERR);

    EMIT_BYTECODE(cmd_code, cmd_t);

    if (cmd->has_imm)
        EMIT_BYTECODE(cmd->imm, arg_t);

    if (cmd->has_reg)
        EMIT_BYTECODE(cmd->reg_id, arg_t);

    return CMD_NO_ERR;
}
#undef EMIT_BYTE_CODE

static cmd_error emit_label(const wchar_t* label_name_ptr, const size_t label_name_len, const size_t position, Labels* labels)
{
    assert(label_name_ptr);
    assert(labels);

    if (label_name_len > MAX_LABEL_NAME_LENGTH - 1)
    {
        return CMD_TOO_LONG_LABEL_ERR;
    }

    // // Try to find label in labels array
    // for (size_t label_id = 0; label_id < labels->amount; label_id++)
    // {
    //     if (wcsncmp(labels->labels_arr[label_id].name, label_name_ptr, label_name_len - 1) == 0)
    //     {
    //         labels->labels_arr[label_id].op_id = (int) position;
    //         return CMD_NO_ERR;
    //     }
    // }

    // Check for first compilation
    if (labels->final_size == 0)
    {
        wcsncpy(labels->labels_arr[labels->amount].name, label_name_ptr, label_name_len - 1);
        labels->labels_arr[labels->amount].op_id = (int) position;

        labels->amount++;
        return CMD_NO_ERR;
    }

    return CMD_REPEATED_LABEL_ERR;
}

static cmd_error get_arg(Command* cmd, Labels* labels, Command_error* cmd_err, wchar_t* arg_start_ptr)
{
    // TODO: can you make a structured parser out of this thing?
    //       Because right now this function feels like a frankenstein who
    //       has been reassembled multiple times...
    //       Even if it works, it doesn't seem like something maintainable.

    assert(arg_start_ptr);
    assert(cmd);
    assert(labels);
    assert(cmd_err);

    size_t arg_len = 0;
    wchar_t* arg_ptr = get_word(arg_start_ptr, &arg_len);
    LOG_TRACE("arg_ptr = %ls", arg_ptr);
    LOG_TRACE("arg_len = %lld", arg_len);

    if (arg_len == 0)
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, TOO_FEW_ARGS_ERR);

    // BAH: Macro for this?
    const wchar_t delim_char = arg_ptr[arg_len];
    arg_ptr[arg_len] = L'\0';

    bool arg_read = false;
    const size_t REG_MARK_LEN = 1;
    wchar_t reg_id[REG_MARK_LEN + 1] = {};

    wchar_t* strtod_end_ptr = NULL;
    double imm_double = wcstod(arg_ptr, &strtod_end_ptr);
    if (!cmd->has_imm && *strtod_end_ptr == L'\0')
    {
        LOG_INFO("imm_double = %lf", imm_double);
        LOG_TRACE("Argument type = imm");
        cmd->has_imm = true;
        cmd->imm = (int) (imm_double * FLOAT_COEFFICIENT);
        arg_read = true;
    }
    else if (!cmd->has_reg && (arg_len == 3) && (swscanf(arg_ptr, L"r%1[a-d]x", &reg_id)))
    {
        LOG_TRACE("Argument type = reg");
        cmd->reg_id = reg_id[0] - L'a' + 1;
        cmd->has_reg = true;
        arg_read = true;
    }
    else
    {
        for (size_t id = 0; id < labels->amount; id++)
        {
            const wchar_t* label_name = labels->labels_arr[id].name;
            if (wcsncmp(arg_ptr, label_name, wcslen(label_name)) == 0)
            {
                cmd->has_imm = true;
                cmd->imm = labels->labels_arr[id].op_id;
                arg_read = true;
            }
        }
    }
    arg_ptr[arg_len] = delim_char;

    if (!arg_read)
    {
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_WRONG_ARG_ERR);
    }
    return CMD_NO_ERR;
}

// TODO: what is garbage args? Add an explanation
#define RETURN_ERR_IF_GARBAGE_ARGS(START_PTR)                         \
    do {                                                              \
        size_t GARBAGE_WORD_LEN = 0;                                  \
        get_word(START_PTR, &GARBAGE_WORD_LEN);                       \
        if (GARBAGE_WORD_LEN)                                         \
            EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_TOO_MANY_ARGS); \
    } while (0)

static cmd_error parse_args(Command* cmd, Labels* labels, Command_error* cmd_err, const int args_bitmask, wchar_t* op_ptr)
{
    // TODO: the same thing I wrote for previous parser...

    assert(op_ptr);
    assert(cmd);
    assert(labels);
    assert(cmd_err);

    if (args_bitmask == 1)
        return CMD_NO_ERR;

    // TODO: WHAT IS CSCSPN??
    ssize_t left_br_pos = cscspn(op_ptr, L"[");
    ssize_t right_br_pos = cscspn(op_ptr, L"]");

    // BAH: Make macro or something?
    if (left_br_pos > 0 && right_br_pos > 0)
    {
        cmd->has_ram = true;
        op_ptr[left_br_pos] = L'\0';
        op_ptr[right_br_pos] = L'\0';
    }

    cmd_error err = CMD_NO_ERR;
    err = get_arg(cmd, labels, cmd_err, op_ptr + left_br_pos + 1);
    if (err != CMD_NO_ERR)
        return err;

    // Find + operator
    size_t operator_pos = cscspn(op_ptr + left_br_pos + 1, L"+");

    // Get second argument
    if (operator_pos)
        err = get_arg(cmd, labels, cmd_err, op_ptr + left_br_pos + operator_pos + 2);

    if (cmd->has_ram)
    {
        op_ptr[left_br_pos] = L'[';
        op_ptr[right_br_pos] = L']';

        // Check for garbage args inside brackets
        RETURN_ERR_IF_GARBAGE_ARGS(NULL); // TODO: What is NULL doing here?
                                                 //       It's not obvious that this has state at all.

        // Check for garbage args outside brackets
        RETURN_ERR_IF_GARBAGE_ARGS(op_ptr + right_br_pos + 1);
    }
    else
    {
        // Check for garbage if no RAM
        RETURN_ERR_IF_GARBAGE_ARGS(NULL);
    }
    return err;
}
#undef RETURN_ERR_IF_GARBAGE_ARGS

static bool parse_label(const line* line_ptr, wchar_t* op_name, const size_t op_name_len)
{
    bool is_label = false;
    if (op_name_len > 1)
    {
        // Check for colon existence in line
        size_t colon_pos = wcscspn(line_ptr->start, L":");
        if (colon_pos != line_ptr->len)
        {
            // Because op_name contains first word we can check only two cases
            // Check for 'label:' case
            if (op_name[op_name_len-1] == L':')
                is_label = true;
            // Check for 'label :' case
            else if (*move_to_non_space_sym(op_name + op_name_len) == L':')
                is_label = true;

            // Check for 'label: garbage' case
            if (*move_to_non_space_sym(line_ptr->start + colon_pos + 1) != L'\0')
                is_label = false;
        }
    }
    return is_label;
}

static cmd_error parse_line_to_command(Command* cmd, Labels* labels, Command_error* cmd_err, const line* line_ptr, const size_t position)
{
    assert(cmd);
    assert(line_ptr);
    assert(labels);

    size_t op_name_len = 0;
    wchar_t* op_name = get_word(line_ptr->start, &op_name_len);
    LOG_DEBUG("op_name = %.*ls", op_name_len, op_name);
    if (op_name_len == 0)
    {
        LOG_TRACE("Empty line!");
        return CMD_NO_ERR;
    }

    // BAH: Or loop?
    // TODO: Overcomplex macro, probably can be extracted partially into function
    // TODO: Also, "\"... align them
    #define STRLEN(S) (sizeof(S)/sizeof(S[0]) - 1)
    #define DEF_CMD(NAME, ARGS_BITMASK, ...)\
        do {\
            if (op_name_len == STRLEN(L ## #NAME) && wcsncmp(op_name, L ## #NAME, STRLEN(L ## #NAME)) == 0)\
            {\
                const int id = OPERATIONS[NAME ## _enum].id;\
                cmd->cmd_id = id;\
                cmd->args_bitmask = ARGS_BITMASK;\
                cmd_error parse_args_ret_val = parse_args(cmd, labels, cmd_err, ARGS_BITMASK, op_name + op_name_len);\
                LOG_TRACE("parse_args = %d", parse_args_ret_val);\
                LOG_TRACE("code: ram %d id %d reg %d imm %d", cmd->has_ram, cmd->cmd_id, cmd->reg_id, cmd->imm);\
                \
                return parse_args_ret_val;\
            }\
        } while(0);
    //            ^ TODO: Do while only makes sense if you don't put semicolon in the end 

    #include "../commands.inc"

    #undef DEF_CMD
    #undef STRLEN
    // TODO: can you move #undef's to commands.inc?

    if (parse_label(line_ptr, op_name, op_name_len))
    {
        cmd_error err = emit_label(op_name, op_name_len, position, labels);
        if (err != CMD_NO_ERR) // TODO: you made a lot of macros for such cases but not this one, why? In this case it can even be a function
        {
            emit_cmd_error(cmd_err, err, move_to_non_space_sym(line_ptr->start), line_ptr->len - 1);
        }
        LOG_TRACE("LABEL LINE");
        return err;
    }
    emit_cmd_error(cmd_err, CMD_WRONG_NAME_ERR, move_to_non_space_sym(line_ptr->start), line_ptr->len - 1);
    return CMD_WRONG_NAME_ERR;
}

static asm_error parse_file_to_commands(File* file, size_t* position, int* code_array, Labels* labels, Compiler_errors* errors)
{
    assert(file);
    assert(position);
    assert(code_array);
    assert(labels);

    LOG_INFO("Start parsing file to commands...");

    asm_error asm_err = ASM_NO_ERR;

    *position = 0;

    // TODO: *errors = {}; ?
    memset(errors, 0, sizeof(Compiler_errors));

    for (size_t i = 0; i < file->line_amount; i++)
    {
        LOG_TRACE("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        LOG_DEBUG("line = %d", i + 1);
        line* line_ptr = file->lines_ptrs + i;
        replace_with_zero(line_ptr, L';'); // Remove comments

        Command cmd = {};
        Command_error err_msg = { .line_idx = i + 1 };
        cmd_error err = CMD_NO_ERR;
        err = parse_line_to_command(&cmd, labels, &err_msg, line_ptr, *position);
        if (err == CMD_NO_ERR)
        {
            if (cmd.cmd_id != 0)
            {
                err = emit_code(&cmd, &err_msg, code_array, position);
            }
        }
        if (err != CMD_NO_ERR) // TODO: seems like a pattern I've already seen
        {
            emit_asm_error(errors, &err_msg, err);
            asm_err = ASM_PARSE_ARGS_ERR;
        }

        LOG_DEBUG("position = %d", *position);
    }
    labels->final_size = labels->amount;

    LOG_INFO("File parsed!");

    return asm_err;
}

static asm_error write_bytecode_to_file(const char* output_file_name, int* code_array, const size_t size)
{
    assert(output_file_name);
    assert(code_array);
    assert(size);

    LOG_INFO("Writing bytecode to file...");

    // BAH: what if wrong folder name?
    FILE* file_ptr = fopen(output_file_name, "wb");
    if (!file_ptr)
        return ASM_FILE_OPEN_ERR;

    size_t amount_of_written = fwrite(code_array, sizeof(uint8_t), size, file_ptr);
    if (amount_of_written < size)
        return ASM_WRITE_ERR;

    int f_close_ret_val = fclose(file_ptr);
    file_ptr = NULL;

    if (f_close_ret_val)
        return ASM_FILE_CLOSE_ERR;

    LOG_INFO("Bytecode has been written!");

    return ASM_NO_ERR;
}

static asm_error text_to_asm(File* input_file, const char* output_file_name)
{
    assert(input_file);
    assert(output_file_name);

    LOG_INFO("Prepare text for converting...");

    tokenize_lines(input_file);
    const size_t line_amount = input_file->line_amount;

    // Find maximum type len and reserve place for the largest
    const size_t max_arg_len = (sizeof(cmd_t) > sizeof(arg_t)) ? sizeof(cmd_t) : sizeof(arg_t);
    arg_t* code_array = (arg_t*) calloc(line_amount * MAX_ARGS_AMOUNT, max_arg_len);

    if (!code_array)
        return ASM_MEM_ALLOC_ERR;

    Labels labels = {};
    Compiler_errors errors = {};
    size_t size = 0;

    parse_file_to_commands(input_file, &size, code_array, &labels, &errors);
    LOG_DEBUG("labels.labels_arr[0].op_id = %d", labels.labels_arr[0].op_id);
    LOG_DEBUG("labels.labels_arr[0].name = %ls", labels.labels_arr[0].name);
    asm_error asm_err = parse_file_to_commands(input_file, &size, code_array, &labels, &errors);
    if (asm_err != ASM_NO_ERR)
    {
        print_compiler_errors(&errors);
        return asm_err;
    }
    LOG_DEBUG("size = %d", size);
    write_bytecode_to_file(output_file_name, code_array, size);

    free_and_null(code_array);

    return ASM_NO_ERR;
}

asm_error file_to_asm(const char* input_file_name, const char* output_file_name)
{
    assert(input_file_name);
    assert(output_file_name);

    File input_file = {};
    if (!init_file(input_file_name, &input_file))
    {
        LOG_ERROR("Can't read file");
        return ASM_INPUT_FILE_READ_ERR;
    }

    asm_error err = text_to_asm(&input_file, output_file_name);
    destruct_file(&input_file);

    return err;
}
