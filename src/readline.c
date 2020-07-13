#include <sys/select.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include "history.h"

#define KEY_UP      (256)
#define KEY_DOWN    (257)
#define KEY_RIGHT   (258)
#define KEY_LEFT    (259)

// With support for escape sequences
static int getch_del(void) {
    struct timeval tv = {
        .tv_sec = 0,
        .tv_usec = 500000
    };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    int res;

    if ((res = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv)) < 0) {
        return res;
    }

    if (FD_ISSET(STDIN_FILENO, &fds)) {
        res = 0;
        if (read(STDIN_FILENO, &res, 1) != 1) {
            return -1;
        }
        return res;
    } else {
        return -1;
    }
}

static int escape_read(void) {
    // Maybe [
    int c = getch_del();

    if (c < 0) {
        return '\033';
    }

    if (c != '[') {
        return -1;
    }

    c = getch_del();

    switch (c) {
    case 'A':
        return KEY_UP;
    case 'B':
        return KEY_DOWN;
    case 'C':
        return KEY_RIGHT;
    case 'D':
        return KEY_LEFT;
    default:
        return -1;
    }
}

static int getch(void) {
    char c;

    if (read(STDIN_FILENO, &c, 1) != 1) {
        return -1;
    }

    if (c == '\033') {
        int r = escape_read();

        if (r > 0) {
            return r;
        }
    }

    return c;
}

int readline(char *buf, size_t lim) {
    int len = 0;
    int cur = 0;
    int chr;

    struct history_entry *ent = NULL;
    struct termios t0, t1;

    tcgetattr(STDIN_FILENO, &t0);
    memcpy(&t1, &t0, sizeof(t0));
    t1.c_lflag &= ~(ICANON | ECHONL | ECHO);
    t1.c_iflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &t1);

    while (1) {
        if ((chr = getch()) <= 0) {
            goto err;
        }

        if (len == lim) {
            goto err;
        }

        switch (chr) {
        case KEY_LEFT:
            if (cur) {
                fputs("\033[D", stdout);
                fflush(stdout);
                --cur;
            }
            break;
        case KEY_RIGHT:
            if (cur < len) {
                fputs("\033[C", stdout);
                fflush(stdout);
                ++cur;
            }
            break;
        case KEY_UP:
            if (!ent) {
                ent = history_head();
            } else {
                ent = ent->next;
            }

            if (ent) {
                if (cur) {
                    fprintf(stdout, "\033[%dD", cur);
                }
                fputs("\033[K", stdout);
                fputs(ent->data, stdout);
                fflush(stdout);
                len = strlen(ent->data);
                cur = len;
            } else {
                if (cur) {
                    fprintf(stdout, "\033[%dD", cur);
                }
                fputs("\033[K", stdout);
                fflush(stdout);
                cur = 0;
                len = 0;
            }
            break;
        case KEY_DOWN:
            if (ent) {
                ent = ent->prev;

                if (ent) {
                    if (cur) {
                        fprintf(stdout, "\033[%dD", cur);
                    }
                    fputs("\033[K", stdout);
                    fputs(ent->data, stdout);
                    fflush(stdout);
                    len = strlen(ent->data);
                    cur = len;
                } else {
                    if (cur) {
                        fprintf(stdout, "\033[%dD", cur);
                    }
                    fputs("\033[K", stdout);
                    fflush(stdout);
                    cur = 0;
                    len = 0;
                }
            }
            break;
        }

        if (chr == KEY_UP || chr == KEY_DOWN) {
            continue;
        }

        // Any other move resets history position
        ent = NULL;

        if (chr == 4) {
            if (len == 0) {
                fputs("exit\n", stderr);
                goto err;
            }
        }

        if (chr == '\n') {
            fputc(chr, stdout);
            buf[len] = 0;
            break;
            // TODO: cleanup this inconsistency in the kernel
        } else if (chr == '\b' || chr == 127) {
            if (cur) {
                --cur;
                for (int i = cur; i < len - 1; ++i) {
                    buf[i] = buf[i + 1];
                }
                fputs("\033[D\033[s", stdout);
                --len;
                for (int i = cur; i < len; ++i) {
                    putchar(buf[i]);
                }
                fputs(" \033[u", stdout);
                fflush(stdout);
            }
        } else if (chr >= ' ' && chr < 255) {
            fputc(chr, stdout);
            fputs("\033[s", stdout);
            for (int i = cur; i < len; ++i) {
                putchar(buf[i]);
            }
            fputs("\033[u", stdout);
            for (int i = len; i > cur; --i) {
                buf[i] = buf[i - 1];
            }
            fflush(stdout);
            buf[cur++] = chr;
            ++len;
        }
    }

    const char *e = buf;
    while (isspace(*e)) {
        ++e;
    }
    if (*e) {
        history_insert(buf);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &t0);
    return len;
err:
    tcsetattr(STDIN_FILENO, TCSANOW, &t0);
    return -1;
}

