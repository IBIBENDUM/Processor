#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>

#include "../Libs/utils.h"
#include "../Libs/textlib.h"
#include "../Libs/console_args.h"
#include "spu.h"

int main(const int argc, char* const* argv)
{
    Args_values values = {};

    if (!handle_cmd_args(argc, argv, "i:m:h", &values))
        return 1;

    LOG_INFO("code_file_name = %s", values.input_file_name);

    cmd_t* code_array = (cmd_t*) read_file(values.input_file_name, BIN);
    if (!code_array)
    {
        LOG_ERROR("Error at code array file opening");
        return 0;
    }
    Spu spu = {};

    construct_spu(&spu);

    execute_program(code_array, &spu);

    destruct_spu(&spu);
    free_and_null(code_array);
}
