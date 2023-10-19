#include "spu.h"
#include "../Libs/stack.h"
#include "../commands.h"

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

Register regs[REGS_AMOUNT]
{
    #define DO(X) {L ## #X, X, 0},
    REGS_LIST
    #undef DO
};

void construct_spu()
{
    init_stack(SPU_STACK);
}

void destruct_spu()
{
    destruct_stack(&SPU_STACK);
}

void HLT_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
    *current_shift += 2;
}

void push_immed(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
    push_stack(&SPU_STACK, *(code_array + *current_shift + 1) * FLOAT_COEFFICIENT);
    *current_shift += 2;
}

void push_reg(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);

    int value = 0;
    value = regs[*(code_array + *current_shift + 1)].value;

    push_stack(&SPU_STACK, value);

    *current_shift += 2;
}

void push_func(int* code_array, size_t* current_shift)
{
    if (*(code_array + *current_shift) & ARG_FORMAT_IMMED)
        push_immed(code_array, current_shift);
    else
        push_reg(code_array, current_shift);
}

void pop_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);

    int value = 0;
    pop_stack(&SPU_STACK, &value);

    regs[*(code_array + *current_shift + 1)].value = value * FLOAT_COEFFICIENT;
    push_stack(&SPU_STACK, value);

    *current_shift += 2;
}

void in_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
    int value = 0;
    fscanf(stdin, "%d", &value);
    push_stack(&SPU_STACK, value * FLOAT_COEFFICIENT);
    *current_shift += 2;
}

void out_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
    int value = 0;
    pop_stack(&SPU_STACK, &value);
    push_stack(&SPU_STACK, value);
    const float out_value = (float) value / (float) FLOAT_COEFFICIENT;
    fprintf(stderr, "%g\n", out_value);
    *current_shift += 2;
}

void add_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);

    int value_1 = 0;
    pop_stack(&SPU_STACK, &value_1);

    int value_2 = 0;
    pop_stack(&SPU_STACK, &value_2);

    int result_value = value_2 + value_1;

    push_stack(&SPU_STACK, result_value);

    *current_shift += 2;
}

void sub_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);

    int value_1 = 0;
    pop_stack(&SPU_STACK, &value_1);

    int value_2 = 0;
    pop_stack(&SPU_STACK, &value_2);

    int result_value = value_2 - value_1;

    push_stack(&SPU_STACK, result_value);

    *current_shift += 2;
}

void mul_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);

    int value_1 = 0;
    pop_stack(&SPU_STACK, &value_1);

    int value_2 = 0;
    pop_stack(&SPU_STACK, &value_2);

    int result_value = value_2 * value_1 / FLOAT_COEFFICIENT;

    push_stack(&SPU_STACK, result_value);

    *current_shift += 2;
}

void div_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);

    int value_1 = 0;
    pop_stack(&SPU_STACK, &value_1);

    int value_2 = 0;
    pop_stack(&SPU_STACK, &value_2);

    int result_value = value_2 * FLOAT_COEFFICIENT / value_1 ;

    push_stack(&SPU_STACK, result_value);

    *current_shift += 2;
}

void execute_program(int* code_array)
{
    size_t current_shift = 0;
    while (*(code_array + current_shift) != OPERATIONS[HLT_enum].id)
    {
        switch (*(code_array + current_shift) & COMMAND_MASK)
        {
            #define DO(X) case OPERATIONS[X##_enum].id: X##_func(code_array, &current_shift); break;
            COMMAND_LIST
            #undef DO

            default: break;
        }
    }
}
