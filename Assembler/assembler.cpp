#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <assert.h>

#include "../Libs/textlib.h"
#include "../Libs/colors.h"
#include "../Libs/logs.h"
#include "../Libs/utils.h"
#include "../common.h"
#include "assembler.h"
#include "assembler_errors.h"

struct Command
{
    cmd_t   cmd_id;
    arg_t   reg_id;
    arg_t   imm_value;
    uint8_t possible_args_bitmask;

    bool    has_ram;
    bool    has_reg;
    bool    has_imm;
    bool    has_label;
};

const ssize_t LABEL_POISON_VALUE = -1;
const size_t MAX_LABEL_NAME_LENGTH = 50;
struct Label
{
    wchar_t name[MAX_LABEL_NAME_LENGTH] = {};
    arg_t cmd_pos = 0;
};

const size_t LABELS_MAX_AMOUNT = 100;
struct Labels
{
    Label  labels_arr[LABELS_MAX_AMOUNT] = {};
    size_t amount = 0;
    size_t final_size = 0;
};

struct Bytecode
{
    uint8_t* code_array;
    size_t position;
};

/**
 * @brief Checks whether the command can have the arguments passed to it
 *
 * struct Command contains possible combinations of arguments,@n
 * args_combinations_arr_mask contains bitmasks for argument combinations
 *
 * @param op_code command byte code @see "common.h"
 * @param cmd      command structure that contains possible
 *                 arguments combinations
 * @return true  if this command has this combination of arguments
 * @return false if not
 */
static bool check_args_correctness(const Command* cmd, const int op_code)
{
    for (size_t bit = 0; bit < ARGS_COMBINATIONS_AMOUNT; bit++)
    {
        if (cmd->possible_args_bitmask & (1 << bit))
        {
            if ((op_code & (~ID_MASK)) == args_combinations_masks_arr[bit])
            {
                return true;
            }
        }
    }
    return false;
}

// Emit command error code and return error code
#define EMIT_CMD_ERROR_AND_RETURN_IT(CMD_ERROR_PTR, CMD_ERROR_ID)       \
    do {                                                                \
        wchar_t* op_name = (wchar_t*) OPERATIONS[cmd->cmd_id - 1].name; \
        line err_substring = {op_name, wcslen(op_name)};                \
        emit_cmd_error(CMD_ERROR_PTR, CMD_ERROR_ID, &err_substring);    \
                                                                        \
        return CMD_ERROR_ID;                                            \
    } while (0)

static cmd_error emit_bytecode(const void* value, const size_t type_size, Bytecode* bytecode)
{
    assert(value);
    assert(bytecode);

    memcpy(bytecode->code_array + bytecode->position, value, type_size);
    bytecode->position += type_size;

    return CMD_NO_ERR;
}

static cmd_error emit_command(const Command* const cmd, Command_error* const cmd_err, Bytecode* bytecode)
{
    assert(cmd);
    assert(bytecode);

    const int op_code = cmd->cmd_id | (ARG_IMM_MASK * cmd->has_imm) | (ARG_REG_MASK * cmd->has_reg) | (ARG_RAM_MASK * cmd->has_ram);

    if (!check_args_correctness(cmd, op_code))
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_WRONG_ARG_ERR);

    emit_bytecode(&op_code, sizeof(cmd_t), bytecode);

    if (cmd->has_imm)
        emit_bytecode(&cmd->imm_value, sizeof(arg_t), bytecode);

    if (cmd->has_reg)
        emit_bytecode(&cmd->reg_id, sizeof(arg_t), bytecode);

    return CMD_NO_ERR;
}
#undef EMIT_BYTE_CODE

static cmd_error emit_label(const line* label_name, const size_t position, Labels* labels)
{
    assert(label_name);
    assert(labels);

    if (label_name->len > MAX_LABEL_NAME_LENGTH - 1)
    {
        return CMD_TOO_LONG_LABEL_ERR;
    }

    // for (size_t label_id = 0; label_id < labels->amount; label_id++)
    // {
    //     if (wcsncmp(labels->labels_arr[label_id].name, label_name->start, label_name->len) == 0)
    //     {
    //         if (labels->labels_arr[label_id].cmd_pos != (arg_t) position)
    //             return CMD_REPEATED_LABEL_ERR;
    //     }
    // }

    // Check for first compilation
    if (labels->final_size == 0)
    {
        wcsncpy(labels->labels_arr[labels->amount].name, label_name->start, label_name->len);
        labels->labels_arr[labels->amount].cmd_pos = (int) position;

        labels->amount++;
        return CMD_NO_ERR;
    }
    return CMD_NO_ERR;
}

static cmd_error emit_imm_arg(Command* cmd, const wchar_t* arg_ptr)
{
    wchar_t* strtod_end_ptr = NULL;
    const double imm_double = wcstod(arg_ptr, &strtod_end_ptr);
    if (*strtod_end_ptr == L'\0' || *strtod_end_ptr == L' ')
    {
        LOG_TRACE("Argument type = imm");
        LOG_INFO("imm_value = %.2lf", imm_double);

        cmd->has_imm = true;
        cmd->imm_value = (arg_t) (imm_double * FLOAT_COEFFICIENT);
        return CMD_NO_ERR;
    }
    return CMD_WRONG_ARG_ERR;
}

static cmd_error emit_reg_arg(Command* cmd, const wchar_t* arg_ptr)
{
    if (arg_ptr[0] == L'r' && arg_ptr[2] == L'x')
    {
        const wchar_t reg_id = arg_ptr[1];
        if (L'a' <= reg_id && reg_id <= L'd')
        {
            LOG_TRACE("Argument type = reg");
            LOG_INFO("reg_id = %d", reg_id);

            cmd->has_reg = true;
            cmd->reg_id = reg_id - L'a' + 1;
            return CMD_NO_ERR;
        }
    }
    return CMD_WRONG_ARG_ERR;
}

static cmd_error emit_label_arg(Command* cmd, Labels* labels, const wchar_t* arg_ptr)
{
    if (labels->final_size == 0)
    {
        cmd->has_imm = true;
        cmd->has_label   = true;
        cmd->imm_value = LABEL_POISON_VALUE;
        return CMD_NO_ERR;
    }

    for (size_t id = 0; id < labels->amount; id++)
    {
        const wchar_t* label_name = labels->labels_arr[id].name;
        if (wcsncmp(arg_ptr, label_name, wcslen(label_name)) == 0)
        {
            cmd->has_imm = true;
            cmd->has_label = true;
            cmd->imm_value = labels->labels_arr[id].cmd_pos;
            return CMD_NO_ERR;
        }
    }
    return CMD_WRONG_ARG_ERR;
}

static cmd_error parse_arg(Command* cmd, Labels* labels, Command_error* cmd_err, wchar_t* arg_start_ptr)
{

    assert(cmd);
    assert(labels);
    assert(cmd_err);
    assert(arg_start_ptr);

    // Replace ']' with '\0' and back so that the search for imm and label
    // takes place without taking into account the bracket
    LOG_INFO("%ls", arg_start_ptr);
    wchar_t* right_br_ptr = wcschr(arg_start_ptr, L']');
    if (right_br_ptr)
        *right_br_ptr = L'\0';

    size_t arg_len = 0;
    wchar_t* arg_ptr = get_word(arg_start_ptr, &arg_len);
    LOG_TRACE("arg_ptr = %.*ls", arg_len, arg_ptr);

    if (arg_len == 0)
    {
        if (right_br_ptr)
            *right_br_ptr = L']';
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, TOO_FEW_ARGS_ERR);
    }

    // TODO: add macro
    bool is_arg_read = false;
    if (!cmd->has_imm && emit_imm_arg(cmd, arg_ptr) == CMD_NO_ERR)
    {
        is_arg_read = true;
    }
    else if (!cmd->has_reg && emit_reg_arg(cmd, arg_ptr) == CMD_NO_ERR)
    {
        is_arg_read = true;
    }
    else if (!cmd->has_label && emit_label_arg(cmd, labels, arg_ptr) == CMD_NO_ERR)
    {
        is_arg_read = true;
    }
    if (right_br_ptr)
        *right_br_ptr = L']';

    if (is_arg_read)
        return CMD_NO_ERR;

    return CMD_WRONG_ARG_ERR;
}

static bool emit_ram_arg(Command* cmd, wchar_t* arg_ptr, wchar_t**  left_br_ptr, wchar_t** right_br_ptr)
{
    *left_br_ptr = wcschr(arg_ptr, L'[');
    *right_br_ptr = wcschr(arg_ptr, L']');

    if (*left_br_ptr && *right_br_ptr && (*right_br_ptr - *left_br_ptr > 0))
    {
        LOG_INFO("cmd has RAM");
        cmd->has_ram = true;
        return true;
    }
    return false;
}

// BAH: add struct for cmd, labels, cmd_err
static cmd_error get_args(Command* cmd, Labels* labels, Command_error* cmd_err, const int possible_args_bitmask, wchar_t* arg_ptr)
{
    assert(arg_ptr);
    assert(cmd);
    assert(labels);
    assert(cmd_err);

    LOG_INFO("arg_ptr = <%ls>", arg_ptr);

    if (possible_args_bitmask == ___)
    {
        if (*move_to_non_space_sym(arg_ptr) != L'\0')
            EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_TOO_MANY_ARGS);

        return CMD_NO_ERR;
    }

    wchar_t* left_br_ptr = NULL;
    wchar_t* right_br_ptr = NULL;
    if (!emit_ram_arg(cmd, arg_ptr, &left_br_ptr, &right_br_ptr))
    {
        left_br_ptr = arg_ptr - 1;
    }

    cmd_error err = CMD_NO_ERR;
    err = parse_arg(cmd, labels, cmd_err, left_br_ptr + 1);
    if (err != CMD_NO_ERR)
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_WRONG_ARG_ERR);

    // Find + operator
    wchar_t* operator_ptr = wcschr(left_br_ptr + 1, L'+');

    // Get second argument
    if (operator_ptr)
    {
        err = parse_arg(cmd, labels, cmd_err, operator_ptr + 1);
        if (err != CMD_NO_ERR)
            EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_WRONG_ARG_ERR);
    }

    // BAH: Make other function
    // If there is "exta" args emit error and return from function
    // The term extra arguments refers to arguments that are not expected
    size_t extra_word_len = 0;

    // Because get_word() is similar to strtok(),
    // you can pass NULL and get a pointer to the next word
    get_word(NULL, &extra_word_len);
    if (extra_word_len)
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_TOO_MANY_ARGS);

    return err;
}

static bool parse_label(line* op_name, const line* line_ptr)
{
    bool is_label = false;
    if (op_name->len > 1)
    {
        // Check for colon (':') existence in line
        size_t colon_pos = wcscspn(line_ptr->start, L":");
        if (colon_pos != line_ptr->len)
        {
            // Because op_name contains first word we can check only two cases
            // Check for 'label:' case
            if (op_name->start[op_name->len-1] == L':')
            {
                op_name->len--;
                is_label = true;
            }
            // Check for 'label :' case
            else if (*move_to_non_space_sym(op_name->start + op_name->len) == L':')
            {
                is_label = true;
            }
            // Check for 'label: extra' case
            if (*move_to_non_space_sym(line_ptr->start + colon_pos + 1) != L'\0')
                is_label = false;
        }
    }
    return is_label;
}

static bool get_operation(Command* cmd, line* op_name, const Operation* op)
{
    if (op_name->len == op->name_len && wcsncmp(op_name->start, op->name, op->name_len) == 0)
    {
        *cmd = (Command){
                        .cmd_id                = op->id,
                        .possible_args_bitmask = op->possible_args_bitmask
                        };
        return true;
    }
    return false;
}

static cmd_error parse_line_to_command(Command* cmd, Labels* labels, Command_error* cmd_err, const line* line_ptr, const size_t position)
{
    assert(cmd);
    assert(line_ptr);
    assert(labels);

    size_t op_name_len = 0;
    wchar_t* op_name_str = get_word(line_ptr->start, &op_name_len);
    line op_name =  {
                    .start = op_name_str,
                    .len = op_name_len
                    };

    if (op_name.len == 0)
    {
        LOG_TRACE("Empty line!");
        return CMD_NO_ERR;
    }

    LOG_DEBUG("op_name_str = <%.*ls>", op_name.len, op_name.start);

    #define DEF_CMD(NAME, POSSIBLE_ARGS_BITMASK, ...)                                                                  \
        if (get_operation(cmd, &op_name, &OPERATIONS[NAME ## _enum]))                                                  \
        {                                                                                                              \
                cmd_error parse_args_ret_val = get_args(cmd, labels, cmd_err, POSSIBLE_ARGS_BITMASK,                   \
                                                        move_to_non_space_sym(op_name.start + op_name.len));           \
                LOG_TRACE("code: ram %d id %d reg %d imm %d", cmd->has_ram, cmd->cmd_id, cmd->reg_id, cmd->imm_value); \
                LOG_TRACE("get_args = %d", parse_args_ret_val);                                                        \
                                                                                                                       \
                return parse_args_ret_val;                                                                             \
                                                                                                                       \
        }
    #include "../commands.inc"
    // There is undef inside "command.inc"

    const line err_substring = (line){move_to_non_space_sym(line_ptr->start), line_ptr->len};
    if (parse_label(&op_name, line_ptr))
    {
        cmd_error err = emit_label(&op_name, position, labels);
        if (err != CMD_NO_ERR)
            emit_cmd_error(cmd_err, err, &err_substring);

        LOG_TRACE("LABEL LINE");
        return err;
    }
    emit_cmd_error(cmd_err, CMD_WRONG_NAME_ERR, &err_substring);
    return CMD_WRONG_NAME_ERR;
}

static asm_error parse_file_to_commands(File* file, Bytecode* bytecode, Labels* labels, Compiler_errors* errors)
{
    assert(file);
    assert(bytecode);
    assert(labels);

    LOG_INFO("Start parsing file to commands...");

    asm_error asm_err = ASM_NO_ERR;

    bytecode->position = 0;
    *errors = {};

    for (size_t i = 0; i < file->line_amount; i++)
    {
        LOG_TRACE("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        LOG_DEBUG("line = %d", i + 1);
        line* line_ptr = file->lines_ptrs + i;
        replace_with_zero(line_ptr, L';'); // Removes comments

        Command cmd = {};
        Command_error err_msg = { .line_idx = i + 1 };
        cmd_error err = CMD_NO_ERR;
        err = parse_line_to_command(&cmd, labels, &err_msg, line_ptr, bytecode->position);
        if (err == CMD_NO_ERR)
        {
            if (cmd.cmd_id != 0)
            {
                err = emit_command(&cmd, &err_msg, bytecode);
            }
        }
        if (err != CMD_NO_ERR)
        {
            emit_asm_error(errors, &err_msg, err);
            asm_err = ASM_PARSE_ARGS_ERR;
        }

        LOG_DEBUG("position = %d", bytecode->position);
    }
    labels->final_size = labels->amount;

    LOG_INFO("File parsed!");

    return asm_err;
}

static asm_error write_bytecode_to_file(const char* output_file_name, Bytecode* bytecode)
{
    assert(output_file_name);
    assert(bytecode);

    LOG_INFO("Writing bytecode to file...");

    // BAH: what if wrong folder name?
    FILE* file_ptr = fopen(output_file_name, "wb");
    if (!file_ptr)
        return ASM_FILE_OPEN_ERR;

    size_t amount_of_written = fwrite(bytecode->code_array, sizeof(uint8_t), bytecode->position, file_ptr);
    if (amount_of_written < bytecode->position)
        return ASM_WRITE_ERR;

    int f_close_ret_val = fclose(file_ptr);
    file_ptr = NULL;

    if (f_close_ret_val)
        return ASM_FILE_CLOSE_ERR;

    LOG_INFO("Bytecode has been written!");

    return ASM_NO_ERR;
}

static asm_error convert_text_to_binary(File* input_file, const char* output_file_name)
{
    assert(input_file);
    assert(output_file_name);

    LOG_INFO("Prepare text for converting...");
    tokenize_lines(input_file);
    const size_t line_amount = input_file->line_amount;

    // Find maximum type len and reserve place for the largest
    // BAH: Make with realloc
    const size_t max_arg_len = (sizeof(cmd_t) > sizeof(arg_t)) ? sizeof(cmd_t) : sizeof(arg_t);
    Bytecode bytecode =
    {
        .code_array = (uint8_t*) calloc(line_amount * MAX_ARGS_AMOUNT, max_arg_len),
        .position = 0
    };

    if (!bytecode.code_array)
        return ASM_MEM_ALLOC_ERR;

    Labels labels = {};
    Compiler_errors errors = {};

    asm_error asm_err = ASM_NO_ERR;
    asm_err = parse_file_to_commands(input_file, &bytecode, &labels, &errors); // First  pass
    asm_err = parse_file_to_commands(input_file, &bytecode, &labels, &errors); // Second pass

    if (asm_err != ASM_NO_ERR)
    {
        print_compiler_errors(&errors);
        return asm_err;
    }

    write_bytecode_to_file(output_file_name, &bytecode);

    free_and_null(bytecode.code_array);

    LOG_INFO("Text converted!");

    return ASM_NO_ERR;
}

asm_error compile_file(const char* input_file_name, const char* output_file_name)
{
    assert(input_file_name);
    assert(output_file_name);

    LOG_INFO("Start compiling file...");
    File input_file = {};
    if (!init_file(input_file_name, &input_file))
    {
        LOG_ERROR("Can't read file");
        return ASM_INPUT_FILE_READ_ERR;
    }

    asm_error err = convert_text_to_binary(&input_file, output_file_name);
    destruct_file(&input_file);
    LOG_INFO("Compilation complete!");

    return err;
}
