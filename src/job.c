#include <stdlib.h>
#include "job.h"

struct job {
    int pgid;
    struct job *next;
};

static struct job *jobs;

void job_push(int pgid) {
    struct job *j = malloc(sizeof(struct job));
    j->pgid = pgid;
    j->next = jobs;
    jobs = j;
}

int job_pop(void) {
    if (!jobs) {
        return -1;
    }
    struct job *j = jobs;
    jobs = j->next;
    int pgid = j->pgid;
    free(j);
    return pgid;
}
