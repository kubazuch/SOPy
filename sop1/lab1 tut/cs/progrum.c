#include "utils.h"

#define MAX_PATH 100

void usage(char *pname) {
    fprintf(stderr, "Usage: %s [OPTION]...\n"
                    "Listuje pliki i ich rozmiary w podanych katalogach.\n"
                    "  -p KATALOG\t\tkatalog do wylistowania\n"
                    "  -r\t\t\tlistowanie rekurencyjne\n"
                    "  -s\t\t\tpodążanie za symlinkami\n"
                    "  -n PREFIX\t\tpomijaj pliki i katalogi zaczynajace się na dany prefiks\n"
                    "  -o PLIK\t\tzapisuje wyjscie programu w PLIK\n"
                    , pname);
    exit(EXIT_FAILURE);
}

void list_dir(FILE *out, char *dirName, char* prefix, int recursive, int symlinks) {
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;

    if (NULL == (dirp = opendir(dirName)))
    {
        if(errno == ENOENT)
        {
            fprintf(stderr, "No such file or directory: %s\n", dirName);
            return;
        }

        ERR("opendir");
    }

    fprintf(out, "path:\t%s\n", dirName);
    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            char path[MAX_PATH];

            if (dp->d_name[0] == '.' || (prefix && startswith(prefix, dp->d_name)))
                continue;

            if(snprintf(path, MAX_PATH, "%s/%s", dirName, dp->d_name) >= MAX_PATH)
            {
                fprintf(stderr, "Path too long! %s", path);
                return;
            }

            if (symlinks ? stat(path, &filestat) : lstat(path, &filestat)) {
                fprintf(stderr, "%s\n", path);
                ERR("lstat");
            }

            if (recursive && S_ISDIR(filestat.st_mode))
            {
                list_dir(out, path, prefix, recursive, symlinks);
            }
            else
                fprintf(out,"%s %ld\n", dp->d_name, filestat.st_size);
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");
    if (closedir(dirp))
        ERR("closedir");
}

int main(int argc, char **argv) {
    char c;
    FILE *out = stdout;
    char *prefix = NULL;
    char *output = NULL;
    int recursive = 0;
    int symlinks = 0;

    while ((c = getopt(argc, argv, "p:o:rsn:")) != -1) {
        switch (c) {
            case 'o':
                output = optarg;
                break;
            case 'p':
                break;
            case 'r':
                recursive = 1;
                break;
            case 's':
                symlinks = 1;
                break;
            case 'n':
                prefix = optarg;
                break;
            case '?':
            default:
                usage(argv[0]);
        }
    }

    if (argc > optind)
        usage(argv[0]);

    if (output && !(out = fopen(output, "w")))
        ERR("fopen");

    optind = 0;
    while ((c = getopt(argc, argv, "p:o:rsn:")) != -1) {
        switch (c) {
            case 'p':
                list_dir(out, optarg, prefix, recursive, symlinks);
                break;
            case 'o':
            case 'r':
            case 's':
            case 'n':
                break;
            case '?':
            default:
                usage(argv[0]);
        }
    }

    if(argc > optind) {
        usage(argv[0]);
    }

    if (out != stdout && fclose(out))
        ERR("fclose");

    return EXIT_SUCCESS;
}