#pragma once
#define HISTORY_SIZE        128

struct history_entry {
    char data[256];
    struct history_entry *prev, *next;
};

void history_insert(const char *cmd);
struct history_entry *history_head(void);
