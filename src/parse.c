#include "parse.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static ssize_t var_paste(char *dst, const char *name, size_t limit) {
    size_t l;
    if (!strcmp(name, "$")) {
        size_t l = snprintf(dst, limit, "%d", getpid());
        // Output was truncated
        if (l > limit) {
            l = limit;
        }
        return l;
    }
    const char *e = getenv(name);
    if (!e) {
        return 0;
    }
    l = strlen(e);
    // Output was truncated
    if (l > limit) {
        l = limit;
    }
    strncpy(dst, e, limit);
    return l;
}

static int cmd_unit_parse(const char *str, struct cmd_unit *unit) {
    // Parse until end of string or |
    size_t len;
    char *word = NULL;
    char var[64];
    size_t varlen;
    char c;
    unit->argc = 0;

    // TODO: escapes
    while ((c = *str++) && c != '|') {
        if (c == '$') {
            varlen = 0;
            switch ((c = *str)) {
            case '$':
            case '?':
                varlen = 1;
                var[0] = c;
                ++str;
                break;
            case '{':
                ++str;
                while ((c = *(str++)) && c != '}') {
                    var[varlen++] = c;
                }
                break;
            default:
                while ((c = *str) && isalnum(c)) {
                    var[varlen++] = c;
                    ++str;
                }
                break;
            }
            var[varlen] = 0;

            if (!word) {
                word = malloc(256);
                len = 0;
            }

            ssize_t res = var_paste(word + len, var, 255 - len);
            if (res > 0) {
                len += res;
            }
        } else if (isspace(c)) {
            if (word) {
                word[len] = 0;
                unit->args[unit->argc++] = word;
                word = NULL;
            }
        } else {
            if (!word) {
                word = malloc(256);
                len = 0;
            }

            if (len == 255) {
                return -1;
            }
            word[len++] = c;
        }
    }

    if (word) {
        word[len] = 0;
        unit->args[unit->argc++] = word;
    }

    unit->args[unit->argc] = NULL;

    return 0;
}

int cmd_parse(const char *str, struct cmd *cmd) {
    const char *p = str;
    const char *e;

    cmd->last = NULL;

    while (1) {
        while (isspace(*p)) ++p;
        if (!*p) {
            break;
        }

        // TODO: escaped/quoted |
        //  and  ||
        e = strchr(p, '|');

        if (p != e) {
            struct cmd_unit *unit = malloc(sizeof(struct cmd_unit));
            if (cmd_unit_parse(p, unit) != 0) {
                // TODO: cleanup
                return -1;
            }

            unit->next = NULL;
            if (cmd->last) {
                cmd->last->next = unit;
            } else {
                cmd->first = unit;
            }
            unit->prev = cmd->last;
            cmd->last = unit;
        }

        if (!e) {
            break;
        }
        p = e + 1;
    }

    return 0;
}
