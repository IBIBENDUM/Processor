#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "spu.h"
#include "../common.h"
#include "../Libs/stack.h"
#include "../Libs/textlib.h"
#include "../Libs/logs.h"
#include "../Libs/stack_logs.h"
#include "../Libs/utils.h"

// BAH: Add RAM sleep

void construct_spu(struct Spu* spu)
{
    LOG_INFO("Initializing SPU...");
    init_stack(spu->spu_stack);
    spu->ram  = (arg_t*) calloc(RAM_SIZE, sizeof(spu->ram[0]));
    spu->vram = (wchar_t*) calloc(RAM_SIZE, sizeof(spu->vram[0]));
    LOG_INFO("SPU initialized");
}

void destruct_spu(struct Spu* spu)
{
    LOG_INFO("Destructing SPU...");
    destruct_stack(&spu->spu_stack);
    free_and_null(spu->ram);
    free_and_null(spu->vram);
    LOG_INFO("SPU destructed");
}

static arg_t* get_bin_arg(cmd_t* code, ssize_t* ip, struct Spu* spu)
{
    assert(code);
    assert(ip);
    assert(spu);

    cmd_t cmd = *(arg_t*)((uint8_t*) code + *ip - sizeof(cmd_t));
    arg_t* res = 0;
    LOG_TRACE("cmd %d", cmd);

    if (cmd & ARG_IMM_MASK)
    {
        res = (arg_t*)((uint8_t*) code + *ip);
        *ip += sizeof(arg_t);
        LOG_TRACE("res_imm = %d", *res);
    }
    if (cmd & ARG_REG_MASK)
    {
        LOG_TRACE("reg_id = %d", *(arg_t*)((uint8_t*) code + *ip));
        res = (arg_t*) &(spu->regs[*(arg_t*)((uint8_t*) code + *ip)].value);
        *ip += sizeof(arg_t);
        LOG_TRACE("res_reg = %d", *res);
    }
    if (cmd & ARG_RAM_MASK)
    {
        res = (arg_t*) &spu->ram[*res / FLOAT_COEFFICIENT];
        LOG_TRACE("RAM");
    }
    return res;
}

static void print_ram(Spu* spu)
{
    LOG_INFO("Printing VRAM...");
    size_t position = 0;
    const size_t vram_width  = spu->ram[VRAM_WIDTH_POS]  / FLOAT_COEFFICIENT;
    const size_t vram_height = spu->ram[VRAM_HEIGHT_POS] / FLOAT_COEFFICIENT;
    const size_t vram_offset = spu->ram[VRAM_OFFSET_POS] / FLOAT_COEFFICIENT;
    LOG_DEBUG("vram_width  = %d", vram_width);
    LOG_DEBUG("vram_height = %d", vram_height);
    LOG_DEBUG("vram_offset = %d", vram_offset);
    if (vram_width * vram_height + vram_offset > RAM_SIZE)
    {
        LOG_ERROR("vram_width * vram_height + vram_offset = %d", vram_width * vram_height + vram_offset);
        LOG_ERROR("VRAM size is larger than RAM");
        return;
    }
    for (size_t y = 0; y < vram_height; y++)
    {
        for (size_t x = 0; x < vram_width; x++)
        {
            position = (vram_width + 1) * y + x;
            LOG_DEBUG("x = %d", x);
            LOG_DEBUG("position = %d", position);
            LOG_DEBUG("position + y + vram_offset = %d", position + y + vram_offset);
            LOG_DEBUG("spu->ram[position + y + vram_offset] = %d", spu->ram[position + y + vram_offset] / FLOAT_COEFFICIENT);
            spu->vram[position] = spu->ram[position + y + vram_offset] / FLOAT_COEFFICIENT;
        }
        spu->vram[position] = L'\n';
    }

    #if _WIN32
    setmode(fileno(stdout), _O_U8TEXT);
    #endif
    fwrite(spu->vram, position + 1, sizeof(wchar_t), stdout);
}

void execute_program(cmd_t* code_array, struct Spu* spu)
{
    assert(code_array);
    assert(spu);

    ssize_t ip = 0;
    while (ip > -1)
    {
        LOG_TRACE("ip = %lld", ip);
        cmd_t cmd = *((uint8_t*) code_array + ip);
        LOG_DEBUG("spu->spu_stack.size = %d", spu->spu_stack.size);
        dump_stack(stderr, &spu->spu_stack, 0);
        switch (cmd & ID_MASK)
        {
            #define DEF_CMD(NAME, ARG_MASK, ...) case OPERATIONS[NAME##_enum].id: ip += sizeof(cmd_t); __VA_ARGS__; break;

            #include "../commands.inc"
            #undef DEF_CMD

            default: LOG_ERROR("There is not such command!"); return;
        }
    }
}
