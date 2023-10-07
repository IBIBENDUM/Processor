/*
Читает файл вида:
push 30
push 10
mul
out
hlt
и преобразовывает в команды вида
17 3
17 1
*/
#include <locale.h>

#include "assembler.h"
#include "textlib.h"

const char* INPUT_FILE_NAME = "input.txt";
const char* OUTPUT_FILE_NAME = "output.txt";

int main()
{
    setlocale(LC_ALL, "");
    // printf("%s\n", INPUT_FILE_NAME);
    File input_file = {};
    init_file(INPUT_FILE_NAME, &input_file);

    parse_file_to_commands(&input_file);
    // write_to_file(OUTPUT_FILE_NAME);

    // destruct_file(&input_file);

    return 0;
}
