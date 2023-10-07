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

const uint8_t COMMAND_MASK = 0b00011111;

struct Operation
{
    const wchar_t* const name;
    const uint8_t code;
};

const size_t OPERATION_AMOUNT = 3;

const Operation OPERATIONS[OPERATION_AMOUNT] =
{
    {L"push", 0b00100001},
    {L"push", 0b01000001},
    {L"add" , 0b00000010},
    {L"div" , 0b00000011}
};
#endif
