/*
 * Copyright (C) 2016  KeyboardFire <andy@keyboardfire.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 1

#define ADDITION 0x1
#define MULTIPLICATION 0x2
#define TO_BINARY 0x4
#define FROM_BINARY 0x8
#define TO_DECIMAL 0x10
#define FROM_DECIMAL 0x20
#define NUM_OPS 6

#define BUF_SIZE 10;

#define lprintf(logfile, ...) \
    do {\
        fprintf(stdout, __VA_ARGS__);\
        if (logfile) {\
            fprintf(logfile, __VA_ARGS__);\
        }\
    } while (0)

#define FAKEBIN(x) (((((a) >> 3) & 1) << 12) |\
                    ((((a) >> 2) & 1) <<  8) |\
                    ((((a) >> 1) & 1) <<  4) |\
                    ((a) & 1))
#define FAKEDEC(x) ((((a) /    1) % 10)            +\
                    (((a) /   10) % 10) * 16       +\
                    (((a) /  100) % 10) * 16*16    +\
                    (((a) / 1000) % 10) * 16*16*16)

int main(int argc, char **argv) {
    srand(time(NULL));

    int mode = 0, printHelp = 0, printVersion = 0, reportTime = 0, err = 0;
    char *logpath = 0;

    for (int i = 0; i < argc; ++i) {
        if (argv[i][0] == '-') {
            for (char *c = argv[i]+1; *c; ++c) {
                switch (*c) {
                case 'a': mode |= ADDITION; break;
                case 'm': mode |= MULTIPLICATION; break;
                case 'b': mode |= TO_BINARY; break;
                case 'B': mode |= FROM_BINARY; break;
                case 'd': mode |= TO_DECIMAL; break;
                case 'D': mode |= FROM_DECIMAL; break;
                case 'h': printHelp = 1; break;
                case 'v': printVersion = 1; break;
                case 't': reportTime = 1; break;
                case 'l':
                    if (c[1]) {
                        logpath = c + 1;
                        goto argend;
                    } else if (i+1 < argc) {
                        logpath = argv[++i];
                        goto argend;
                    } else {
                        fprintf(stderr, "-l requires an argument\n");
                        err = 1;
                        break;
                    }
                default:
                    fprintf(stderr, "unknown option -%c\n", *c);
                    err = 1;
                    break;
                }
            }
        }
        argend:;
    }

    if (err) return 1;

    if (printVersion) {
        printf("hxqz version %d.%d.%d\n",
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        return 0;
    }

    if (printHelp || !mode) {
        printf(
            "usage: hxqz [-a|-m|-b|-B|-d|-D] [OPTION]...\n"
            "       hxqz -h\n"
            "       hxqz -v\n"
            "\n"
            "-h         output this help message\n"
            "-v         output the version of hxqz\n"
            "-a         test addition\n"
            "-m         test multiplication\n"
            "-b         test conversion to binary\n"
            "-B         test conversion from binary\n"
            "-d         test conversion to decimal\n"
            "-D         test conversion from decimal\n"
            "-t         report time taken to answer\n"
            "-l FILE    log output to FILE\n"
        );
        return 0;
    }


    FILE *logfile = 0;
    if (logpath) {
        if (!(logfile = fopen(logpath, "wx"))) {
            switch (errno) {
            case EACCES:
                fprintf(stderr, "insufficient permissions for log file %s\n",
                        logpath);
                return 1;
            case EEXIST:
                fprintf(stderr, "log file %s exists\n", logpath);
                return 1;
            default:
                fprintf(stderr, "could not open log file %s (errno=%d)\n",
                        logpath, errno);
                return 1;
            }
        }
    }

    while (1) {
        unsigned int a, b;
        int answer;

        int op;
        do { op = 1 << (rand() % NUM_OPS); } while (!(op & mode));
        switch (op) {
            case ADDITION:
                a = rand() % 16;
                b = rand() % 16;
                answer = a+b;
                lprintf(logfile, "%x+%x=? ", a, b);
                break;
            case MULTIPLICATION:
                a = rand() % 16;
                b = rand() % 16;
                answer = a*b;
                lprintf(logfile, "%x*%x=? ", a, b);
                break;
            case TO_BINARY:
                a = rand() % 16;
                answer = FAKEBIN(a);
                lprintf(logfile, "%x hex->bin? ", a);
                break;
            case FROM_BINARY:
                a = rand() % 16;
                answer = a;
                lprintf(logfile, "%04x bin->hex? ", FAKEBIN(a));
                break;
            case TO_DECIMAL:
                a = rand() % (16*16);
                answer = FAKEDEC(a);
                lprintf(logfile, "%x hex->dec? ", a);
                break;
            case FROM_DECIMAL:
                a = rand() % (16*16);
                answer = a;
                lprintf(logfile, "%d dec->hex? ", a);
                break;
        }

        struct timeval start;
        gettimeofday(&start, NULL);

        char *buf;
        size_t n = 0;
        if (getline(&buf, &n, stdin) == -1) { // eof
            lprintf(logfile, "\nbye\n");
            return 0;
        }
        if (logfile) fprintf(logfile, "%s", buf);
        unsigned int reply = strtol(buf, NULL, 16);
        free(buf);

        if (answer == reply) lprintf(logfile, "right");
        else                 lprintf(logfile, "wrong, %x", answer);


        struct timeval end;
        gettimeofday(&end, NULL);

        if (reportTime) {
            lprintf(logfile, " [%f sec]\n",
                ((end.tv_sec   * 1000000.0 + end.tv_usec) -
                 (start.tv_sec * 1000000.0 + start.tv_usec)) / 1000000.0);
        } else {
            lprintf(logfile, "\n");
        }
    }

    return 0;
}
