#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>

char* cast_time_to_str(struct tm* time);

char* get_current_time_str();

char* get_current_date_str();

#endif
