#include <string.h>
#include <stdio.h>
#include <cwchar>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>

#include "../Libs/textlib.h"
#include "../Libs/colors.h"
#include "../Libs/logs.h"
#include "../common.h"
#include "assembler.h"

#define ASM_DEBUG
#ifdef ASM_DEBUG
    #define ASM_DEBUG_MSG(...) DEBUG_MSG(COLOR_YELLOW, __VA_ARGS__)
    #define ASM_ERROR_MSG(...) DEBUG_MSG(COLOR_RED, __VA_ARGS__)
#else
    #define ASM_DEBUG_MSG(...)
    #define ASM_ERROR_MSG(...)
#endif

const int ARG_POISON_VALUE = INT_MAX;

enum cmd_error
{
    CMD_NO_ERR,
    CMD_EMPTY_LINE,
    CMD_LABEL_LINE,
    CMD_REPEATED_MARK_ERR,
    CMD_WRONG_NAME_ERR,
    CMD_WRONG_ARG_ERR,
    CMD_TOO_LONG_MARK_ERR
};

struct Command
{
    int cmd_id;
    int reg_id;
    int imm;
    uint8_t args_bitmask;

    bool has_ram;
    bool is_ram_opened;
    bool is_ram_ended;
    bool has_reg;
    bool has_imm;
};

const size_t MAX_LABEL_NAME_LENGTH = 20;
struct Label
{
    int op_id = 0;
    wchar_t name[MAX_LABEL_NAME_LENGTH] = {};
};

const size_t LABELS_MAX_AMOUNT = 16;
struct Labels
{
    Label labels_arr[LABELS_MAX_AMOUNT] = {};
    size_t amount = 0;
};

static cmd_error emit_code(int* code_array, size_t* position, Command* cmd)
{
    assert(code_array);
    assert(position);

    int cmd_code = cmd->cmd_id | (ARG_IMM_MASK * cmd->has_imm) | (ARG_REG_MASK * cmd->has_reg) | (ARG_RAM_MASK * cmd->has_ram);

    bool is_correct_args = false;
    ASM_DEBUG_MSG("cmd->args_bitmask = %d", cmd->args_bitmask);
    for (size_t i = 0; i < ARGS_COMBINATIONS_AMOUNT; i++)
    {
        if (cmd->args_bitmask & (1 << i))
        {
            if ((cmd_code >> ARGS_MASK_OFFSET) == args_combinations_arr[i])
            {
                is_correct_args = true;
                break;
            }
        }
    }
    if (!is_correct_args)
        return CMD_WRONG_ARG_ERR;

    *(cmd_t*)((uint8_t*) code_array + *position) = cmd_code;
    *position += sizeof(cmd_t);

    if (cmd->has_reg)
    {
        *(arg_t*)((uint8_t*) code_array + *position) = cmd->reg_id;
        *position += sizeof(arg_t);
    }
    if (cmd->has_imm)
    {
        ASM_DEBUG_MSG("cmd->imm = %d", cmd->imm);
        *(arg_t*)((uint8_t*) code_array + *position) = cmd->imm;
        *position += sizeof(arg_t);
    }
    return CMD_NO_ERR;
}

static void remove_comment(line* line_ptr)
{
    wchar_t* comment_sym = wcschr(line_ptr->start, L';');
    if (comment_sym)
        *comment_sym = L'\0';
}

// #define CHAR_TO_ZERO_AND_BACK(POINTER, ...)\
//     do {
//         const wchar_t TAKEN_CHAR = *POINTER;\
//         *POINTER = L'\0';\
//         __VA_ARGS__;\
//         *POINTER = TAKEN_CHAR;\
//     } while (0)

static cmd_error get_arg(wchar_t* arg_start_ptr, Command* cmd, Labels* labels)
{
    size_t arg_len = 0;
    wchar_t* arg_ptr = get_word(arg_start_ptr, &arg_len);
    ASM_DEBUG_MSG("arg_len = %lld", arg_len);

    if (arg_len == 0)
    {
        return CMD_WRONG_ARG_ERR;
    }

    // CHAR_TO_ZERO_AND_BACK();
    const wchar_t delim_char = arg_ptr[arg_len];
    arg_ptr[arg_len] = L'\0';

    bool arg_read = false;
    wchar_t reg_id[2] = {};
    int imm = 0;
    if ((arg_len == 3) && (swscanf(arg_ptr, L"r%1[a-d]x", &reg_id)))
    {
        ASM_DEBUG_MSG("Reg\n");
        cmd->reg_id = reg_id[0] - L'a' + 1;
        cmd->has_reg = true;
        arg_read = true;
    }
    else if (swscanf(arg_ptr, L"%d", &imm))
    {
        ASM_DEBUG_MSG("Imm\n");
        cmd->has_imm = true;
        cmd->imm = imm;
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
        return CMD_WRONG_ARG_ERR;

    return CMD_NO_ERR;
}

// static cmd_error parse_possible_parse_combinations()

static cmd_error parse_args(const int args_bitmask, wchar_t* op_ptr, Command* cmd, Labels* labels)
{
    assert(cmd);

    if (args_bitmask == 1)
        return CMD_NO_ERR;

    ASM_DEBUG_MSG("%ls", op_ptr);
    size_t left_br_pos = 0;
    size_t right_br_pos = 0;

    swscanf(op_ptr, L" [%n%*[^]]%n ", &left_br_pos, &right_br_pos);
    if (right_br_pos)
    {
        ASM_DEBUG_MSG("cmd->has_ram = true");
        cmd->has_ram = true;
    }

    if (cmd->has_ram)
    {
        op_ptr[left_br_pos - 1] = L'\0';
        op_ptr[right_br_pos] = L'\0';
        ASM_DEBUG_MSG("%ls", op_ptr + left_br_pos);
    }

    cmd_error err = CMD_NO_ERR;
    err = get_arg(op_ptr + left_br_pos, cmd, labels);
    if (err != CMD_NO_ERR)
        return err;

    size_t operator_pos = wcscspn(op_ptr + left_br_pos, L"+");
    if (operator_pos == wcslen(op_ptr + left_br_pos))
        operator_pos = 0;

    ASM_DEBUG_MSG("%ls", op_ptr + left_br_pos);
    ASM_DEBUG_MSG("operator_pos = %lld", operator_pos);

    if (operator_pos)
    {
        err = get_arg(op_ptr + left_br_pos + operator_pos + 1, cmd, labels);
    }

    // BAH: Check for trash
    if (cmd->has_ram)
    {
        op_ptr[left_br_pos - 1] = L'[';
        op_ptr[right_br_pos] = L']';
        ASM_DEBUG_MSG("%ls", op_ptr + left_br_pos - 1);
    }

    return err;
}

static cmd_error emit_label(wchar_t* label_name_ptr, const size_t op_name_len, const size_t position, Labels* labels)
{
    if (op_name_len > MAX_LABEL_NAME_LENGTH - 1)
        return CMD_TOO_LONG_MARK_ERR;

    for (size_t id = 0; id < labels->amount; id++)
    {
        if (wcsncmp(labels->labels_arr[id].name, label_name_ptr, op_name_len - 1) == 0)
        {
            labels->labels_arr[id].op_id = (int) position;
            return CMD_NO_ERR;
        }
    }
    return CMD_WRONG_ARG_ERR;
}

static cmd_error parse_line_to_command(Command* cmd, line* line_ptr, const size_t position, Labels* labels)
{
    size_t op_name_len = 0;
    wchar_t* op_name = get_word(line_ptr->start, &op_name_len);

    if (op_name_len == 0)
    {
        ASM_DEBUG_MSG("Empty line!\n");
        return CMD_EMPTY_LINE;
    }

    ASM_DEBUG_MSG("==================================");
    #define DEF_CMD(NAME, ARGS_BITMASK, ...)\
        do {\
            if (wcsncmp(op_name, L ## #NAME, op_name_len) == 0)\
            {\
                const int id = OPERATIONS[NAME ## _enum].id;\
                cmd->cmd_id = id;\
                cmd->args_bitmask = ARGS_BITMASK;\
                cmd_error parse_args_ret_val = parse_args(ARGS_BITMASK, op_name + op_name_len, cmd, labels);\
                ASM_DEBUG_MSG("parse_args = %d\n", parse_args_ret_val);\
                ASM_DEBUG_MSG("code: ram %d id %d reg %d imm %d\n", cmd->has_ram, cmd->cmd_id, cmd->reg_id, cmd->imm);\
                \
                return parse_args_ret_val;\
            }\
        } while(0);
    #include "../commands.h"
    #undef DEF_CMD

    ASM_DEBUG_MSG("%ls", line_ptr->start);
    wchar_t* mark_end_ptr = wcschr(line_ptr->start, L':');
    if (mark_end_ptr && *move_to_non_space_sym(mark_end_ptr + 1) == L'\0')
    {
        emit_label(op_name, op_name_len, position, labels);
        ASM_DEBUG_MSG("LABEL LINE");
        return CMD_LABEL_LINE;
    }

    ASM_ERROR_MSG("NO CMD NAME\n");

    return CMD_WRONG_NAME_ERR;
}

static cmd_error set_labels_names(File* file, Labels* labels)
{
    for (size_t line_idx = 0; line_idx < file->line_amounts; line_idx++)
    {
        line* line_ptr = file->lines_ptrs + line_idx;
        remove_comment(line_ptr);

        size_t op_name_len = 0;
        wchar_t* op_name = get_word(line_ptr->start, &op_name_len);

        if (op_name_len)
        {
            if (op_name[op_name_len - 1] == ':')
            {
                for (size_t label_idx = 0; label_idx < labels->amount; label_idx++)
                {
                    if (wcsncmp(labels->labels_arr[label_idx].name, op_name, op_name_len - 1) == 0)
                        return CMD_REPEATED_MARK_ERR;
                }
                wcsncpy(labels->labels_arr[labels->amount].name, op_name, op_name_len - 1);
                labels->amount++;
            }
        }
    }
    return CMD_NO_ERR;
}

static int* parse_file_to_commands(File* file, size_t* position, int* code_array, Labels* labels)
{
    assert(file);
    assert(position);
    for (size_t i = 0; i < file->line_amounts; i++)
    {
        line* line_ptr = file->lines_ptrs + i;
        Command cmd = {};
        cmd_error err = CMD_NO_ERR;

        err = parse_line_to_command(&cmd, line_ptr, *position, labels);
        if (err == CMD_NO_ERR)
            emit_code(code_array, position, &cmd);
        else if (err > 3)
            ASM_ERROR_MSG("ERROR!!!\n");
    }
    return code_array;
}

asm_error write_bytecode_to_file(const char* output_file_name, int* code_array, const size_t size)
{
    assert(output_file_name);
    assert(code_array);
    assert(size);

    FILE* file_ptr = fopen(output_file_name, "wb");
    if (!file_ptr)
        return FILE_OPEN_ERR;

    fwrite(code_array, sizeof(uint8_t), size, file_ptr);

    int ret_val = fclose(file_ptr);
    file_ptr = NULL;

    if (ret_val)
        return FILE_CLOSE_ERR;

    return NO_ERR;
}

asm_error text_to_asm(File* input_file, const char* output_file_name)
{
    assert(input_file);
    assert(output_file_name);

    tokenize_lines(input_file);
    const size_t line_amount = input_file->line_amounts;

    const size_t max_arg_len = (sizeof(cmd_t) > sizeof(arg_t)) ? sizeof(cmd_t) : sizeof(arg_t);
    int* code_array = (int*) calloc(line_amount * MAX_ARGS_AMOUNT, max_arg_len);

    Labels labels = {};

    set_labels_names(input_file, &labels);

    size_t size = 0;
    parse_file_to_commands(input_file, &size, code_array, &labels);
    size = 0;
    parse_file_to_commands(input_file, &size, code_array, &labels);

    write_bytecode_to_file(output_file_name, code_array, size);

    return NO_ERR;
}
