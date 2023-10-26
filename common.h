#ifndef COMMANDS_H
#define COMMANDS_H

#include <inttypes.h>

typedef int arg_t;
typedef int cmd_t;

const int MAX_ARGS_AMOUNT = 3;

const int ARGS_MASK_OFFSET = 5;

enum Op_code_masks
{
    ARG_IMM_MASK   = 0b00100000,
    ARG_REG_MASK   = 0b01000000,
    ARG_RAM_MASK   = 0b10000000,
    ID_MASK        = 0b00011111
};

enum Cmds_ids
{
    #define DEF_CMD(NAME, ...) NAME##_enum,
    #include "commands.h"
    #undef DEF_CMD

    OPERATION_AMOUNT
};

struct Operation
{
    const wchar_t* const name;
    const short id;
};

constexpr Operation OPERATIONS[OPERATION_AMOUNT] =
{
    #define DEF_CMD(NAME, ...) {L ## #NAME,  NAME##_enum + 1},
    #include "commands.h"
    #undef DEF_CMD
};

#endif
