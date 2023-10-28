#include <locale.h>

#include "../Libs/textlib.h"

#include "assembler.h"

const char* INPUT_FILE_NAME = "input.txt";
const char* OUTPUT_FILE_NAME = "../output.asm";

int main()
{
    setlocale(LC_ALL, "");

    File input_file = {};
    init_file(INPUT_FILE_NAME, &input_file);

    asm_error err_code = NO_ERR;
    text_to_asm(&input_file, OUTPUT_FILE_NAME);

    // printf("err_code = %d", err_code);

    destruct_file(&input_file);

    return 0;
}
