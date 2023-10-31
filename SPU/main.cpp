#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>

#include "../Libs/textlib.h"
#include "../Libs/console_args.h"
#include "spu.h"

// const char* code_file_name = "../output.asm";

int main(const int argc, char* const* argv)
{
    Args_values values =
    {
        // .input_file = code_file_name
    };

    if (!handle_cmd_args(argc, argv, "i:h", &values))
        return 1;

    LOG_INFO("code_file_name = %s", values.input_file);
    set_log_level(LOG_LVL_ERROR);

    cmd_t* code_array = (cmd_t*) read_file(values.input_file, BIN);
    if (!code_array)
    {
        LOG_ERROR("Error at code array file opening");
        return 0;
    }
    Spu spu = {};

    construct_spu(&spu);

    execute_program(code_array, &spu);

    destruct_spu(&spu);
    FREE_AND_NULL(code_array);
}
