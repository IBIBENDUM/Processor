#ifndef TEXTLIB_H
#define TEXTLIB_H

#include <stdio.h>
#include "colors.h"

#ifdef NDEBUG
    #define DEBUG(FORMAT, ...)
#else
    #define DEBUG(FORMAT, ...)\
    do\
    {\
        printf(PAINT_TEXT(COLOR_YELLOW, FORMAT), ##__VA_ARGS__);\
    } while(0)
#endif

#define FREE_AND_NULL(ptr)\
do{\
    free(ptr);\
    ptr = NULL;\
} while(0)

#define HANDLE_ERROR(exp, message, ...)\
    do {\
        if (!(exp))\
        {\
            perror(__FILE__ ": " message);\
            return __VA_ARGS__;\
        }\
    } while(0)

const int SET_MODE_CONST = 0x00040000;

enum COMPARE_TYPE
{
    COMPARE_FORWARD = 1,
    COMPARE_BACKWARD = -1
};

typedef struct line_struct
{
    wchar_t* start;
    size_t len;
} line;

typedef struct File
{
    wchar_t* file_name;
    wchar_t* buffer;
    line* lines_ptrs;
    size_t line_amounts;
} File;

ssize_t get_file_size(const ssize_t descriptor);

size_t get_char_amount(const wchar_t* const string, const wchar_t ch);

size_t get_lines_amount(const wchar_t* const string);

File init_file(const char* file_name);

void destruct_file(File* file_struct);

// needs free()
wchar_t* read_file(const char* file_name);

// need free()
line* parse_lines_to_arr(wchar_t* string, const size_t lines_amount);

void print_line(line* line_ptr, FILE* file_ptr);

void write_lines_to_file(line* line_ptr, size_t lines_amount, FILE* file_ptr);

void write_in_dictionary_format(line* line_ptr, const size_t lines_amount, FILE* file_ptr);

void print_seperator(FILE* file_ptr);

const wchar_t* move_to_alphabet_sym(const wchar_t* str, int direction);

int compare_lines_forward(const void* line_1_ptr, const void* line_2_ptr);

int compare_lines_backward(const void* line_1_ptr, const void* line_2_ptr);

void empty_lines(line* line_ptr);

void print_tatarstan_symbolism(FILE* file_ptr);

#endif
