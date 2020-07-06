#pragma once
#include <sys/types.h>

extern int last_status;

ssize_t var_paste(char *dst, const char *name, size_t limit);
