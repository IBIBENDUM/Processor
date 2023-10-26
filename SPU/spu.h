#ifndef SPU_H
#define SPU_H

#include "../common.h"
#include "../Libs/textlib.h"
#include "../Libs/stack.h"

const int FLOAT_COEFFICIENT = 100;

void construct_spu();

void destruct_spu();

void execute_program(cmd_t* code_array);

#endif
