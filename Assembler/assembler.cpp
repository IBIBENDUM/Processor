#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <assert.h>

#include "../Libs/textlib.h"
#include "../Libs/colors.h"
#include "../Libs/logs.h"
#include "../Libs/utils.h"
#include "../Libs/time_utils.h"
#include "../common.h"

#include "assembler.h"
#include "assembler_errors.h"
#include "assembler_listing.h"
#include "assembler_emitters.h"

static cmd_error parse_arg(Command* cmd, Labels* labels, Command_error* cmd_err, wchar_t* arg_start_ptr)
{
    assert(cmd);
    assert(labels);
    assert(cmd_err);
    assert(arg_start_ptr);

    // Replace ']' with '\0' and back so that the search for imm and label
    // takes place without taking into account the bracket
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
    bool is_arg_read = false; // There is bool because i need to return right bracket
    if (!cmd->has_imm && emit_imm_arg(cmd, arg_ptr) == CMD_NO_ERR)
        is_arg_read = true;

    else if (!cmd->has_reg && emit_reg_arg(cmd, arg_ptr) == CMD_NO_ERR)
        is_arg_read = true;

    else if (!cmd->has_label && emit_label_arg(cmd, labels, arg_ptr) == CMD_NO_ERR)
        is_arg_read = true;

    if (right_br_ptr)
        *right_br_ptr = L']';

    if (is_arg_read)
        return CMD_NO_ERR;

    return CMD_WRONG_ARG_ERR;
}

// BAH: add struct for cmd, labels, cmd_err
static cmd_error get_args(Command* cmd, Labels* labels, Command_error* cmd_err, const int possible_args_bitmask, wchar_t* arg_ptr)
{
    assert(arg_ptr);
    assert(cmd);
    assert(labels);
    assert(cmd_err);

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

    // If there is "extra" args emit error and return from function
    // The term extra arguments refers to arguments that are not expected
    size_t extra_word_len = 0;

    // Because get_word() is similar to strtok(),
    // you can pass NULL and get a pointer to the next word
    get_word(NULL, &extra_word_len);
    if (extra_word_len)
        EMIT_CMD_ERROR_AND_RETURN_IT(cmd_err, CMD_TOO_MANY_ARGS);

    return err;
}

bool parse_label(line* op_name, const line* line_ptr)
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

        LOG_TRACE("Empty line!");
        return err;
    }
    emit_cmd_error(cmd_err, CMD_WRONG_NAME_ERR, &err_substring);

    return CMD_WRONG_NAME_ERR;
}

static asm_error realloc_bytecode_array(Bytecode* bytecode, size_t* bytecode_capacity)
{
    assert(bytecode);
    assert(bytecode_capacity);

    *bytecode_capacity *= 2;
    uint8_t* new_code_array = (uint8_t*) realloc(bytecode->code_array, *bytecode_capacity * sizeof(uint8_t));

    if (!new_code_array)
        return ASM_MEM_ALLOC_ERR;

    bytecode->code_array = new_code_array;

    return ASM_NO_ERR;
}

static asm_error parse_file_to_commands(File* file, Bytecode* bytecode, Labels* labels, Compiler_errors* errors)
{
    assert(file);
    assert(bytecode);
    assert(labels);

    LOG_INFO("Start parsing file to commands...");

    asm_error asm_err = ASM_NO_ERR;

    size_t bytecode_capacity = MIN_BYTECODE_CAPACITY;

    bytecode->position = 0;
    *errors = {};

    for (size_t i = 0; i < file->line_amount; i++)
    {
        if (bytecode->position > bytecode_capacity)
            realloc_bytecode_array(bytecode, &bytecode_capacity);

        LOG_TRACE("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
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

    const size_t amount_of_written = fwrite(bytecode->code_array, sizeof(uint8_t), bytecode->position, file_ptr);
    if (amount_of_written < bytecode->position)
        return ASM_WRITE_ERR;

    const int f_close_ret_val = fclose(file_ptr);
    file_ptr = NULL;

    if (f_close_ret_val)
        return ASM_FILE_CLOSE_ERR;

    LOG_INFO("Bytecode has been written!");

    return ASM_NO_ERR;
}

static asm_error convert_text_to_binary(File* input_file, const char* output_file_name, const char* listing_file_name)
{
    assert(input_file);
    assert(output_file_name);

    LOG_INFO("Prepare text for converting...");

    tokenize_lines(input_file); // Replace '\0' with '\n'

    const size_t line_amount = input_file->line_amount;
    Bytecode bytecode =
    {
        .code_array = (uint8_t*) calloc(MIN_BYTECODE_CAPACITY, sizeof(uint8_t)),
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

    generate_listing(listing_file_name, input_file, output_file_name, &bytecode);

    free_and_null(bytecode.code_array);

    LOG_INFO("Text converted!");

    return ASM_NO_ERR;
}

asm_error compile_file(const char* input_file_name, const char* output_file_name, const char* listing_file_name)
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

    asm_error err = convert_text_to_binary(&input_file, output_file_name, listing_file_name);
    destruct_file(&input_file);
    LOG_INFO("Compilation complete!");

    return err;
}
