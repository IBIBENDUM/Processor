#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <assert.h>
#include <sys/stat.h>
#include <cwchar>
#include <unistd.h>
#include <fcntl.h>

#include "textlib.h"

ssize_t get_file_size(const ssize_t descriptor)
{
    struct stat file_info = {};
    if (fstat((int) descriptor, &file_info) != -1)
        return (ssize_t) file_info.st_size;

    return -1;
}

size_t get_char_amount(const wchar_t* const string, const wchar_t ch)
{
    assert(string);

    size_t amount = 0;
    for (const wchar_t* ch_ptr = string; (ch_ptr = wcschr(ch_ptr, ch)); amount++, ch_ptr++); // () for remove warning
    return amount;
}

size_t get_lines_amount(const wchar_t* const string)
{
    return get_char_amount(string, '\n') + 1;
}

size_t get_line_len(const wchar_t* string)
{
    return wcscspn(string, L"\n") + 1;
}

wchar_t* read_file(const char* file_name, enum File_read_mode mode)
{
    FILE* file_ptr = fopen(file_name, "rb");
    #if _WIN32
    if (mode == TEXT)
        setmode(fileno(file_ptr), SET_MODE_CONST);
    #endif
    HANDLE_ERROR(file_ptr, "Couldn't open file", NULL);
    TL_DEBUG_MSG("File opened\n");

    const ssize_t descriptor = fileno(file_ptr);
    HANDLE_ERROR(descriptor != -1, "Couldn't get file descriptor", NULL);

    const ssize_t size = get_file_size(descriptor);
    HANDLE_ERROR(size != -1, "Couldn't get file size", NULL);
    TL_DEBUG_MSG("Got size\n");

    wchar_t* buffer = (wchar_t*) calloc(size + 1, sizeof(wchar_t));
    HANDLE_ERROR(buffer, "Error at memory allocation", NULL);
    TL_DEBUG_MSG("Buffer allocated\n");

    const size_t fread_ret_val = fread(buffer, sizeof(wchar_t), size, file_ptr);
    HANDLE_ERROR(fread_ret_val, "Error at file reading", NULL);
    TL_DEBUG_MSG("File read\n");

    const size_t fclose_ret_val = fclose(file_ptr);
    file_ptr = NULL;
    HANDLE_ERROR(!fclose_ret_val, "Error at closing file", NULL);
    TL_DEBUG_MSG("File closed\n");

    return buffer;
}

void replace_with_zero(line* line_ptr, const wchar_t symbol)
{
    assert(line_ptr);

    wchar_t* symbol_ptr = wcschr(line_ptr->start, symbol);
    if (symbol_ptr)
        *symbol_ptr = L'\0';
}

static void initialize_line(line* line_ptr, wchar_t* string, const size_t len)
{
    line_ptr->start = string;
    line_ptr->len = len;
}

static void empty_one_line(line* line_ptr)
{
    initialize_line(line_ptr, NULL, 0);
}

void empty_lines(line* lines_ptrs, size_t size)
{
    line* line_ptr = lines_ptrs;
    for (size_t i = 0; i < size; i++)
    {
        empty_one_line(line_ptr + i);
    }
}

// Needs free()
// add const
line* parse_lines_to_arr(wchar_t* string, const size_t lines_amount)
{
    assert(string);

    line* lines_ptrs = (line*) calloc(lines_amount + 1, sizeof(lines_ptrs[0]));
    HANDLE_ERROR(lines_ptrs, "Error at memory allocation", NULL);

    line* line_ptr = lines_ptrs;
    wchar_t* str_ptr = string;

    size_t i = 0;
    do
    {
        size_t line_length = get_line_len(str_ptr);
        initialize_line(&line_ptr[i], str_ptr, line_length);

        str_ptr += line_length; // move to symbol after \n
        i++;
    }
    while (i < lines_amount);

    return lines_ptrs;
}

bool init_file(const char* file_name, File* file)
{
    assert(file_name);
    assert(file);

    wchar_t* buffer = read_file(file_name, TEXT);
    HANDLE_ERROR(buffer, "Error at buffering file", false);

    size_t lines_amount = get_lines_amount(buffer);

    line* lines_ptrs = parse_lines_to_arr(buffer, lines_amount);
    HANDLE_ERROR(lines_ptrs, "Error at lines parsing", false);

    file->buffer = buffer;
    file->file_name = file_name;
    file->lines_ptrs = lines_ptrs;
    file->line_amount = lines_amount;

    return true;
}

// TODO: Realization with line* (without file)
void tokenize_lines(File* file)
{
    assert(file);

    for (size_t i = 0; i < file->line_amount; i++)
    {
        line* line_ptr = file->lines_ptrs + i;
        *(line_ptr->start + line_ptr->len - 1) = 0;
    }
}

void destruct_file(File* file_struct)
{
    assert(file_struct);

    empty_lines(file_struct->lines_ptrs, file_struct->line_amount);
    FREE_AND_NULL(file_struct->buffer);
    FREE_AND_NULL(file_struct->lines_ptrs);
}

static bool check_alphabet_line(line* line_ptr)
{
    assert(line_ptr);

    wchar_t* string = line_ptr->start;
    for (size_t i = 0; i < line_ptr->len; i++)
    {
        if (iswalpha(string[i]))
            return true;
    }
    return false;
}

void print_line(line* line_ptr, FILE* file_ptr)
{
    assert(line_ptr);
    assert(file_ptr);

    if (check_alphabet_line(line_ptr))
    {
        size_t len = line_ptr->len;
        if (len > 2) // Dont print empty lines
            fwrite(line_ptr->start, sizeof(wchar_t), len, file_ptr);
    }
}

void write_lines_to_file(line* line_ptr, size_t lines_amount, FILE* file_ptr)
{
    assert(line_ptr);
    assert(file_ptr);
    TL_DEBUG_MSG("%s:\n", __PRETTY_FUNCTION__);
    for (size_t i = 0; i < lines_amount; i++)
        print_line(line_ptr + i, file_ptr);
}

static void write_dictionary_separator(const wchar_t symbol, FILE* file_ptr)
{
    assert(file_ptr);

    fwprintf(file_ptr, L"\n----------------------\n%c\n", towupper(symbol));
}

void print_separator(FILE* file_ptr)
{
    assert(file_ptr);

    fwprintf(file_ptr, L"--------------------------------------\n");
}

void write_in_dictionary_format(line* line_ptr, const size_t lines_amount, FILE* file_ptr)
{
    assert(line_ptr);
    assert(file_ptr);

    wchar_t prev_symbol = *move_to_alphabet_sym(line_ptr[0].start, COMPARE_FORWARD);
    // write_dictionary_separator(prev_symbol, file_ptr);
    for (size_t i = 0; i < lines_amount; i++)
    {
        if (check_alphabet_line(&line_ptr[i]))
        {

            size_t len = line_ptr[i].len;
            if (len > 2)
            {
                wchar_t symbol = *move_to_alphabet_sym(line_ptr[i].start, COMPARE_FORWARD);
                if (symbol != prev_symbol)
                {
                    write_dictionary_separator(symbol, file_ptr);
                    prev_symbol = symbol;
                }
                // fwprintf(file_ptr, L"%s\n", line_ptr[i].start);
                fwrite(line_ptr[i].start, sizeof(wchar_t), len, file_ptr);
            }
        }
    }
}

const wchar_t* move_to_alphabet_sym(const wchar_t* str, const int direction)
{
    assert(str);

    while (*(str) != L'\n' && !iswalpha(*(str)))
        str += direction;

    return str;
}

wchar_t* move_to_non_space_sym(wchar_t* str)
{
    assert(str);

    while (*(str) != L'\n' && *(str) != L'\0' && iswspace(*(str)))
        str++;

    return str;
}

wchar_t* get_word(wchar_t* string, size_t* word_len_ptr)
{
    static wchar_t* backup_string;

    if (!string)
        string = backup_string;

    if (!string)
        return NULL;

    string = move_to_non_space_sym(string);
    size_t word_len = wcscspn(string, L" ");
    if (word_len_ptr)
        *word_len_ptr = word_len;

    if (*(string + word_len) == '\0')
        backup_string = NULL;
    else
        backup_string = string + word_len + 1;

    return string;
}

static int compare_lines(const wchar_t* line_1_ptr, const wchar_t* line_2_ptr, const int direction)
{
    assert(line_1_ptr);
    assert(line_2_ptr);

    // Skip leading non alphabet symbols
    const wchar_t* line_1 = move_to_alphabet_sym(line_1_ptr, direction);
    const wchar_t* line_2 = move_to_alphabet_sym(line_2_ptr, direction);

    while (*line_1 == *line_2)
    {
        if (*line_1 == L'\n')
        {
            TL_DEBUG_MSG("return 0\n");
            TL_DEBUG_MSG("--------------------------\n");
            return 0;
        }
        TL_DEBUG_MSG("ch1 = %d ch2 = %d\n", *line_1, *line_2);
        line_1 += direction;
        line_2 += direction;
    }

    TL_DEBUG_MSG("return %d\n", *line_1 - *line_2);
    TL_DEBUG_MSG("--------------------------\n");
    return *line_1 - *line_2;
}

int compare_lines_forward(const void* line_1_ptr_void, const void* line_2_ptr_void)
{
    assert(line_1_ptr_void);
    assert(line_2_ptr_void);

    const line* line_1_ptr = (const line*) line_1_ptr_void;
    const line* line_2_ptr = (const line*) line_2_ptr_void;

    return compare_lines(line_1_ptr->start, line_2_ptr->start, COMPARE_FORWARD);
}

int compare_lines_backward(const void* line_1_ptr_void, const void* line_2_ptr_void)
{
    assert(line_1_ptr_void);
    assert(line_2_ptr_void);

    const line* line_1_ptr = (const line*) line_1_ptr_void;
    const line* line_2_ptr = (const line*) line_2_ptr_void;

    return compare_lines(line_1_ptr->start + line_1_ptr->len - 2, line_2_ptr->start + line_2_ptr->len - 2, COMPARE_BACKWARD);
}

void print_tatarstan_symbolism(FILE* file_ptr)
{
    fwprintf(file_ptr, L"                                    .,****,,.\n\
                          //%%%%%%%%%%%%%%%%%%%%%%/*(%%%%%%%%%%%%%%%%%%%%#/,\n\
                     /%%%%%%%%#*///#*#/%%%%%%(/#*%%%%%%(/(*#*///#%%%%%%#/\n\
                 /%%%%%%#/##///#%%%%%%%%%%##//*****/(##%%%%%%%%%%((/#(#(#%%%%#*\n\
              /%%%%#**#/(#%%%%#/,                       *(%%%%%%(#//*/#%%%%/\n\
           /%%%%#/#//%%%%%%(,       */(#############//.       *#%%%%#/%%#/%%%%#,\n\
         /%%%%//*((%%%%/     ./########%%%%%%################/     .(%%%%/#*/(%%%%,\n\
       *%%%%((#/%%%%(.    *###########/######################(,    *%%%%(/#/#%%#,\n\
      #%%(#//%%%%(    ,########&#%%(.#(/##%%%%########.  /########/    ,#%%#(//%%%%/\n\
    *%%%%//(/%%#.   *###############(*(,/ #########/  .##########(.   *%%%%///(%%%%\n\
   *%%%%/(/#%%/   .###################/(.%%(%% #######%%    ##########/    #%%(#(/%%%%.\n\
  *%%#/%%/%%%%*   *###########(%%#%%#####/,#%%*%%,##########%%     *%%######.   (%%#/#/%%#.\n\
 .#%%#//#%%*   *#######          /####,#.(/*@ #############     %%####.   (%%/%%*#%%(\n\
 /%%#/#/%%(   ,########&%%.     ,   ###*%%*(,(*,################   /###(   ,%%%%/(/%%%%*\n\
 #%%/(/%%%%,   (#############*       #/ %%.(,# &(,%%/         (##(   ####*   (%%#*##%%/\n\
,%%%%(*/%%%%.  .###############         ,* / (*/(*.               (#####(   *%%#/(/%%#\n\
*%%%%//#%%#   ,###############         &/*///%%#*%%(              .######(   ,%%##*(%%#\n\
,%%%%///%%%%.   ###############.      ,*/%%/.#,(/%%/   ./          /######/   *%%##*/%%#\n\
 #%%#*/%%%%*   /####.         .      ((((/#*#/%%# ,###(   ,      %%######*   (%%#*/#%%/\n\
 /%%(((*%%(   ,###&,,/######   / #   /*(/#/(/%%#######(   %%     ######(   ,%%#/(/%%%%,\n\
  #%%#//#%%/   ,#################     #(############%%   ,##(   (####(    #%%(//%%%%/\n\
  ,%%%%/#/%%%%/   ,###############%%   .#############,  #######   .###(    #%%#//#%%(\n\
   *%%%%/#/(%%/    (#############.  #########%% /.  %%########*  (###*   .%%%%//(#%%#\n\
    ,%%%%#*/%%%%%%,   .(##########,  &###################. ,*   ###/    /%%%%#//%%%%(\n\
      (%%#///%%%%#.    /###(.*   ,####################%%%%   ####/    *%%%%##*/%%%%*\n\
       *%%%%(/#/(%%#*    .(#################################/     /%%%%/((##%%(\n\
         *%%%%#///(%%%%(,     ,/#########################/.     *#%%#////#%%#.\n\
           ,#%%%%/*#*#%%(#/        ,//(#########(//.       ,(%%/%%(*#*(%%%%/\n\
              /%%%%%%//%%%%#/%%%%%%#*,                     */#%%%%%%(/%%%%/#%%%%#,\n\
                 *#%%%%((%%%%%%//%%%%%%%%%%%%%%%%#########%%%%%%%%%%%%%%/%%%%%%#(%%/%%%%/.\n\
                     ,/%%%%%%#/%%%%#(%%%%%%/%%%%%%/#/%%*%%(%%%%(#%%%%/%%/#%%%%/\n\
                           //%%%%%%%%###/%%%%/%%%%%%#//%%%%%%%%%%#/*");

}
