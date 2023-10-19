#ifndef COMMANDS_H
#define COMMANDS_H

#include <inttypes.h>

enum Code_masks
{
    COMMAND_MASK     = 0b00011111,
    ARG_FORMAT_IMMED = 0b00100000,
    ARG_FORMAT_REG   = 0b01000000
};

#define COMMAND_LIST\
    DO(HLT)\
    DO(push)\
    DO(pop)\
    DO(in)\
    DO(out)\
    DO(add)\
    DO(sub)\
    DO(mul)\
    DO(div)\

enum Command_list
{
    #define DO(X) X##_enum,
    COMMAND_LIST
    #undef DO

    OPERATION_AMOUNT
};

struct Operation
{
    const wchar_t* const name;
    const short id;
};

constexpr Operation OPERATIONS[OPERATION_AMOUNT] =
{
    #define DO(X) {L ## #X,  X##_enum + 1},
    COMMAND_LIST
    #undef DO
};

#endif
