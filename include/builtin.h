#pragma once

struct cmd_unit;

typedef int (*builtin_func_t) (const struct cmd_unit *);

builtin_func_t builtin_find(const char *name);
