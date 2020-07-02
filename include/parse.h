#pragma once

struct cmd_unit {
    int fds[3];
    int pid, res;
    int argc;
    char *args[64];
    struct cmd_unit *prev, *next;
};

struct cmd {
    struct cmd_unit *first, *last;
};

int cmd_parse(const char *str, struct cmd *cmd);
