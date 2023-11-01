#include <stdlib.h>

#include "utils.h"
void free_and_null(void* pointer)
{
    free(pointer);
    pointer = NULL;
}
