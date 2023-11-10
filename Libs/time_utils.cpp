#include <time.h>
#include <assert.h>

#include "time_utils.h"

char* cast_time_to_str(struct tm* time)
{
    assert(time);
    static char time_buf[] = "%H:%M:%S";

    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", time);
    return time_buf;
}

char* get_current_time_str()
{
    time_t time_info = time(NULL);
    struct tm* time = localtime(&time_info);
    return cast_time_to_str(time);
}

char* get_current_date_str()
{
    time_t time_info = time(NULL);
    struct tm* time = localtime(&time_info);

    // SD SADASD A
    static char date_buf[16] = "%H:%M:%S";

    strftime(date_buf, sizeof(date_buf), "%x", time);
    return date_buf;
}
