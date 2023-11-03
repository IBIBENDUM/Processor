#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <assert.h>

#include "assembler.h"
#include "../Libs/textlib.h"
#include "../Libs/logs.h"
#include "../Libs/console_args.h"

const char* file_out_name = "../Examples/Binaries/output.asm";

int main(const int argc, char* const* argv)
{
    Args_values values =
    {
        .output_file_name = file_out_name,
    };

    if (!handle_cmd_args(argc, argv, "i:o:m:h", &values))
        return 1;

    set_log_level(LOG_LVL_TRACE);
    asm_error err = ASM_NO_ERR;
    err = compile_file(values.input_file_name, values.output_file_name);
    print_asm_error(err);

    return 0;
}
