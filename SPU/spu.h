#ifndef SPU_H
#define SPU_H

#include "../Libs/textlib.h"
#include "../Libs/stack.h"

const int FLOAT_COEFFICIENT = 100;

void construct_spu();

void destruct_spu();

void execute_program(int* code_array);

#endif
