#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <assert.h>

#include "assembler.h"
#include "../Libs/textlib.h"
#include "../Libs/logs.h"
#include "../Libs/console_args.h"

const char* file_out_name = "../Examples/output.asm";

int main(const int argc, char* const* argv)
{
    Args_values values =
    {
        .output_file_name = file_out_name,
    };

    if (!handle_cmd_args(argc, argv, "i:o:m:h", &values))
        return 1;

    setlocale(LC_ALL, "");
    set_log_level(LOG_LVL_DEBUG);

    asm_error err = ASM_NO_ERR;
    err = file_to_asm(values.input_file_name, values.output_file_name);

    if (err != ASM_NO_ERR)
        LOG_ERROR("err = %s", asm_errors_strs[err]);
    else
        LOG_INFO("%s", asm_errors_strs[err]);

    return 0;
}
