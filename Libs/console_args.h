#ifndef CONSOLE_ARGS_H
#define CONSOLE_ARGS_H

#include "logs.h"

struct Args_values
{
    const char* input_file_name;
    const char* output_file_name;
    enum log_level log_level;
};

bool handle_cmd_args(const int argc, char* const* argv, const char* format, Args_values* values);
void print_help(const char* format);

#endif
