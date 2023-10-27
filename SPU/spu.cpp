#include <inttypes.h>

#include "spu.h"
#include "../common.h"
#include "../Libs/stack.h"

#define STK_DEBUG
#include "../Libs/stack_logs.h"
#include "../Libs/logs.h"

#define SPU_DEBUG
#ifdef SPU_DEBUG
    #define SPU_DEBUG_MSG(...) DEBUG_MSG(COLOR_YELLOW, __VA_ARGS__)
    #define SPU_ERROR_MSG(...) DEBUG_MSG(COLOR_RED, __VA_ARGS__)
#else
    #define SPU_DEBUG_MSG(...)
    #define SPU_ERROR_MSG(...)
#endif

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
    double value;
};

// BAH: Remove global variable
static stack SPU_STACK = {};
static stack RAM_STACK = {};

void construct_spu()
{
    init_stack(SPU_STACK);
    init_stack(RAM_STACK);
}

void destruct_spu()
{
    destruct_stack(&SPU_STACK);
    destruct_stack(&RAM_STACK);
}



arg_t* get_bin_arg(const cmd_t* code, ssize_t* ip, Register* regs)
{
    cmd_t cmd = *(arg_t*)((uint8_t*) code + *ip - sizeof(cmd_t));
    arg_t* res = 0;

    if (cmd & ARG_IMM_MASK)
    {
        res = (arg_t*)((uint8_t*) code + *ip);
        *ip += sizeof(arg_t);
    }
    if (cmd & ARG_REG_MASK)
    {
        res = (arg_t*) &(regs[*(arg_t*)((uint8_t*) code + *ip)].value);
        *ip += sizeof(arg_t);
    }
    if (cmd & ARG_RAM_MASK)
    {
        res = (arg_t*) &RAM_STACK.data[*res];
        *ip += sizeof(arg_t);
    }
    return res;
}

void execute_program(cmd_t* code_array)
{
    Register regs[REGS_AMOUNT]
    {
        #define DEF_REG(NAME) {L ## #NAME, NAME, 0},
        REGS_LIST
        #undef DEF_REG
    };

    ssize_t ip = 0;
    cmd_t cmd = *((uint8_t*) code_array + ip);

    while (ip > -1)
    {
        dump_stack(stderr, &SPU_STACK, 0);
        switch (cmd & ID_MASK)
        {
            #define DEF_CMD(NAME, ARG_MASK, ...) case OPERATIONS[NAME##_enum].id: SPU_DEBUG_MSG("name = %s, id = %d\n", #NAME, OPERATIONS[NAME##_enum].id); ip += sizeof(cmd_t); __VA_ARGS__; break;

            #include "../commands.h"
            #undef DEF_CMD

            default: printf(PAINT_TEXT(COLOR_RED, "There is not such command!\n"));
        }
        SPU_DEBUG_MSG("ip = %d\n", ip);
        cmd = *((uint8_t*) code_array + ip);
    }
}
