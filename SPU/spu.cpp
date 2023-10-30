#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "spu.h"
#include "../common.h"
#include "../Libs/stack.h"

// #define STK_DEBUG
#include "../Libs/stack_logs.h"
#include "../Libs/logs.h"

// #define SPU_DEBUG
#ifdef SPU_DEBUG
    #define SPU_DEBUG_MSG(...) DEBUG_MSG(COLOR_YELLOW, __VA_ARGS__)
    #define SPU_ERROR_MSG(...) DEBUG_MSG(COLOR_RED, __VA_ARGS__)
#else
    #define SPU_DEBUG_MSG(...)
    #define SPU_ERROR_MSG(...)
#endif

void construct_spu(struct Spu* spu)
{
    LOG_INFO("Initializing SPU...");
    init_stack(spu->spu_stack);
    spu->ram  = (arg_t*) calloc(RAM_SIZE, sizeof(spu->ram[0]));
    spu->vram = (char*) calloc(RAM_SIZE, sizeof(spu->vram[0]));
    LOG_INFO("SPU initialized");
}

void destruct_spu(struct Spu* spu)
{
    LOG_INFO("Destructing SPU...");
    destruct_stack(&spu->spu_stack);
    FREE_AND_NULL(spu->ram);
    // FREE_AND_NULL(spu->vram);
    LOG_INFO("SPU destructed");
}

static arg_t* get_bin_arg(cmd_t* code, ssize_t* ip, struct Spu* spu)
{
    assert(code);
    assert(ip);
    assert(spu);

    cmd_t cmd = *(arg_t*)((uint8_t*) code + *ip - sizeof(cmd_t));
    arg_t* res = 0;
    SPU_DEBUG_MSG("cmd %d", cmd);

    if (cmd & ARG_IMM_MASK)
    {
        res = (arg_t*)((uint8_t*) code + *ip);
        *ip += sizeof(arg_t);
        SPU_DEBUG_MSG("res_imm = %d", *res);
    }
    if (cmd & ARG_REG_MASK)
    {
        SPU_DEBUG_MSG("reg_id = %d", *(arg_t*)((uint8_t*) code + *ip));
        res = (arg_t*) &(spu->regs[*(arg_t*)((uint8_t*) code + *ip)].value);

        *ip += sizeof(arg_t);
        SPU_DEBUG_MSG("res_reg = %d", *res);
    }
    if (cmd & ARG_RAM_MASK)
    {
        res = (arg_t*) &spu->ram[*res / FLOAT_COEFFICIENT];
        SPU_DEBUG_MSG("RAM");
    }
    return res;
}

void print_ram(Spu* spu)
{
    LOG_INFO("Printing RAM...");
    for (size_t y = 0; y < VRAM_HEIGHT; y++)
    {
        for (size_t x = 0; x < VRAM_WIDTH; x++)
        {
            switch (spu->ram[(VRAM_WIDTH) * y + x + VRAM_OFFSET])
            {
                case 0:
                {
                    spu->vram[(VRAM_WIDTH + 1) * y + x] = '.';
                    break;
                }
                case 100:
                {
                    spu->vram[(VRAM_WIDTH + 1) * y + x] = '0';
                    break;
                }
                default:
                {
                    spu->vram[(VRAM_WIDTH + 1) * y + x] = '?';
                    break;
                }
            }
        }
        spu->vram[(VRAM_WIDTH + 1) * y + VRAM_WIDTH] = '\n';
    }
    fwrite(spu->vram, VRAM_HEIGHT * (VRAM_WIDTH + 1), sizeof(char), stderr);
}

void execute_program(cmd_t* code_array, struct Spu* spu)
{
    assert(code_array);
    assert(spu);


    ssize_t ip = 0;
    while (ip > -1)
    {
        SPU_DEBUG_MSG("ip = %lld\n", ip);
        cmd_t cmd = *((uint8_t*) code_array + ip);
        dump_stack(stderr, &spu->spu_stack, 0);
        switch (cmd & ID_MASK)
        {
            #define DEF_CMD(NAME, ARG_MASK, ...) case OPERATIONS[NAME##_enum].id: ip += sizeof(cmd_t); __VA_ARGS__; break;

            #include "../commands.h"
            #undef DEF_CMD

            default: printf(PAINT_TEXT(COLOR_RED, "There is not such command!\n")); return;
        }
    }
}
