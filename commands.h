#define PUSH(ARG) push_stack(&spu->spu_stack, ARG)
#define POP(ARG)  pop_stack(&spu->spu_stack, &ARG)
#define GET_ARG   get_bin_arg(code_array, &ip, spu)

DEF_CMD(push, __I | _RI| _R_ | M_I | MR_ | MRI,
    {
        arg_t arg = *GET_ARG;
        SPU_DEBUG_MSG("push %d\n", arg);
        PUSH(arg);
    })

DEF_CMD(pop, ___ | _R_ | M_I | MR_ | MRI,
    {
        arg_t value = 0;
        POP(value);
        arg_t* reg_id = GET_ARG;
        *reg_id = value;
        SPU_DEBUG_MSG("reg_id = %d", *reg_id);
    })

DEF_CMD(sqrt, ___,
    {
        arg_t value = 0;
        POP(value);
        PUSH((arg_t) (sqrt((double) value * FLOAT_COEFFICIENT)));
    })

DEF_CMD(jmp, __I | _R_ | M_I | MR_ | MRI,
    {
        arg_t pos = *GET_ARG;
        SPU_DEBUG_MSG("jmp %d\n", pos);
        ip = pos;
    })

DEF_CMD(call, __I | _R_ | M_I | MR_ | MRI,
    {
        arg_t pos = *GET_ARG;
        SPU_DEBUG_MSG("call %d\n", pos);
        PUSH((arg_t) ip);
        ip = pos;
    })

DEF_CMD(ret, ___,
    {
        // arg_t ret_value = 0;
        // POP(ret_value);
        // regs[rax + 1].value = ret_value;
        // SPU_DEBUG_MSG("ret_value %d\n", ret_value);

        arg_t ret_pos = 0;
        POP(ret_pos);

        SPU_DEBUG_MSG("ret %d\n", ret_pos);

        ip = ret_pos;
    })

#define MAKE_COND_JMP(NAME, SIGN)\
    DEF_CMD(NAME, __I | _R_ | M_I | MR_ | MRI,\
        {\
            arg_t value_1 = 0;\
            POP(value_1);\
            \
            arg_t value_2 = 0;\
            POP(value_2);\
            \
            SPU_DEBUG_MSG("value_1 = %d value_2 = %d\n", value_2, value_1);\
            \
            arg_t pos = *GET_ARG;\
            if (value_2 SIGN value_1)\
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

DEF_CMD(HLT, ___,
    {
        ip = -1;
    })

DEF_CMD(in, ___,
    {
        arg_t value = 0;
        printf("Please enter value: ");
        if (!fscanf(stdin, "%d", &value))
        {
            SPU_ERROR_MSG("WRONG INPUT!");
            ip = -1;
        }
        PUSH(value * FLOAT_COEFFICIENT);
    })

DEF_CMD(out, ___,
    {
        arg_t value = 0;
        POP(value);
        PUSH(value);
        const float out_value = (float) value / (float) FLOAT_COEFFICIENT;
        fprintf(stderr, PAINT_TEXT(COLOR_GREEN, "out = %g\n"), out_value);
    })

DEF_CMD(dump, ___,
    {
        print_ram(spu);
    })

#define MAKE_OPERATION_CMD(NAME, OPERATION, COEFF)\
DEF_CMD(NAME, ___,\
    {\
        arg_t value_1 = 0;\
        POP(value_1);\
        \
        arg_t value_2 = 0;\
        POP(value_2);\
        \
        PUSH((arg_t) (value_2 * (COEFF) OPERATION value_1));\
    })

MAKE_OPERATION_CMD(add, +, 1)
MAKE_OPERATION_CMD(sub, -, 1)

MAKE_OPERATION_CMD(mul, *, 1.0 / FLOAT_COEFFICIENT)
MAKE_OPERATION_CMD(div, /, FLOAT_COEFFICIENT)
