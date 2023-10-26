#include <inttypes.h>

#include "spu.h"
#include "../common.h"
#include "../Libs/stack.h"
#include "../Libs/logs.h"

#define SPU_DEBUG

#ifdef SPU_DEBUG
    #define SPU_DEBUG_MSG(...) DEBUG_MSG(__VA_ARGS__)
#else
    #define SPU_DEBUG_MSG(...)
#endif

// BAH: Remove global variable
static stack SPU_STACK = {};

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

void construct_spu()
{
    init_stack(SPU_STACK);
}

void destruct_spu()
{
    destruct_stack(&SPU_STACK);
}
//
// void HLT_func(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//     *ip += 2;
// }
//
// void push_immed(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//     push_stack(&SPU_STACK, *(code_array + *ip + 1) * FLOAT_COEFFICIENT);
//     *ip += 2;
// }
//
// void push_reg(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//
//     int value = 0;
//     value = regs[*(code_array + *ip + 1)].value;
//
//     push_stack(&SPU_STACK, value);
//
//     *ip += 2;
// }

arg_t get_bin_arg(const cmd_t* code, size_t* ip, Register* regs)
{
    cmd_t cmd = *(arg_t*)((uint8_t*) code + *ip - sizeof(cmd_t));

    arg_t res = 0;
    if (cmd & ARG_IMM_MASK)
    {
        res += *(arg_t*)((uint8_t*) code + *ip);
        *ip += sizeof(arg_t);
    }
    if (cmd & ARG_REG_MASK)
    {
        res += regs[*(arg_t*)((uint8_t*) code + *ip)].value;
        *ip += sizeof(arg_t);
    }
    return res;
}
//
// void pop_func(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//
//     int value = 0;
//     pop_stack(&SPU_STACK, &value);
//
//     regs[*(code_array + *ip + 1)].value = value * FLOAT_COEFFICIENT;
//     push_stack(&SPU_STACK, value);
//
//     *ip += 2;
// }
//
// void in_func(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//     int value = 0;
//     fscanf(stdin, "%d", &value);
//     push_stack(&SPU_STACK, value * FLOAT_COEFFICIENT);
//     *ip += 2;
// }
//
// void out_func(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//     int value = 0;
//     pop_stack(&SPU_STACK, &value);
//     push_stack(&SPU_STACK, value);
//     const float out_value = (float) value / (float) FLOAT_COEFFICIENT;
//     fprintf(stderr, "%g\n", out_value);
//     *ip += 2;
// }
//
// void add_func(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//
//     int value_1 = 0;
//     pop_stack(&SPU_STACK, &value_1);
//
//     int value_2 = 0;
//     pop_stack(&SPU_STACK, &value_2);
//
//     int result_value = value_2 + value_1;
//
//     push_stack(&SPU_STACK, result_value);
//
//     *ip += 2;
// }
//
// void sub_func(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//
//     int value_1 = 0;
//     pop_stack(&SPU_STACK, &value_1);
//
//     int value_2 = 0;
//     pop_stack(&SPU_STACK, &value_2);
//
//     int result_value = value_2 - value_1;
//
//     push_stack(&SPU_STACK, result_value);
//
//     *ip += 2;
// }
//
// void mul_func(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//
//     int value_1 = 0;
//     pop_stack(&SPU_STACK, &value_1);
//
//     int value_2 = 0;
//     pop_stack(&SPU_STACK, &value_2);
//
//     int result_value = value_2 * value_1 / FLOAT_COEFFICIENT;
//
//     push_stack(&SPU_STACK, result_value);
//
//     *ip += 2;
// }
//
// void div_func(int* code_array, size_t* ip)
// {
//     SPU_DEBUG_MSG("%s", __FUNCTION__);
//
//     int value_1 = 0;
//     pop_stack(&SPU_STACK, &value_1);
//
//     int value_2 = 0;
//     pop_stack(&SPU_STACK, &value_2);
//
//     int result_value = value_2 * FLOAT_COEFFICIENT / value_1 ;
//
//     push_stack(&SPU_STACK, result_value);
//
//     *ip += 2;
// }

void execute_program(cmd_t* code_array)
{
    Register regs[REGS_AMOUNT]
    {
        #define DO(X) {L ## #X, X, 0},
        REGS_LIST
        #undef DO
    };

    size_t ip = 0;
    uint8_t cmd = *(code_array + ip);
    while (cmd != OPERATIONS[HLT_enum].id && cmd != 0)
    {
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
