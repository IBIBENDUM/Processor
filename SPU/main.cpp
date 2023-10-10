#include <stdlib.h>

#include "../Libs/textlib.h"
#include "spu.h"

const char* code_file_name = "../output.bin";
int main()
{
    int* code_array = read_bin_file(code_file_name);

    execute_program(code_array);

    FREE_AND_NULL(code_array);
}
