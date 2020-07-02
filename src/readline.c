//#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>

//#define KEY_UP      (256)
//#define KEY_DOWN    (257)
//#define KEY_RIGHT   (258)
//#define KEY_LEFT    (259)
//
//// With support for escape sequences
//static int getch_del(void) {
//    struct timeval tv = {
//        .tv_sec = 0,
//        .tv_usec = 500000
//    };
//    fd_set fds;
//    FD_ZERO(&fds);
//    FD_SET(STDIN_FILENO, &fds);
//    int res;
//
//    if ((res = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv)) < 0) {
//        return res;
//    }
//
//    if (FD_ISSET(STDIN_FILENO, &fds)) {
//        res = 0;
//        if (read(STDIN_FILENO, &res, 1) != 1) {
//            return -1;
//        }
//        return res;
//    } else {
//        return -1;
//    }
//}
//
//static int escape_read(void) {
//    // Maybe [
//    int c = getch_del();
//
//    if (c < 0) {
//        return '\033';
//    }
//
//    if (c != '[') {
//        return -1;
//    }
//
//    c = getch_del();
//
//    switch (c) {
//    case 'A':
//        return KEY_UP;
//    case 'B':
//        return KEY_DOWN;
//    case 'C':
//        return KEY_RIGHT;
//    case 'D':
//        return KEY_LEFT;
//    default:
//        return -1;
//    }
//}
//
//static int getch(void) {
//    char c;
//
//    if (read(STDIN_FILENO, &c, 1) != 1) {
//        return -1;
//    }
//
//    if (c == '\033') {
//        int r = escape_read();
//
//        if (r > 0) {
//            return r;
//        }
//    }
//
//    return c;
//}

int readline(char *buf, size_t lim) {
    // TODO: rewrite this for new kernel line discipline to disable ECHO/ICANON
    ssize_t len = read(STDIN_FILENO, buf, lim);

    // Just strip newline
    if (len <= 0) {
        return -1;
    }

    if (buf[len - 1] == '\n') {
        buf[len - 1] = 0;
    }

    //printf("Got len = %ld\n", len);
    //while (1);

    //int len = 0;
    //int cur = 0;
    //int chr;

    //while (1) {
    //    if ((chr = getch()) <= 0) {
    //        return -1;
    //    }

    //    if (len == lim) {
    //        printf("Input line is too long\n");
    //        return -1;
    //    }

    //    switch (chr) {
    //    case KEY_LEFT:
    //        if (cur) {
    //            puts2("\033[D");
    //            --cur;
    //        }
    //        break;
    //    case KEY_RIGHT:
    //        if (cur < len) {
    //            puts2("\033[C");
    //            ++cur;
    //        }
    //        break;
    //    }

    //    if (chr == '\n') {
    //        putchar(chr);
    //        buf[len] = 0;
    //        break;
    //    } else if (chr == '\b') {
    //        if (cur) {
    //            --cur;
    //            for (int i = cur; i < len - 1; ++i) {
    //                buf[i] = buf[i + 1];
    //            }
    //            puts2("\033[D\033[s");
    //            --len;
    //            for (int i = cur; i < len; ++i) {
    //                putchar(buf[i]);
    //            }
    //            puts2(" \033[u");
    //        }
    //    } else if (chr >= ' ' && chr < 255) {
    //        putchar(chr);
    //        puts2("\033[s");
    //        for (int i = cur; i < len; ++i) {
    //            putchar(buf[i]);
    //        }
    //        puts2("\033[u");
    //        for (int i = len; i > cur; --i) {
    //            buf[i] = buf[i - 1];
    //        }
    //        buf[cur++] = chr;
    //        ++len;
    //    }
    //}

    return len;
}

