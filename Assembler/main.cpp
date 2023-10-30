#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <assert.h>

#include "../Libs/textlib.h"
#include "../Libs/logs.h"
#include "assembler.h"
#include "../Libs/console_args.h"

const char* file_in_name = "input.s";
const char* file_out_name = "../output.asm";

int main(const int argc, char* const* argv)
{
    Args_values values =
    {
        .input_file = file_in_name,
        .output_file = file_out_name
    };

    if (!handle_cmd_args(argc, argv, "i:o:h", &values))
        return 1;

    setlocale(LC_ALL, "");

    set_log_level(LOG_LVL_INFO);

    File input_file = {};
    init_file(values.input_file, &input_file);

    asm_error err = ASM_NO_ERR;

    err = text_to_asm(&input_file, values.output_file);

    if (err != ASM_NO_ERR)
        LOG_ERROR("err = %s", asm_errors_strs[err]);
    else
        LOG_INFO("%s", asm_errors_strs[err]);

    destruct_file(&input_file);

    return 0;
}
