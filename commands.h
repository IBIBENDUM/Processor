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
        SPU_DEBUG_MSG("jmp\n");
        arg_t pos = get_bin_arg(code_array, &ip, regs);
        ip = pos - 1;
    })

// #define MAKE_COND_JMP(NAME, SIGN)\
//     DEF_CMD(NAME, 0b001,\
//         {\
//             SPU_DEBUG_MSG("%s\n", #NAME);\
//             \
//             arg_t value_1 = 0;\
//             pop_stack(&SPU_STACK, &value_1);\
//             push_stack(&SPU_STACK, value_1);\
//             \
//             arg_t value_2 = 0;\
//             pop_stack(&SPU_STACK, &value_2);\
//             push_stack(&SPU_STACK, value_2);\
//             \
//             if (value_2 SIGN value_1)\
//             {\
//                 arg_t pos = get_bin_arg(code_array, &ip, regs);\
//                 ip = pos - 1;\
//             }\
//         })
//
// MAKE_COND_JMP(ja, >)

DEF_CMD( HLT,  0b000)
DEF_CMD(  in,  0b000)
DEF_CMD( out,  0b000,
    {
        arg_t value = 0;
        pop_stack(&SPU_STACK, &value);
        push_stack(&SPU_STACK, value);
        const float out_value = (float) value / (float) FLOAT_COEFFICIENT;
        fprintf(stderr, "out = %g\n", out_value);
    })

DEF_CMD( add,  0b000)
DEF_CMD( sub,  0b000)
DEF_CMD( mul,  0b000)
DEF_CMD( div,  0b000)
