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

const size_t MAX_ERR_STR_LENGTH = 25;
struct Command
{
    arg_t cmd_id;
    arg_t reg_id;
    arg_t imm;
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
            if ((cmd_code >> ARGS_MASK_OFFSET) == args_combinations_arr[i])
            {
                return true;
                break;
            }
        }
    }
    return false;
}

#define EMIT_BYTECODE(CODE, TYPE)\
    do {\
    *(TYPE*)((uint8_t*) code_array + *position) = CODE;\
    *position += sizeof(TYPE);\
    } while (0)

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
#undef EMIT_ARG
#undef EMIT_BYTE_CODE

static cmd_error emit_label(const wchar_t* label_name_ptr, const size_t label_name_len, const size_t position, Labels* labels)
{
    assert(label_name_ptr);
    assert(labels);

    if (label_name_len > MAX_LABEL_NAME_LENGTH - 1)
    {
        return CMD_TOO_LONG_LABEL_ERR;
    }

    for (size_t label_id = 0; label_id < labels->amount; label_id++)
    {
        if (wcsncmp(labels->labels_arr[label_id].name, label_name_ptr, label_name_len - 1) == 0)
        {
            labels->labels_arr[label_id].op_id = (int) position;
            return CMD_NO_ERR;
        }
    }

    if (labels->final_size == 0)
    {
        wcsncpy(labels->labels_arr[labels->amount].name, label_name_ptr, label_name_len - 1);
        labels->amount++;
        return CMD_NO_ERR;
    }

    return CMD_REPEATED_LABEL_ERR;
}

static cmd_error get_arg(Command* cmd, Labels* labels, Command_error* cmd_err, wchar_t* arg_start_ptr)
{
    assert(arg_start_ptr);
    assert(cmd);
    assert(labels);
    assert(cmd_err);

    size_t arg_len = 0;
    wchar_t* arg_ptr = get_word(arg_start_ptr, &arg_len);
    LOG_TRACE("arg_ptr = %ls", arg_ptr);
    LOG_TRACE("arg_len = %lld", arg_len);

    if (arg_len == 0)
    {
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, TOO_FEW_ARGS_ERR);
    }

    // BAH: Macro for this?
    const wchar_t delim_char = arg_ptr[arg_len];
    arg_ptr[arg_len] = L'\0';

    bool arg_read = false;
    const size_t REG_MARK_LEN = 1;
    wchar_t reg_id[REG_MARK_LEN + 1] = {};

    int imm = 0;
    if (!cmd->has_reg && (arg_len == 3) && (swscanf(arg_ptr, L"r%1[a-d]x", &reg_id)))
    {
        LOG_TRACE("Argument type = reg");
        cmd->reg_id = reg_id[0] - L'a' + 1;
        cmd->has_reg = true;
        arg_read = true;
    }
    else if (!cmd->has_imm && swscanf(arg_ptr, L"%d", &imm))
    {
        LOG_TRACE("Argument type = imm");
        cmd->has_imm = true;
        cmd->imm = imm * FLOAT_COEFFICIENT;
        arg_read = true;
    }
    else
    {
        for (size_t id = 0; id < labels->amount; id++)
        {
            if (wcsncmp(arg_ptr, labels->labels_arr[id].name, arg_len) == 0)
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

static cmd_error parse_args(Command* cmd, Labels* labels, Command_error* cmd_err, const int args_bitmask, wchar_t* op_ptr)
{
    assert(op_ptr);
    assert(cmd);
    assert(labels);
    assert(cmd_err);

    if (args_bitmask == 1)
        return CMD_NO_ERR;

    LOG_TRACE("op_ptr = %ls", op_ptr);
    size_t left_br_pos = 0;
    size_t right_br_pos = 0;

    swscanf(op_ptr, L" [%n%*[^]]%n ", &left_br_pos, &right_br_pos);
    if (right_br_pos)
    {
        LOG_TRACE("cmd->has_ram = true");
        cmd->has_ram = true;
    }

    if (cmd->has_ram)
    {
        op_ptr[left_br_pos - 1] = L'\0';
        op_ptr[right_br_pos] = L'\0';
        LOG_TRACE("%ls", op_ptr + left_br_pos);
    }

    cmd_error err = CMD_NO_ERR;
    err = get_arg(cmd, labels, cmd_err, op_ptr + left_br_pos);
    if (err != CMD_NO_ERR)
        return err;

    // Make in loop (while ... < MAX_ARGS_LEN and operator_pos+)
    size_t operator_pos = wcscspn(op_ptr + left_br_pos, L"+");
    if (operator_pos == wcslen(op_ptr + left_br_pos))
        operator_pos = 0;

    LOG_TRACE("op_ptr + left_br_pos = %ls", op_ptr + left_br_pos);
    LOG_TRACE("operator_pos = %lld", operator_pos);

    if (operator_pos)
    {
        err = get_arg(cmd, labels, cmd_err, op_ptr + left_br_pos + operator_pos + 1);
    }

    if (cmd->has_ram)
    {
        op_ptr[left_br_pos - 1] = L'[';
        op_ptr[right_br_pos] = L']';
        LOG_TRACE("%ls", op_ptr + left_br_pos - 1);
    }

    if (get_word(NULL, NULL))
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_TOO_MANY_ARGS);

    return err;
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

    LOG_TRACE("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    #define STRLEN(S) (sizeof(S)/sizeof(S[0] - 1))
    #define DEF_CMD(NAME, ARGS_BITMASK, ...)\
        do {\
            if (wcsncmp(op_name, L ## #NAME, STRLEN(L ## #NAME)) == 0)\
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

    #include "../commands.h"

    #undef DEF_CMD
    #undef STRLEN

// aboba:        +
// aboba :       +
// aboba: dassda X
// : dassda      X
// :

    wchar_t* label_end_ptr = wcschr(line_ptr->start, L':');
    if (label_end_ptr && *move_to_non_space_sym(label_end_ptr + 1) == L'\0')
    {
        cmd_error err = emit_label(op_name, op_name_len, position, labels);
        if (err != CMD_NO_ERR)
        {
            emit_cmd_error(cmd_err, err, op_name, op_name_len - 1);
        }
        LOG_TRACE("LABEL LINE");
        return err;
    }

    emit_cmd_error(cmd_err, CMD_WRONG_NAME_ERR, op_name, op_name_len);
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
    memset(errors, 0, sizeof(Compiler_errors));

    for (size_t i = 0; i < file->line_amount; i++)
    {
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
        if (err != CMD_NO_ERR)
        {
            emit_asm_error(errors, &err_msg, err);
            asm_err = ASM_PARSE_ARGS_ERR;
        }
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

    asm_error asm_err = parse_file_to_commands(input_file, &size, code_array, &labels, &errors);
    if (asm_err != ASM_NO_ERR)
    {
        print_errors(&errors);
        return asm_err;
    }
    LOG_DEBUG("size = %d", size);
    write_bytecode_to_file(output_file_name, code_array, size);

    FREE_AND_NULL(code_array);

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
