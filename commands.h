#ifndef COMMANDS_H
#define COMMANDS_H

#include <inttypes.h>
/*
push a
push a b
pop rax
pop a ; comment
in
out
HLT

add
sub
mul
div
sqrt
*/

// typedef int elem_t;

enum Code_formats
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

enum Command_list
{
    #define DO(X) X,
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
    #define DO(X) {L ## #X,  X + 1},
    COMMAND_LIST
    #undef DO
};

#define REGS_LIST\
    DO(rax)\
    DO(rbx)\
    DO(rcx)\
    DO(rdx)\

enum Regs_list
{
    #define DO(X) X,
    REGS_LIST
    #undef DO

    REGS_AMOUNT
};

struct Register
{
    const wchar_t* const name;
    const uint8_t id;
    int value;
};

Register regs[REGS_AMOUNT]
{
    #define DO(X) {L ## #X, X, 0},
    REGS_LIST
    #undef DO
};

#endif
