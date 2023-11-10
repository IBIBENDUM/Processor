#ifndef LOGS_H
#define LOGS_H

#ifdef NOTDEBUG
    #define DEBUG_MSG(COLOR, FORMAT, ...)
#else
    #define DEBUG_MSG(COLOR, FORMAT, ...)\
    do\
    {\
        fprintf(stderr, PAINT_TEXT(COLOR,"[%s, %d] %s():\n"), __FILE__, __LINE__, __func__);\
        fprintf(stderr, PAINT_TEXT(COLOR, FORMAT "\n"), ##__VA_ARGS__);\
    } while(0)
#endif

enum log_level
{
    LOG_LVL_TRACE,
    LOG_LVL_DEBUG,
    LOG_LVL_INFO,
    LOG_LVL_WARN,
    LOG_LVL_ERROR,
    LOG_LVL_DISABLE,
    LOG_AMOUNT_OF_LVLS
};

const char* const log_levels_strings[] =
{
    "TRACE", "DEBUG", "INFO",
    "WARN" , "ERROR", "DISABLE"
};

// BAH: Make struct for this~~~~~~~~~~~~~~~~~~>
#define LOG_TRACE(...) init_log(LOG_LVL_TRACE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) init_log(LOG_LVL_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  init_log(LOG_LVL_INFO,  __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  init_log(LOG_LVL_WARN,  __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) init_log(LOG_LVL_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

void set_log_level(enum log_level level);

void init_log(const enum log_level log_level, const char* file, const char* func, const int line, const char* format, ...);

#endif
