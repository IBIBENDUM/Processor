#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <assert.h>

#include "../Libs/textlib.h"
#include "../Libs/logs.h"
#include "assembler.h"

const char* file_in_name = "input.s";
const char* file_out_name = "../output.asm";

static bool handle_cmd_args(const int argc, char* const* argv);
static void print_help();

int main(const int argc, char* const* argv)
{
    if (handle_cmd_args(argc, argv))
        return 1;

    setlocale(LC_ALL, "");

    set_log_level(LOG_LVL_INFO);

    File input_file = {};
    init_file(file_in_name, &input_file);

    asm_error err = ASM_NO_ERR;

    err = text_to_asm(&input_file, file_out_name);

    if (err != ASM_NO_ERR)
        LOG_ERROR("err = %s", asm_errors_strs[err]);
    else
        LOG_INFO("%s", asm_errors_strs[err]);

    destruct_file(&input_file);

    return 0;
}


// MAKE IN OTHER FILE
static bool handle_cmd_args(const int argc, char* const* argv)
{
    assert(argv);
    assert(argc);

    int arg = 0;

    while ((arg = getopt(argc, argv, "i:o:h")) != -1)
    {
        switch (arg)
        {
            case 'i': {
                file_in_name = optarg;
                break;
            }

            case 'o': {
                file_out_name = optarg;
                break;
            }

            case 'h': {
                print_help();
                return 1;
            }

            default: {
                printf("Wrong option found\n");
                print_help();

                return 1;
            }
        }
    }

    return 0;
}

static void print_help()
{
    printf("OPTIONS:\n");
    printf("-h             Display help message\n");
    printf("-i             Choose input file name\n");
    printf("-o             Choose output file name\n");
}

