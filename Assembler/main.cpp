#include <locale.h>

#include "../Libs/textlib.h"
#include "assembler.h"

const char* INPUT_FILE_NAME = "input.txt";
const char* OUTPUT_FILE_NAME = "../output.bin";

int main()
{
    setlocale(LC_ALL, "");
    File input_file = {};
    init_file(INPUT_FILE_NAME, &input_file);

    size_t size = 0;
    int* code_array = parse_file_to_commands(&input_file, &size);
    write_to_file(OUTPUT_FILE_NAME, code_array, size);

    destruct_file(&input_file);

    return 0;
}
