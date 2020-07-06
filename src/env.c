#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int last_status = 0;

ssize_t var_paste(char *dst, const char *name, size_t limit) {
    size_t l;
    // Special variables
    if (name[0] && !name[1]) {
        switch (name[0]) {
        case '$':
            l = snprintf(dst, limit, "%d", getpid());
            // Output was truncated
            if (l > limit) {
                l = limit;
            }
            return l;
        case '?':
            l = snprintf(dst, limit, "%d", last_status);
            // Output was truncated
            if (l > limit) {
                l = limit;
            }
            return l;
        default:
            break;
        }
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

