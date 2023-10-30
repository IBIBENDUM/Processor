#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>

#include "../Libs/textlib.h"
#include "spu.h"

static bool handle_cmd_args(const int argc, char* const* argv);
static void print_help();

const char* code_file_name = "../output.asm";

int main(const int argc, char* const* argv)
{
    if (handle_cmd_args(argc, argv))
        return 1;

    cmd_t* code_array = (cmd_t*) read_bin_file(code_file_name);

    Spu spu = {};

    construct_spu(&spu);

    execute_program(code_array, &spu);

    destruct_spu(&spu);
    FREE_AND_NULL(code_array);
}


static bool handle_cmd_args(const int argc, char* const* argv)
{
    assert(argv);
    assert(argc);

    int arg = 0;

    while ((arg = getopt(argc, argv, "i:h")) != -1)
    {
        switch (arg)
        {
            case 'i': {
                code_file_name = optarg;
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
}

