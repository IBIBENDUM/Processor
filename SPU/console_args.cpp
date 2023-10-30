#include "console_args.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>

bool handle_cmd_args(const int argc, char* const* argv, const char* format, Args_values* values)
{
    assert(argv);
    assert(argc);

    int arg = 0;

    while ((arg = getopt(argc, argv, format)) != -1)
    {
        switch (arg)
        {
            case 'i': {
                if (!values->input_file)
                    return false;

                values->input_file = optarg;
                break;
            }

            case 'h': {
                print_help(format);
                return false;
            }

            case 'o': {
                if (!values->output_file)
                    return false;

                values->output_file = optarg;
                break;
            }

            default: {
                printf("Wrong option found\n");
                print_help(format);

                return false;
            }
        }
    }

    return true;
}

void print_help(const char* format)
{
    printf("OPTIONS:\n");
    for (int i = 0; i < strlen(format); i++)
    {
        if (format[i] != ':')
        {
            const char arg = format[i];
            switch (arg)
            {
                case 'h':   printf("-h             Display help message\n"); break;
                case 'i':   printf("-i             Choose input file name\n"); break;
                default: printf(" PARSE ARG ERR"); break; // ???
            }
        }
    }
}

