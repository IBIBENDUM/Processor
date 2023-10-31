#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>

#include "logs.h"
#include "colors.h"

log_level current_log_level = LOG_LVL_TRACE;

const char* log_levels_colors[] =
{
    COLOR_LIGHT_GRAY, COLOR_LIGHT_YELLOW, COLOR_LIGHT_CYAN,
    COLOR_LIGHT_MAGENTA, COLOR_LIGHT_RED
};

struct Log_event
{
    enum log_level level;
    const char* format;
    const char* file;
    const char* func;
    const int line;
    FILE* output;
    struct tm* time;
    va_list args;
};

static char* cast_time_to_str(struct tm* time)
{
    assert(time);
    static char time_buf[] = "%H:%M:%S";

    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", time);
    return time_buf;
}

static void init_event(Log_event* log_info, FILE* output_ptr)
{
    assert(log_info);
    assert(output_ptr);

    time_t time_info = time(NULL);
    log_info->time = localtime(&time_info);
    log_info->output = output_ptr;
}

static void write_log(Log_event* log_info)
{
    assert(log_info);

    const char* time_str = cast_time_to_str(log_info->time);
    fprintf(log_info->output, COLOR_DARK_GRAY "[%s] %s%-5s: " COLOR_WHITE "%s:%s():%d:\n" TEXT_RESET, time_str, log_levels_colors[log_info->level], log_levels_strings[log_info->level], log_info->file, log_info->func, log_info->line);
    vfprintf(log_info->output, log_info->format, log_info->args);
    fprintf(log_info->output, "\n\n");
}

void set_log_level(enum log_level level)
{
    current_log_level = level;
}

void init_log(const log_level log_level, const char* file, const char* func, const int line, const char* format, ...)
{
    assert(file);
    assert(func);
    assert(line);
    assert(format);

    struct Log_event log_info =
    {
        .level  = log_level,
        .format = format,
        .file   = file,
        .func   = func,
        .line   = line
    };

    if (log_level >= current_log_level)
    {
        init_event(&log_info, stderr);
        va_start(log_info.args, format);
        write_log(&log_info);
        va_end(log_info.args);
    }
}
