#ifndef SPU_H
#define SPU_H

#include "../common.h"
#include "../Libs/textlib.h"
#include "../Libs/stack.h"
#include "../Libs/stack_logs.h"

#define FREE_AND_NULL(ptr)\
do{\
    free(ptr);\
    ptr = NULL;\
} while(0)

#define REGS_LIST\
    DEF_REG(rax)\
    DEF_REG(rbx)\
    DEF_REG(rcx)\
    DEF_REG(rdx)\

enum Regs_list
{
    #define DEF_REG(NAME) NAME,
    REGS_LIST
    #undef DEF_REG

    REGS_AMOUNT
};

struct Register
{
    const wchar_t* const name;
    const uint8_t id;
    arg_t value;
};

struct Spu
{
    stack spu_stack;
    arg_t* ram;
    char* vram;
    Register regs[REGS_AMOUNT]
    {
        #define DEF_REG(NAME) {L ## #NAME, NAME, 0},
        REGS_LIST
        #undef DEF_REG
    };
};

void construct_spu(struct Spu* spu);

void destruct_spu(struct Spu* spu);

void execute_program(cmd_t* code_array, struct Spu* spu);

#endif
