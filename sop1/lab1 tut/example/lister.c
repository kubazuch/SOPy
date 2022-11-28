#include "utils.h"

#define MAX_PATH 101

void usage(char *pname) {
    fprintf(stderr, "Usage: %s [OPTION]...\n"
                    "Listuje pliki i ich rozmiary w podanych katalogach.\n"
                    "  -o PLIK\t\tzapisuje wyjscie programy w PLIK, max. 1\n"
                    "  -p KATALOG\t\tkatalog do wylistowania\n", pname);
    exit(EXIT_FAILURE);
}

void list_dir(char *dirName, FILE *out) {
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;
    char path[MAX_PATH];

    if (getcwd(path, MAX_PATH) == NULL)
        ERR("getcwd");
    if (chdir(dirName))
    {
        if(errno == ENOENT)
        {
            fprintf(stderr, "No such file or directory: %s\n", dirName);
            return;
        }

        ERR("chdir");
    }
    if (NULL == (dirp = opendir(".")))
        ERR("opendir");

    fprintf(out, "SCIEZKA:\n%s\nLISTA PLIKOW:\n", dirName);
    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (dp->d_name[0] == '.')
                continue;

            if (lstat(dp->d_name, &filestat))
                ERR("lstat");

            fprintf(out,"%s %ld\n", dp->d_name, filestat.st_size);
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");
    if (closedir(dirp))
        ERR("closedir");
    if (chdir(path))
        ERR("chdir");
}

int main(int argc, char **argv) {
    char c;
    FILE *out = stdout;
    while ((c = getopt(argc, argv, "p:o:")) != -1) {
        switch (c) {
            case 'o':
                if (out != stdout)
                    usage(argv[0]);
                if ((out = fopen(optarg, "w")) == NULL)
                    ERR("fopen");
                break;
            case 'p':
                break;
            case '?':
            default:
                usage(argv[0]);
        }
    }

    if (argc > optind)
        usage(argv[0]);

    optind = 0;
    while ((c = getopt(argc, argv, "p:o:")) != -1) {
        switch (c) {
            case 'o':
                break;
            case 'p':
                list_dir(optarg, out);
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