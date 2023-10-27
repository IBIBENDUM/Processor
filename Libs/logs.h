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

#endif
