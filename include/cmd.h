#pragma once

//struct cmd_exec {
//    int in_fd, out_fd, err_fd;
//    size_t argc;
//    char *args[12];
//};
//
//struct cmd_link {
//    struct cmd_exec cmd;
//    struct cmd_link *prev, *next;
//};

int eval(char *str);
