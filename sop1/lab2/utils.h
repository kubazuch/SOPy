#ifndef KUZU_UTILS
#define KUZU_UTILS

#define _XOPEN_SOURCE 500

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include <stdint.h>

extern char *optarg;
extern int opterr, optind, optopt;

/// Logging utils
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define PUTENV(string) if (putenv(string) != 0) perror("putenv")

/// Random utils
/*
 * Returns random integer in the range [min, max]
 *
 * Uses rand(), so remember to use srand()!
 */
int randint(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

/*
 * Returns random double between [0, 1]
 */
double randdouble() {
    return (double) rand() / (double) RAND_MAX;
}

/*
 * Returns random double between [min, max]
 */
double randrange(double min, double max) {
    return min + (double) rand() / ((double) RAND_MAX / (max - min));
}

/// String utils
int startswith(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

int endswithext(char *name, char *extension) {
    const char *ext = strrchr(name, '.');
    return ext && strcmp(extension, ext + 1) == 0;
}

#endif