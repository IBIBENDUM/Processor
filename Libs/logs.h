#ifndef LOGS_H
#define LOGS_H

#ifdef NOTDEBUG
    #define DEBUG_MSG(FORMAT, ...)
#else
    #define DEBUG_MSG(FORMAT, ...)\
    do\
    {\
        fprintf(stderr, PAINT_TEXT(COLOR_YELLOW,"[%s, %d] %s():\n"), __FILE__, __LINE__, __func__);\
        fprintf(stderr, PAINT_TEXT(COLOR_YELLOW, FORMAT "\n"), ##__VA_ARGS__);\
    } while(0)
#endif

#endif
