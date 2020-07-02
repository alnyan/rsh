#include <sys/fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "builtin.h"
#include "config.h"
#include "parse.h"
#include "cmd.h"

#define DEF_BUILTIN(b_name) \
    static int __bcmd_##b_name(const struct cmd_unit *cmd)
#define DECL_BUILTIN(b_name) \
    { .name = #b_name, .exec = __bcmd_##b_name }

extern char **environ;

struct sh_builtin {
    const char *name;
    int (*exec) (const struct cmd_unit *cmd);
};

static struct sh_builtin __builtins[];

////

DEF_BUILTIN(env) {
    for (size_t i = 0; environ[i]; ++i) {
        printf("%s\n", environ[i]);
    }

    return 0;
}

DEF_BUILTIN(exit) {
    if (cmd->argc > 1) {
        exit(atoi(cmd->args[1]));
    }
    exit(0);
}

DEF_BUILTIN(cd) {
    if (cmd->argc != 2) {
        printf("usage: cd <path>\n");
        return -1;
    }
    return chdir(cmd->args[1]);
}

DEF_BUILTIN(chmod) {
    mode_t mode = 0;
    const char *p;
    if (cmd->argc != 3) {
        printf("usage: chmod <mode> <filename>\n");
        return -1;
    }
    p = cmd->args[1];
    while (*p) {
        if (*p > '7' || *p < '0') {
            printf("Invalid mode: %s\n", cmd->args[1]);
            return -1;
        }
        mode <<= 3;
        mode |= *p - '0';
        ++p;
    }

    if (chmod(cmd->args[2], mode)) {
        perror(cmd->args[2]);
        return -1;
    }

    return 0;
}

DEF_BUILTIN(stat) {
    struct stat st;
    if (cmd->argc != 2) {
        printf("usage: stat <filename>\n");
        return -1;
    }

    if (stat(cmd->args[1], &st) != 0) {
        return -1;
    }

    printf("device  %u\n",  (unsigned int) st.st_dev);      // TODO: __dev_t
    printf("inode   %u\n",  (unsigned int) st.st_ino);      // TODO: __ino_t
    printf("mode    %u\n",  st.st_mode);
    printf("nlink   %u\n",  (unsigned int) st.st_nlink);    // TODO: __nlink_t
    printf("uid     %u\n",  st.st_uid);
    printf("gid     %u\n",  st.st_gid);
    printf("rdev    %u\n",  (unsigned int) st.st_rdev);     // TODO: __dev_t
    printf("size    %u\n",  (unsigned int) st.st_size);     // TODO: off_t here
    printf("atime   %u\n",  (unsigned int) st.st_atime);
    printf("mtime   %u\n",  (unsigned int) st.st_mtime);
    printf("ctime   %u\n",  (unsigned int) st.st_ctime);
    printf("blksize %u\n",  (unsigned int) st.st_blksize);  // TODO: __blksize_t
    printf("blocks  %u\n",  (unsigned int) st.st_blocks);   // TODO: __blkcnt_t

    return 0;
}

DEF_BUILTIN(sync) {
    sync();
    return 0;
}

DEF_BUILTIN(touch) {
    int fd;
    if (cmd->argc != 2) {
        printf("usage: touch <filename>\n");
        return -1;
    }

    if ((fd = open(cmd->args[1], O_WRONLY | O_CREAT, 0755)) < 0) {
        perror(cmd->args[1]);
        return -1;
    }

    close(fd);

    return 0;
}

DEF_BUILTIN(chown) {
    uid_t uid;
    gid_t gid;
    const char *p;

    if (cmd->argc != 3) {
        printf("usage: chown <uid>:<gid> <filename>\n");
        return -1;
    }

    if (!(p = strchr(cmd->args[1], ':'))) {
        printf("Invalid UID:GID pair: %s\n", cmd->args[1]);
        return -1;
    }
    uid = atoi(cmd->args[1]);
    gid = atoi(p + 1);

    if (chown(cmd->args[2], uid, gid)) {
        perror(cmd->args[2]);
        return -1;
    }

    return 0;
}

DEF_BUILTIN(exec) {
    //struct cmd_exec new_cmd;
    if (cmd->argc < 2) {
        printf("usage: exec <command> ...\n");
    }

    char path_path[256];
    int res;

    if (cmd->args[1][0] == '.' || cmd->args[1][0] == '/') {
        if ((res = access(cmd->args[1], X_OK)) != 0) {
            perror(cmd->args[1]);
            return res;
        }

        strcpy(path_path, cmd->args[1]);
        exit(execve(path_path, (char *const *) &cmd->args[1], NULL));
    }

    snprintf(path_path, sizeof(path_path), "%s/%s", PATH, cmd->args[1]);
    if ((res = access(path_path, X_OK)) == 0) {
        exit(execve(path_path, (char *const *) &cmd->args[1], NULL));
    }

    return -1;
}

DEF_BUILTIN(clear) {
    printf("\033[2J\033[1;1f");
    return 0;
}

DEF_BUILTIN(echo) {
    for (int i = 1; i < cmd->argc; ++i) {
        printf("%s ", cmd->args[i]);
    }
    printf("\n");
    return 0;
}

DEF_BUILTIN(set) {
    if (cmd->argc != 3) {
        fprintf(stderr, "Usage: set <key> <value>\n");
        return -1;
    }

    setenv(cmd->args[1], cmd->args[2], 1);

    return 0;
}

// TODO: support usernames (getpwnam_r)
DEF_BUILTIN(setid) {
    if (cmd->argc == 2) {
        // Assume gid == uid
        if (setuid(atoi(cmd->args[1])) != 0) {
            return -1;
        }
        if (setgid(atoi(cmd->args[1])) != 0) {
            return -1;
        }
        return 0;
    }
    if (cmd->argc != 3) {
        printf("usage: setid <uid> <gid>\n");
        return -1;
    }

    if (setuid(atoi(cmd->args[1])) != 0) {
        return -1;
    }
    if (setgid(atoi(cmd->args[2])) != 0) {
        return -1;
    }
    return 0;
}

DEF_BUILTIN(builtins) {
    for (size_t i = 0; __builtins[i].name; ++i) {
        printf("%s ", __builtins[i].name);
    }
    printf("\n");
    return 0;
}

////

static struct sh_builtin __builtins[] = {
    DECL_BUILTIN(builtins),
    DECL_BUILTIN(cd),
    DECL_BUILTIN(chmod),
    DECL_BUILTIN(chown),
    DECL_BUILTIN(clear),
    DECL_BUILTIN(echo),
    DECL_BUILTIN(env),
    DECL_BUILTIN(exec),
    DECL_BUILTIN(exit),
    DECL_BUILTIN(set),
    DECL_BUILTIN(setid),
    DECL_BUILTIN(stat),
    DECL_BUILTIN(sync),
    DECL_BUILTIN(touch),
    {NULL}
};

builtin_func_t builtin_find(const char *name) {
    for (size_t i = 0; i < sizeof(__builtins) / sizeof(__builtins[0]); ++i) {
        if (!__builtins[i].name) {
            return NULL;
        }
        if (!strcmp(__builtins[i].name, name)) {
            return __builtins[i].exec;
        }
    }
    return NULL;
}
