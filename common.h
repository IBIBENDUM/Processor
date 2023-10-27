#ifndef COMMANDS_H
#define COMMANDS_H

#include <inttypes.h>
#include <limits.h>

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


enum Args_combinations =
{
    ___ = 0x0                                        >> CHAR_BIT * 0,
    __I = ARG_IMM_MASK                               >> CHAR_BIT * 1,
    _R_ = ARG_RAM_MASK                               >> CHAR_BIT * 2,
    M_I = ARG_RAM_MASK | ARG_IMM_MASK                >> CHAR_BIT * 3,
    MR_ = ARG_RAM_MASK | ARG_RAM_MASK                >> CHAR_BIT * 4,
    MRI = ARG_RAM_MASK | ARG_RAM_MASK | ARG_IMM_MASK >> CHAR_BIT * 5
}


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
