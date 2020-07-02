#include "parse.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int cmd_unit_parse(const char *str, struct cmd_unit *unit) {
    // Parse until end of string or |
    size_t len;
    char *word = NULL;
    char c;
    unit->argc = 0;

    // TODO: escapes
    // TODO: variable substitution
    while ((c = *str++) && c != '|') {
        if (isspace(c)) {
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
