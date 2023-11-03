#ifndef COMMANDS_H
#define COMMANDS_H

#include <inttypes.h>
#include <limits.h>

typedef int cmd_t;
typedef int arg_t;

const int FLOAT_COEFFICIENT = 100; ///< Decimal precision
const size_t VRAM_HEIGHT    = 40;  ///< Height of Virtual RAM frame
const size_t VRAM_WIDTH     = 80;  ///< Width  of Virtual RAM frame
const size_t VRAM_OFFSET    = 20;  ///< Number of cells not occupied by VRAM
const size_t RAM_SIZE       = VRAM_HEIGHT * VRAM_WIDTH + VRAM_OFFSET;

const int MAX_ARGS_AMOUNT   = 3;  ///< Maximum command arguments amount

/*****************************************************
*          OPERATION CODE STRUCTURE
*  | RAM | REG | IMM | ID | ID | ID | ID | ID | ID |
*  |-----|-----|-----|----|----|----|----|----|----|
*  |  1  |  1  |  0  |  0 |  0 |  0 |  0 |  0 |  1 |
*           EXAMPLE TABLE FOR PUSH [RAX]
******************************************************/

/// @brief Operation codes masks
///
/// They are needed to obtain information about command
/// from the operation code
enum Op_code_masks
{
    ARG_IMM_MASK   = 0b00100000,
    ARG_REG_MASK   = 0b01000000,
    ARG_RAM_MASK   = 0b10000000,
    ID_MASK        = 0b00011111
};

enum Args_combinations_masks
{
    ___ = 1 << 0,
    __I = 1 << 1,
    _R_ = 1 << 2,
    _RI = 1 << 3,
    M_I = 1 << 4,
    MR_ = 1 << 5,
    MRI = 1 << 6,
    ARGS_COMBINATIONS_AMOUNT
};

const uint8_t args_combinations_masks_arr[] =
{
    0b00000000, 0b00100000, 0b01000000,
    0b01100000, 0b10100000, 0b11000000,
    0b11100000
};

enum Cmds_ids
{
    #define DEF_CMD(NAME, ...) NAME##_enum,
    #include "commands.inc"
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
    #include "commands.inc"
    #undef DEF_CMD
};

#endif
