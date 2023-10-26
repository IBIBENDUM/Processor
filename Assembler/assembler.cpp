#include <string.h>
#include <stdio.h>
#include <cwchar>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>

#include "../Libs/textlib.h"
#include "../Libs/colors.h"
#include "../common.h"
#include "assembler.h"


// BAH: remake this
#define ASM_DEBUG

#ifdef ASM_DEBUG
    #define ASM_DEBUG_MSG(...) DEBUG_MSG(__VA_ARGS__)
#else
    #define ASM_DEBUG_MSG(...)
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

    bool has_ram;
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

static void emit_code(int* code_array, size_t* position, Command* cmd)
{
    assert(code_array);
    assert(position);

    // BAH: Make uneven code
    code_array[(*position)++] = cmd->cmd_id;

    // ASM_DEBUG_MSG("cmd_id = <%ls>", cmd->cmd_id);
    if (cmd->has_reg)
        code_array[(*position)++] = cmd->reg_id;

    if (cmd->has_imm)
        code_array[(*position)++] = cmd->imm;
}

static void remove_comment(line* line_ptr)
{
    wchar_t* comment_sym = wcschr(line_ptr->start, L';');
    if (comment_sym)
        *comment_sym = L'\0';
}

static cmd_error get_arg(Command* cmd, const int args_bitmask, Labels* labels)
{
    size_t arg_len = 0;
    wchar_t* arg_ptr = get_word(NULL, &arg_len);
    ASM_DEBUG_MSG("arg_len = %lld", arg_len);

    if (arg_len == 0)
    {
        return CMD_WRONG_ARG_ERR;
    }

    const wchar_t delim_char = arg_ptr[arg_len];
    arg_ptr[arg_len] = L'\0';

    bool arg_read = false;
    wchar_t reg_id[2] = {};
    int imm = 0;

    // BAH: Make read not by scanf
    if ((args_bitmask & (ARG_REG_MASK >> ARGS_MASK_OFFSET)) && (arg_len == 3) && (swscanf(arg_ptr, L"r%1[a-d]x", &reg_id)))
    {
        ASM_DEBUG_MSG("Reg\n");
        cmd->reg_id = reg_id[0] - L'a' + 1;
        cmd->has_reg = true;
        arg_read = true;
    }
    else if ((args_bitmask & (ARG_IMM_MASK >> ARGS_MASK_OFFSET)) && swscanf(arg_ptr, L"%d", &imm))
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

static cmd_error parse_args(const int args_bitmask, Command* cmd, Labels* labels)
{
    assert(cmd);

    if (!args_bitmask)
        return CMD_NO_ERR;

    //BAH: Remake for any count of args
    cmd_error get_arg_ret_val = get_arg(cmd, args_bitmask, labels);
    if (get_arg_ret_val != CMD_NO_ERR)
        return get_arg_ret_val;

    size_t operation_len = 0;
    wchar_t* operation_ptr = get_word(NULL, &operation_len);

    if (operation_len == 0)
    {
        return CMD_NO_ERR;
    }
    if (operation_len == 1 && operation_ptr[0] == '+')
    {
        get_arg_ret_val = get_arg(cmd, args_bitmask, labels);
        if (get_arg_ret_val == CMD_NO_ERR)
        {
            size_t trash_len = 0;
            wchar_t* trash_ptr = get_word(NULL, &trash_len);
            if (trash_len == 0)
                return get_arg_ret_val;
        }
    }
    return CMD_WRONG_ARG_ERR;
}

static cmd_error emit_label(wchar_t* label_name_ptr, const size_t op_name_len, const size_t position, Labels* labels)
{
    if (op_name_len > MAX_LABEL_NAME_LENGTH - 1)
        return CMD_TOO_LONG_MARK_ERR;

    for (size_t id = 0; id < labels->amount; id++)
    {
        if (wcsncmp(labels->labels_arr[id].name, label_name_ptr, op_name_len - 1) == 0)
        {
            labels->labels_arr[id].op_id = (int) position + 1;
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

    // BAH: make by strcspn?
    if (op_name[op_name_len - 1] == ':')
    {
        emit_label(op_name, op_name_len, position, labels);
        return CMD_LABEL_LINE;
    }

    ASM_DEBUG_MSG("==================================");
    #define DEF_CMD(NAME, ARGS_BITMASK, ...)\
        do {\
            if (wcsncmp(op_name, L ## #NAME, op_name_len) == 0)\
            {\
                const int id = OPERATIONS[NAME ## _enum].id;\
                cmd->cmd_id = id;\
                cmd_error parse_args_ret_val = parse_args(ARGS_BITMASK, cmd, labels);\
                ASM_DEBUG_MSG("parse_args = %d\n", parse_args_ret_val);\
                ASM_DEBUG_MSG("code: ram %d id %d reg %d imm %d\n", cmd->has_ram, cmd->cmd_id, cmd->reg_id, cmd->imm);\
                \
                return parse_args_ret_val;\
            }\
        } while(0);
    #include "../commands.h"
    #undef DEF_CMD

    printf(PAINT_TEXT(COLOR_RED, "NO CMD NAME\n"));

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

static int* parse_file_to_commands(File* file, size_t* position)
{
    assert(file);
    assert(position);

    tokenize_lines(file);
    const size_t line_amount = file->line_amounts;
    int* code_array = (int*) calloc(line_amount * MAX_ARGS_AMOUNT, sizeof(int));
    ASM_DEBUG_MSG("line_amount = %lld", line_amount);

    Labels labels = {};

    set_labels_names(file, &labels); // BAH: Make by two compiles

    for (size_t i = 0; i < line_amount; i++)
    {
        line* line_ptr = file->lines_ptrs + i;
        Command cmd = {};
        cmd_error err = CMD_NO_ERR;

        err = parse_line_to_command(&cmd, line_ptr, *position, &labels);
        if (err == CMD_NO_ERR)
            emit_code(code_array, position, &cmd);
        else if (err > 3)
            printf(PAINT_TEXT(COLOR_RED, "ERROR!!!\n"));
    }
    return code_array;
}

static asm_error write_to_file(const char* output_file_name, int* code_array, const size_t size)
{
    assert(output_file_name);
    assert(code_array);
    assert(size);

    FILE* file_ptr = fopen(output_file_name, "wb");
    if (!file_ptr)
        return FILE_OPEN_ERR;

    fwrite(code_array, sizeof(int), size, file_ptr);

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

    size_t size = 0;
    int* code_array = parse_file_to_commands(input_file, &size);
    write_to_file(output_file_name, code_array, size);

    return NO_ERR;
}
