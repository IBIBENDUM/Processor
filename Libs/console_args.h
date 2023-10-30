#ifndef CONSOLE_ARGS_H
#define CONSOLE_ARGS_H

struct Args_values
{
    const char* input_file;
    const char* output_file;
};

bool handle_cmd_args(const int argc, char* const* argv, const char* format, Args_values* values);
void print_help(const char* format);

#endif
