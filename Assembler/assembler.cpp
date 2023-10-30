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
    #define LOG_TRACE(...)
    #define ASM_ERROR_MSG(...)
#endif


enum cmd_error
{
    #define DEF_CMD_ERR(NAME, ...) NAME,
    #include "cmd_errs.h"
    #undef DEF_CMD_ERR
};

const size_t MAX_ERR_STR_LENGTH = 25;
struct Command
{
    int cmd_id;
    int reg_id;
    int imm;
    uint8_t args_bitmask;

    bool has_ram;
    bool has_reg;
    bool has_imm;
    bool has_label;

    size_t line;
    const wchar_t* err_str_ptr;
    size_t err_str_len;
};

const size_t MAX_LABEL_NAME_LENGTH = 50;
struct Label
{
    int op_id = 0;
    wchar_t name[MAX_LABEL_NAME_LENGTH] = {};
};

const size_t LABELS_MAX_AMOUNT = 50;
struct Labels
{
    Label labels_arr[LABELS_MAX_AMOUNT] = {};
    size_t amount = 0;
};

static void emit_error_arg(Command* cmd, const wchar_t* source, const size_t len)
{
    cmd->err_str_ptr = source;
    cmd->err_str_len = len;
}

static bool check_args_correctness(const int cmd_code, Command* const cmd)
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

static cmd_error emit_code(void* code_array, size_t* const position, Command* const cmd)
{
    assert(code_array);
    assert(position);

    int cmd_code = cmd->cmd_id | (ARG_IMM_MASK * cmd->has_imm) | (ARG_REG_MASK * cmd->has_reg) | (ARG_RAM_MASK * cmd->has_ram);

    LOG_TRACE("cmd->args_bitmask = %d", cmd->args_bitmask);

    if (!check_args_correctness(cmd_code, cmd))
    {
        emit_error_arg(cmd, OPERATIONS[cmd->cmd_id - 1].name, wcslen(OPERATIONS[cmd->cmd_id - 1].name));
        return CMD_WRONG_ARG_ERR;
    }

    EMIT_BYTECODE(cmd_code, cmd_t);

    if (cmd->has_imm)
        EMIT_BYTECODE(cmd->imm, arg_t);

    if (cmd->has_reg)
        EMIT_BYTECODE(cmd->reg_id, arg_t);

    return CMD_NO_ERR;
}

#undef EMIT_ARG
#undef EMIT_BYTE_CODE

// #define CHAR_TO_ZERO_AND_BACK(POINTER, ...)\
//     do {
//         const wchar_t TAKEN_CHAR = *POINTER;\
//         *POINTER = L'\0';\
//         __VA_ARGS__;\
//         *POINTER = TAKEN_CHAR;\
//     } while (0)

static cmd_error get_arg(wchar_t* arg_start_ptr, Command* cmd, Labels* labels)
{
    assert(arg_start_ptr);
    assert(cmd);
    assert(labels);

    size_t arg_len = 0;
    wchar_t* arg_ptr = get_word(arg_start_ptr, &arg_len);
    LOG_TRACE("arg_len = %lld", arg_len);
    LOG_TRACE("arg_ptr = %ls", arg_ptr);

    if (arg_len == 0)
    {
        emit_error_arg(cmd, OPERATIONS[cmd->cmd_id - 1].name, wcslen(OPERATIONS[cmd->cmd_id - 1].name));
        return TOO_FEW_ARGS_ERR;
    }

    // CHAR_TO_ZERO_AND_BACK();
    const wchar_t delim_char = arg_ptr[arg_len];
    arg_ptr[arg_len] = L'\0';

    bool arg_read = false;
    wchar_t reg_id[2] = {};
    int imm = 0;
    if (!cmd->has_reg && (arg_len == 3) && (swscanf(arg_ptr, L"r%1[a-d]x", &reg_id)))
    {
        LOG_TRACE("Reg");
        cmd->reg_id = reg_id[0] - L'a' + 1;
        cmd->has_reg = true;
        arg_read = true;
    }
    else if (!cmd->has_imm && swscanf(arg_ptr, L"%d", &imm))
    {
        LOG_TRACE("Imm");
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
        emit_error_arg(cmd, OPERATIONS[cmd->cmd_id - 1].name, wcslen(OPERATIONS[cmd->cmd_id - 1].name));
        return CMD_WRONG_ARG_ERR;
    }
    return CMD_NO_ERR;
}

static cmd_error parse_args(const int args_bitmask, wchar_t* op_ptr, Command* cmd, Labels* labels)
{
    assert(op_ptr);
    assert(cmd);
    assert(labels);

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
    err = get_arg(op_ptr + left_br_pos, cmd, labels);
    if (err != CMD_NO_ERR)
        return err;

    size_t operator_pos = wcscspn(op_ptr + left_br_pos, L"+");
    if (operator_pos == wcslen(op_ptr + left_br_pos))
        operator_pos = 0;

    LOG_TRACE("op_ptr + left_br_pos = %ls", op_ptr + left_br_pos);
    LOG_TRACE("operator_pos = %lld", operator_pos);

    if (operator_pos)
    {
        err = get_arg(op_ptr + left_br_pos + operator_pos + 1, cmd, labels);
    }

    if (cmd->has_ram)
    {
        op_ptr[left_br_pos - 1] = L'[';
        op_ptr[right_br_pos] = L']';
        LOG_TRACE("%ls", op_ptr + left_br_pos - 1);
    }

    if (get_word(NULL, NULL))
    {
        emit_error_arg(cmd, OPERATIONS[cmd->cmd_id - 1].name, wcslen(OPERATIONS[cmd->cmd_id - 1].name));
        err = CMD_TOO_MANY_ARGS;
    }
    return err;
}

static cmd_error emit_label(wchar_t* label_name_ptr, const size_t op_name_len, const size_t position, Labels* labels)
{
    assert(label_name_ptr);
    assert(labels);

    if (op_name_len > MAX_LABEL_NAME_LENGTH - 1)
    {
        return CMD_TOO_LONG_LABEL_ERR;
    }

    for (size_t id = 0; id < labels->amount; id++)
    {
        if (wcsncmp(labels->labels_arr[id].name, label_name_ptr, op_name_len - 1) == 0)
        {
            labels->labels_arr[id].op_id = (int) position;
            return CMD_NO_ERR;
        }
    }
    return CMD_WRONG_LABEL_NAME;
}

static cmd_error parse_line_to_command(Command* cmd, line* line_ptr, const size_t position, Labels* labels)
{
    assert(cmd);
    assert(line_ptr);
    assert(labels);

    size_t op_name_len = 0;
    wchar_t* op_name = get_word(line_ptr->start, &op_name_len);

    if (op_name_len == 0)
    {
        LOG_TRACE("Empty line!");
        return CMD_NO_ERR;
    }

    LOG_TRACE("==================================");
    #define DEF_CMD(NAME, ARGS_BITMASK, ...)\
        do {\
            if (wcsncmp(op_name, L ## #NAME, op_name_len) == 0)\
            {\
                const int id = OPERATIONS[NAME ## _enum].id;\
                cmd->cmd_id = id;\
                cmd->args_bitmask = ARGS_BITMASK;\
                cmd_error parse_args_ret_val = parse_args(ARGS_BITMASK, op_name + op_name_len, cmd, labels);\
                LOG_TRACE("parse_args = %d", parse_args_ret_val);\
                LOG_TRACE("code: ram %d id %d reg %d imm %d", cmd->has_ram, cmd->cmd_id, cmd->reg_id, cmd->imm);\
                \
                return parse_args_ret_val;\
            }\
        } while(0);
    #include "../commands.h"
    #undef DEF_CMD

    LOG_TRACE("%ls", line_ptr->start);
    wchar_t* label_end_ptr = wcschr(line_ptr->start, L':');
    if (label_end_ptr && *move_to_non_space_sym(label_end_ptr + 1) == L'\0')
    {
        cmd_error err = emit_label(op_name, op_name_len, position, labels);
        if (err != CMD_NO_ERR)
        {
            emit_error_arg(cmd, op_name, op_name_len - 1);
        }
        LOG_TRACE("LABEL LINE");
        return err;
    }

    emit_error_arg(cmd, line_ptr->start, line_ptr->len);
    return CMD_WRONG_NAME_ERR;
}

static void print_line(const size_t line_idx)
{
    fprintf(stderr, PAINT_TEXT(COLOR_WHITE, "input_file: line %lld: "), line_idx);
}

static void print_asm_error(Command* cmd, cmd_error error)
{
    #define DEF_CMD_ERR(NAME, FORMAT, ...)\
    case NAME:\
    {\
        print_line(cmd->line);\
        fprintf(stderr, PAINT_TEXT(COLOR_LIGHT_RED, "error: "));\
        fprintf(stderr, FORMAT, cmd->err_str_ptr);\
        fprintf(stderr, "\n");\
        fprintf(stderr, TEXT_RESET);\
        break;\
    }

    switch (error)
    {
        #include "cmd_errs.h"

        default:
        {
            print_line(cmd->line);;
            fprintf(stderr, PAINT_TEXT(COLOR_LIGHT_RED, "undefined error"));
            fprintf(stderr, TEXT_RESET);
            break;
        }
    }

    #undef DEF_CMD_ERR
}


static cmd_error set_labels_names(File* file, Labels* labels)
{
    assert(file);
    assert(labels);
    LOG_INFO("Setting labels names...");

    LOG_INFO("file->line_amounts = %lld", file->line_amounts);
    for (size_t line_idx = 0; line_idx < file->line_amounts; line_idx++)
    {
        line* line_ptr = file->lines_ptrs + line_idx;
        replace_with_zero(line_ptr, L';');

        Command cmd = {};

        size_t op_name_len = 0;
        wchar_t* op_name = get_word(line_ptr->start, &op_name_len);

        if (op_name_len)
        {
            // BAH: if line contains only :?
            if (op_name[op_name_len - 1] == ':')
            {
                if (labels->amount < LABELS_MAX_AMOUNT)
                {
                    for (size_t label_idx = 0; label_idx < labels->amount; label_idx++)
                    {
                        if (wcsncmp(labels->labels_arr[label_idx].name, op_name, op_name_len - 1) == 0)
                        {
                            cmd.line = line_idx + 1;
                            emit_error_arg(&cmd, op_name, op_name_len - 1);

                            print_asm_error(&cmd, CMD_REPEATED_LABEL_ERR);
                            return CMD_REPEATED_LABEL_ERR;
                        }
                    }
                    wcsncpy(labels->labels_arr[labels->amount].name, op_name, op_name_len - 1);
                    labels->amount++;
                }
                else
                    return CMD_TOO_MANY_LABEL_ERR;
            }
        }
    }
    return CMD_NO_ERR;
}

static asm_error parse_file_to_commands(File* file, size_t* position, int* code_array, Labels* labels)
{
    assert(file);
    assert(position);
    assert(code_array);
    assert(labels);

    LOG_INFO("Start parsing text to commands");

    asm_error asm_err = ASM_NO_ERR;

    *position = 0;
    for (size_t i = 0; i < file->line_amounts; i++)
    {
        line* line_ptr = file->lines_ptrs + i;
        Command cmd = {};
        cmd.line = i + 1;
        cmd_error err = CMD_NO_ERR;
        err = parse_line_to_command(&cmd, line_ptr, *position, labels);
        if (err == CMD_NO_ERR)
        {
            if (cmd.cmd_id != 0)
            {
                err = emit_code(code_array, position, &cmd);
            }
        }
        if (err != CMD_NO_ERR)
        {
            print_asm_error(&cmd, err);
            asm_err = ASM_PARSE_ARGS_ERR;
        }
    }
    return asm_err;
}

static asm_error write_bytecode_to_file(const char* output_file_name, int* code_array, const size_t size)
{
    assert(output_file_name);
    assert(code_array);
    assert(size);

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

    return ASM_NO_ERR;
}

asm_error text_to_asm(File* input_file, const char* output_file_name)
{
    assert(input_file);
    assert(output_file_name);

    LOG_INFO("Prepare text for converting...");

    tokenize_lines(input_file);
    const size_t line_amount = input_file->line_amounts;

    const size_t max_arg_len = (sizeof(cmd_t) > sizeof(arg_t)) ? sizeof(cmd_t) : sizeof(arg_t);
    arg_t* code_array = (arg_t*) calloc(line_amount * MAX_ARGS_AMOUNT, max_arg_len);

    if (!code_array)
        return ASM_MEM_ALLOC_ERR;

    Labels labels = {};

    // TODO: Remove this
    cmd_error err = set_labels_names(input_file, &labels);
    if (err != CMD_NO_ERR)
        return ASM_PARSE_LABELS_ERR;

    size_t size = 0;
    asm_error asm_err = parse_file_to_commands(input_file, &size, code_array, &labels);
    if (asm_err != ASM_NO_ERR)
        return asm_err;

    LOG_DEBUG("size = %d", size);
    parse_file_to_commands(input_file, &size, code_array, &labels);
    write_bytecode_to_file(output_file_name, code_array, size);

    FREE_AND_NULL(code_array);

    return ASM_NO_ERR;
}
