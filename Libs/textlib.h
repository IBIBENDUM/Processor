#ifndef TEXTLIB_H
#define TEXTLIB_H

#include <stdio.h>
#include <wchar.h>

#include "colors.h"
#include "logs.h"

#ifdef TL_DEBUG
    #define TL_DEBUG_MSG(...) DEBUG_MSG(COLOR_YELLOW, __VA_ARGS__)
#else
    #define TL_DEBUG_MSG(...)
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

enum File_read_mode
{
    BIN,
    TEXT
};

enum Compare_type
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
    const char* file_name;
    wchar_t* buffer;
    line* lines_ptrs;
    size_t line_amount;
} File;

ssize_t get_file_size(const ssize_t descriptor);

size_t get_char_amount(const wchar_t* const string, const wchar_t ch);

size_t get_lines_amount(const wchar_t* const string);

void replace_with_zero(line* line_ptr, const wchar_t symbol);

void tokenize_lines(File* file);

bool init_file(const char* file_name, File* file);

void destruct_file(File* file_struct);

// needs free()
wchar_t* read_file(const char* file_name, enum File_read_mode mode);

// need free()
line* parse_lines_to_arr(wchar_t* string, const size_t lines_amount);

void print_line(line* line_ptr, FILE* file_ptr);

void write_lines_to_file(line* line_ptr, size_t lines_amount, FILE* file_ptr);

void write_in_dictionary_format(line* line_ptr, const size_t lines_amount, FILE* file_ptr);

void print_separator(FILE* file_ptr);

const wchar_t* move_to_alphabet_sym(const wchar_t* str, int direction);

int compare_lines_forward(const void* line_1_ptr, const void* line_2_ptr);

int compare_lines_backward(const void* line_1_ptr, const void* line_2_ptr);

void empty_lines(line* line_ptr);

wchar_t* get_word(wchar_t* string, size_t* word_len);

size_t cscspn (wchar_t* source, const wchar_t* ref);

wchar_t* move_to_non_space_sym(wchar_t* str);

void print_tatarstan_symbolism(FILE* file_ptr);

#endif
