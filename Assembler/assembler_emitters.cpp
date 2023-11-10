#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <assert.h>

#include "../Libs/textlib.h"
#include "../Libs/logs.h"
#include "../common.h"

#include "assembler.h"
#include "assembler_errors.h"
#include "assembler_emitters.h"

cmd_error emit_bytecode(const void* value, const size_t type_size, Bytecode* bytecode)
{
    assert(value);
    assert(bytecode);

    memcpy(bytecode->code_array + bytecode->position, value, type_size);
    bytecode->position += type_size;

    return CMD_NO_ERR;
}

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

cmd_error emit_command(const Command* const cmd, Command_error* const cmd_err, Bytecode* bytecode)
{
    assert(cmd);
    assert(cmd_err);
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

cmd_error emit_label(const line* label_name, const size_t position, Labels* labels)
{
    assert(label_name);
    assert(labels);

    if (label_name->len > MAX_LABEL_NAME_LENGTH - 1)
    {
        return CMD_TOO_LONG_LABEL_ERR;
    }

    for (size_t label_id = 0; label_id < labels->amount; label_id++)
    {
        if (wcsncmp(labels->labels_arr[label_id].name, label_name->start, label_name->len) == 0)
        {
            if (labels->labels_arr[label_id].cmd_pos != (arg_t) position)
                return CMD_REPEATED_LABEL_ERR;
        }
    }
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

cmd_error emit_imm_arg(Command* cmd, const wchar_t* arg_ptr)
{
    wchar_t* strtod_end_ptr = NULL;
    const double imm_double = wcstod(arg_ptr, &strtod_end_ptr);
    if (*strtod_end_ptr == L'\0' || *strtod_end_ptr == L' ')
    {
        LOG_TRACE("Argument type = imm");
        LOG_DEBUG("imm_value = %.2lf", imm_double);

        cmd->has_imm = true;
        cmd->imm_value = (arg_t) (imm_double * FLOAT_COEFFICIENT);
        return CMD_NO_ERR;
    }
    return CMD_WRONG_ARG_ERR;
}

cmd_error emit_reg_arg(Command* cmd, const wchar_t* arg_ptr)
{
    if (arg_ptr[0] == L'r' && arg_ptr[2] == L'x')
    {
        const wchar_t reg_id = arg_ptr[1];
        if (L'a' <= reg_id && reg_id <= L'd')
        {
            LOG_TRACE("Argument type = reg");
            LOG_DEBUG("reg_id = %d", reg_id);

            cmd->has_reg = true;
            cmd->reg_id = reg_id - L'a' + 1;
            return CMD_NO_ERR;
        }
    }
    return CMD_WRONG_ARG_ERR;
}

cmd_error emit_label_arg(Command* cmd, Labels* labels, const wchar_t* arg_ptr)
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

bool emit_ram_arg(Command* cmd, wchar_t* arg_ptr, wchar_t**  left_br_ptr, wchar_t** right_br_ptr)
{
    *left_br_ptr = wcschr(arg_ptr, L'[');
    *right_br_ptr = wcschr(arg_ptr, L']');

    if (*left_br_ptr && *right_br_ptr && (*right_br_ptr - *left_br_ptr > 0))
    {
        LOG_DEBUG("cmd has RAM");
        cmd->has_ram = true;
        return true;
    }
    return false;
}
