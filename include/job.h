#pragma once

void job_push(int pgid);
int job_pop(void);
int job_resume_fg(int pgid);
