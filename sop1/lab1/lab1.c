#include "utils.h"

#define MAX_PATH 100

void flagrep() {
    fprintf(stderr, "INVALID FLAG REPETITION");
    exit(1);
}

void flagmal() {
    fprintf(stderr, "INVALID FLAG REPETITION");
    exit(1);
}

void list_dir(char *dirName) {
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;
    char path[MAX_PATH];

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

    printf("%s:\n", dirName);
    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (dp->d_name[0] == '.')
                continue;

            if(snprintf(path, MAX_PATH, "%s/%s", dirName, dp->d_name) >= MAX_PATH)
            {
                fprintf(stderr, "Path too long: %s\n", path);
                continue;
            }

            if (lstat(path, &filestat))
                ERR("lstat");

            char *type = S_ISREG(filestat.st_mode) ? "file" :
                        (S_ISDIR(filestat.st_mode) ? "directory" :
                        (S_ISLNK(filestat.st_mode) ? "link" : "other"));

            printf("%s - type: %s, size: %ld\n", dp->d_name, type, filestat.st_size);
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");
    if (closedir(dirp))
        ERR("closedir");

    printf("\n");
}

int main(int argc, char **argv) {
    char c;
    int anyp = 0;
    int recursive = 0;
    char *output = NULL;

    while ((c = getopt(argc, argv, "p:ro:")) != -1) {
        switch (c) {
            case 'o':
                if (output)
                    flagrep();
                output = optarg;
                break;
            case 'p':
                anyp = 1;
                break;
            case 'r':
                if (recursive)
                    flagrep();
                recursive = 1;
                break;
            case '?':
            default:
                flagmal();
        }
    }

    if(!anyp) {
        list_dir(".");
        return EXIT_SUCCESS;
    }

    optind = 0;
    while ((c = getopt(argc, argv, "p:ro:")) != -1) {
        switch (c) {
            case 'p':
                list_dir(optarg);
                break;
            case 'o':
            case 'r':
                break;
            case '?':
            default:
                flagmal();
        }
    }

    return EXIT_SUCCESS;
}