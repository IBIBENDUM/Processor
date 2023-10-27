DEF_CMD(push,  0b111,
    {
        arg_t arg = get_bin_arg(code_array, &ip, regs);
        SPU_DEBUG_MSG("push %d\n", arg);
        push_stack(&SPU_STACK, arg * FLOAT_COEFFICIENT);
    })

DEF_CMD(pop,  0b110,
    {
        arg_t value = 0;
        pop_stack(&SPU_STACK, &value);

        arg_t reg_id = get_bin_arg(code_array, &ip, regs);
        regs[reg_id].value = value;
        push_stack(&SPU_STACK, value);
    })

DEF_CMD(jmp,  0b001,
    {
        arg_t pos = get_bin_arg(code_array, &ip, regs);
        SPU_DEBUG_MSG("jmp %d\n", pos);
        ip = pos;
    })

DEF_CMD(call,  0b001,
    {
        push_stack(&SPU_STACK, ip);

        arg_t pos = get_bin_arg(code_array, &ip, regs);
        SPU_DEBUG_MSG("call %d\n", pos);
        ip = pos;
    })

DEF_CMD(ret,  0b000,
    {
        push_stack(&SPU_STACK, ip);

        arg_t ret_pos = 0;
        pop_stack(&SPU_STACK, &ret_pos);

        SPU_DEBUG_MSG("ret %d\n", ret_pos);

        ip = ret_pos;
    })

#define MAKE_COND_JMP(NAME, SIGN)\
    DEF_CMD(NAME, 0b001,\
        {\
            arg_t value_1 = 0;\
            pop_stack(&SPU_STACK, &value_1);\
            \
            arg_t value_2 = 0;\
            pop_stack(&SPU_STACK, &value_2);\
            \
            push_stack(&SPU_STACK, value_2);\
            \
            SPU_DEBUG_MSG("value_1 = %d value_2 = %d\n", value_2, value_1);\
            \
            arg_t pos = get_bin_arg(code_array, &ip, regs);\
            if (value_1 SIGN value_2)\
            {\
                SPU_DEBUG_MSG("%s %d\n", #NAME, pos);\
                ip = pos;\
            }\
        })

MAKE_COND_JMP(ja, >)
MAKE_COND_JMP(jae, >=)
MAKE_COND_JMP(jb, <)
MAKE_COND_JMP(jbe, <=)
MAKE_COND_JMP(je, ==)
#undef MAKE_COND_JMP

DEF_CMD( HLT,  0b000,
    {
        ip = -1;
    })
DEF_CMD(  in,  0b000,
    {
        arg_t value = 0;
        printf("Please enter value: ");
        if (!fscanf(stdin, "%d", &value))
        {
            SPU_ERROR_MSG("WRONG INPUT!");
            ip = -1;
        }
        push_stack(&SPU_STACK, value * FLOAT_COEFFICIENT);
    })

DEF_CMD( out,  0b000,
    {
        arg_t value = 0;
        pop_stack(&SPU_STACK, &value);
        push_stack(&SPU_STACK, value);
        const float out_value = (float) value / (float) FLOAT_COEFFICIENT;
        fprintf(stderr, "out = %g\n", out_value);
    })

#define MAKE_OPERATION_CMD(NAME, OPERATION, COEFF)\
DEF_CMD(NAME,  0b000,\
    {\
        arg_t value_1 = 0;\
        pop_stack(&SPU_STACK, &value_1);\
        \
        arg_t value_2 = 0;\
        pop_stack(&SPU_STACK, &value_2);\
        \
        push_stack(&SPU_STACK, value_2 * (COEFF) OPERATION value_1);\
    })

MAKE_OPERATION_CMD(add, +, 1)
MAKE_OPERATION_CMD(sub, -, 1)

MAKE_OPERATION_CMD(mul, *, 1.0 / FLOAT_COEFFICIENT)
MAKE_OPERATION_CMD(div, /, FLOAT_COEFFICIENT)
