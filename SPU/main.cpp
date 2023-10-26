#include <stdlib.h>
#include <inttypes.h>

#include "../Libs/textlib.h"
#include "spu.h"

const char* code_file_name = "../output.asm";
int main()
{
    cmd_t* code_array = (cmd_t*) read_bin_file(code_file_name);
    construct_spu();

    execute_program(code_array);

    destruct_spu();
    FREE_AND_NULL(code_array);
}
