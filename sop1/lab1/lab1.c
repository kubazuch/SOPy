#include "utils.h"

#define MAX_PATH 100
#define MAX_SUBDIRS 20

void flagrep(char *name) {
    fprintf(stderr, "%s: INVALID FLAG REPETITION\n", name);
    exit(1);
}

void flagmal(char *name) {
    fprintf(stderr, "%s: Malformed flags!\n", name);
    ERR("getopt");
}

void list_dir(char *dirName, int recursive, FILE* output) {
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;
    char path[MAX_PATH];
    char *subdirs[MAX_SUBDIRS];
    int subdir_cnt = 0;

    if (NULL == (dirp = opendir(dirName))) {
        if (errno == ENOENT) {
            fprintf(stderr, "No such directory: %s\n", dirName);
            return;
        } else if (errno == ENOTDIR) {
            fprintf(stderr, "Given file is not a directory: %s\n", dirName);
            return;
        }

        ERR("opendir");
    }

    fprintf(output, "%s:\n", dirName);
    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {

            if (dp->d_name[0] == '.')
                continue;

            if (snprintf(path, MAX_PATH, "%s/%s", dirName, dp->d_name) >= MAX_PATH) {
                fprintf(stderr, "Path too long: %s\n", path);
                continue;
            }

            if (lstat(path, &filestat))
                ERR("lstat");

            if (recursive && S_ISDIR(filestat.st_mode) && subdir_cnt < MAX_SUBDIRS)
                subdirs[subdir_cnt++] = strdup(path);

            char *type = S_ISREG(filestat.st_mode) ? "file" :
                        (S_ISDIR(filestat.st_mode) ? "directory" :
                        (S_ISLNK(filestat.st_mode) ? "link" : "other"));

            fprintf(output, "%s - type: %s, size: %ld\n", dp->d_name, type, filestat.st_size);
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");
    if (closedir(dirp))
        ERR("closedir");

    fprintf(output, "\n");

    for (int i = 0; i < subdir_cnt; i++) {
        list_dir(subdirs[i], recursive, output);
        free(subdirs[i]);
    }
}

int main(int argc, char **argv) {
    char c;
    int anyp = 0;
    int recursive = 0;
    FILE *output = stdout;

    while ((c = getopt(argc, argv, "p:ro:")) != -1) {
        switch (c) {
            case 'o':
                if (output != stdout)
                    flagrep(argv[0]);
                if (!(output = fopen(optarg, "w"))) {
                    fprintf(stderr, "%s: %s: No such file or directory.\n", argv[0], optarg);
                    return 1;
                }
                break;
            case 'p':
                anyp = 1;
                break;
            case 'r':
                if (recursive)
                    flagrep(argv[0]);
                recursive = 1;
                break;
            case '?':
            default:
                flagmal(argv[0]);
        }
    }

    if (!anyp) {
        list_dir(".", recursive, output);
        return EXIT_SUCCESS;
    }

    optind = 0;
    while ((c = getopt(argc, argv, "p:ro:")) != -1) {
        switch (c) {
            case 'p':
                list_dir(optarg, recursive, output);
                break;
            case 'o':
            case 'r':
                break;
            case '?':
            default:
                flagmal(argv[0]);
        }
    }

    return EXIT_SUCCESS;
}