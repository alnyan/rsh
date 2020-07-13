#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "history.h"

static struct history_entry *g_history_head, *g_history_tail;
size_t history_size = 0;

struct history_entry *history_head(void) {
    return g_history_head;
}

void history_insert(const char *command) {
    struct history_entry *ent;
    if (history_size == HISTORY_SIZE) {
        // Just resuse the last entry
        assert(g_history_tail && g_history_tail->prev);
        ent = g_history_tail;
        g_history_tail = g_history_tail->prev;
        g_history_tail->next = NULL;
    } else {
        ent = malloc(sizeof(struct history_entry));
        assert(ent);
    }

    strcpy(ent->data, command);
    ent->prev = NULL;
    ent->next = g_history_head;
    if (!g_history_tail) {
        assert(!g_history_head);
        g_history_tail = ent;
    } else {
        g_history_head->prev = ent;
    }
    g_history_head = ent;
    ++history_size;
}
