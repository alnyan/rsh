#pragma once

#define COLOR_RED       "\033[31m"
#define COLOR_GREEN     "\033[32m"
#define COLOR_YELLOW    "\033[33m"
#define COLOR_MAGENTA   "\033[35m"
#define COLOR_CYAN      "\033[36m"
#define COLOR_RESET     "\033[0m"

// TODO: take this from env (kernel does not support env yet)
#define PATH            "/bin"
#define PS1             COLOR_MAGENTA "%u" COLOR_RESET \
                        "@" \
                        COLOR_CYAN "%h" COLOR_RESET " " \
                        COLOR_YELLOW "%d" COLOR_RESET \
                        " %# "

