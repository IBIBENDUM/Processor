#include "spu.h"
#include "../Libs/stack.h"
#include "../commands.h"

static stack stk {};


void HLT_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
    *current_shift += 2;
}

void push_immed(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
    *current_shift += 2;
}

void push_reg(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
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
    *current_shift += 2;
}

void in_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
    *current_shift += 2;
}

void out_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
    *current_shift += 2;
}

void add_func(int* code_array, size_t* current_shift)
{
    DEBUG_MSG("%s", __FUNCTION__);
    *current_shift += 2;
}

void execute_program(int* code_array)
{
    size_t current_shift = 0;
    while (*(code_array + current_shift) != OPERATIONS[HLT].id)
    {
        switch (*(code_array + current_shift) & COMMAND_MASK)
        {
            #define DO(X) case OPERATIONS[X].id: X##_func(code_array, &current_shift); break;
            COMMAND_LIST
            #undef DO

            default: break;
        }
    }
}
