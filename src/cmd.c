#include <sys/termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>

#include "builtin.h"
#include "config.h"
#include "parse.h"
#include "cmd.h"

#define CMD_NOFORK      (1 << 0)

typedef int (*spawn_func_t) (void *arg, struct cmd_unit *cmd);
extern char **environ;

static int spawn_binary(void *arg, struct cmd_unit *cmd) {
    return execve(arg, (char *const *) cmd->args, environ);
}

static int spawn_builtin(void *arg, struct cmd_unit *cmd) {
    builtin_func_t func = arg;
    return func(cmd);
}

static int cmd_spawn(spawn_func_t fn, void *arg, struct cmd_unit *cmd, int *pgid, int flags) {
    int pid;
    int pipe_fds[2];

    if (cmd->next) {
        // Create pipe pair for this -> next communication
        int res = pipe(pipe_fds);
        if (res != 0) {
            return -1;
        }

        // this stdout = write end
        cmd->fds[1] = pipe_fds[1];
        // next stdin = read end
        cmd->next->fds[0] = pipe_fds[0];
    }

    if (flags & CMD_NOFORK) {
        int old_fd0 = -1;
        int old_fd1 = -1;
        int res;

        if (cmd->prev) {
            old_fd0 = dup(STDIN_FILENO);
            dup2(cmd->fds[0], STDIN_FILENO);
            close(cmd->fds[0]);
        }
        if (cmd->next) {
            old_fd1 = dup(STDOUT_FILENO);
            dup2(cmd->fds[1], STDOUT_FILENO);
            close(cmd->fds[1]);
        }

        res = fn(arg, cmd);

        if (cmd->prev) {
            dup2(old_fd0, STDIN_FILENO);
            close(old_fd0);
        }

        if (cmd->next) {
            dup2(old_fd1, STDOUT_FILENO);
            close(old_fd1);
        }

        return res;
    } else {
        if ((pid = fork()) < 0) {
            perror("fork()");
            return -1;
        }
    }

    if (pid == 0) {
        pid = getpid();
        if (*pgid == -1) {
            *pgid = pid;
        }
        setpgid(pid, *pgid);

        if (cmd->prev) {
            dup2(cmd->fds[0], STDIN_FILENO);
            close(cmd->fds[0]);
        }
        if (cmd->next) {
            dup2(cmd->fds[1], STDOUT_FILENO);
            close(cmd->fds[1]);
        }

        if (isatty(STDIN_FILENO)) {
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(STDIN_FILENO, *pgid);
        }

        exit(fn(arg, cmd));
    } else {
        // Store pgid on parent side
        if (*pgid == -1) {
            *pgid = pid;
        }
        cmd->pid = pid;
        if (cmd->next) {
            // Close write end, it's now handled by child
            close(pipe_fds[1]);
        }
        if (cmd->prev) {
            // Close read end, it's now handled by child
            close(cmd->fds[0]);
        }
        return 0;
    }
}

static int cmd_exec_binary(struct cmd_unit *cmd, int *pgid) {
    char path_path[256];
    int res;

    if (cmd->args[0][0] == '.' || cmd->args[0][0] == '/') {
        if ((res = access(cmd->args[0], X_OK)) != 0) {
            perror(cmd->args[0]);
            return res;
        }

        strcpy(path_path, cmd->args[0]);
        return cmd_spawn(spawn_binary, path_path, cmd, pgid, 0);
    }

    const char *pathvar = getenv("PATH");
    if (!pathvar || !*pathvar) {
        return -1;
    }

    const char *p = pathvar;
    const char *e;
    while (1) {
        e = strchr(p, ':');
        size_t len;
        if (!e) {
            len = strlen(p);
        } else {
            len = e - p;
        }

        strncpy(path_path, p, len);
        path_path[len] = '/';
        strcpy(path_path + len + 1, cmd->args[0]);

        if ((res = access(path_path, X_OK)) == 0) {
            return cmd_spawn(spawn_binary, path_path, cmd, pgid, 0);
        }

        if (!e) {
            break;
        }
        p = e + 1;
    }

    return -1;
}

static int cmd_unit_exec(struct cmd_unit *cmd, int *pgid) {
    int res;
    builtin_func_t builtin;

    if ((builtin = builtin_find(cmd->args[0])) != NULL) {
        return cmd_spawn(spawn_builtin, builtin, cmd, pgid, CMD_NOFORK);
    }

    if ((res = cmd_exec_binary(cmd, pgid)) == 0) {
        return 0;
    }

    printf("sh: command not found: %s\n", cmd->args[0]);
    return -1;
}

int eval(char *str) {
    char *p;
    struct cmd cmd;
    int cmd_res, pgid;

    while (isspace(*str)) {
        ++str;
    }

    if ((p = strchr(str, '#'))) {
        *p = 0;
    }

    if (!*str) {
        return 0;
    }

    if (cmd_parse(str, &cmd) != 0) {
        return -1;
    }

    // Spawn all subprocesses
    pgid = -1;
    for (struct cmd_unit *u = cmd.first; u; u = u->next) {
        u->pid = -1;
        cmd_unit_exec(u, &pgid);
    }

    // Wait for spawned subprocesses to finish
    for (struct cmd_unit *u = cmd.first; u; u = u->next) {
        if (u->pid != -1) {
            if (waitpid(u->pid, &cmd_res, 0) < 0) {
                perror("waitpid()");
            }
        }
    }

    // Regain control of foreground group
    pgid = getpgid(0);
    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, pgid);

    return 0;
}
