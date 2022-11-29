#include "utils.h"

#define MAX_PATH 100
#define MAXFD 20

void usage(char *pname) {
    fprintf(stderr, "Usage: %s [OPTION]...\n"
                    "Listuje pliki i ich rozmiary w podanych katalogach.\n"
                    "  -p KATALOG\t\tkatalog do wylistowania\n"
                    "  -d LICZBA\t\tlmaksymalna głębokość rekurencji\n"
                    "  -e EXT\t\tpomijaj pliki posiadające podane rozszerzenie\n"
                    "  -o \t\t\tzapisuje wyjscie programu w zmiennej środowiskowej L1_OUTPUTFILE jeśli wskazuje ona na plik\n"
            , pname);
    exit(EXIT_FAILURE);
}

int maxdepth;
char *extention;
FILE *out;

int visit(const char *path, const struct stat *s, int type, struct FTW *f)
{
    if (f->level > maxdepth)
        return 0;

    const char *name = path + f->base;

    if(name[0] == '.')
        return 0;

    const char *ext = strrchr(name, '.');
    if(extention && ext && strcmp(extention, ext+1) == 0)
        return 0;

    switch (type)
    {
        case FTW_D:
            if (f->level != maxdepth)
                fprintf(out, "path:\t%s\n", path);
            break;
        case FTW_F:
            fprintf(out,"%s %ld\n", name, s->st_size);
            break;
    }

    return 0;
}

int main(int argc, char **argv) {
    char c;
    out = stdout;
    extention = NULL;
    char *output = NULL;
    maxdepth = 0;

    while ((c = getopt(argc, argv, "p:d:e:o")) != -1) {
        switch (c) {
            case 'o':
                output = getenv("L1_OUTPUTFILE");
                break;
            case 'p':
                break;
            case 'd':
                maxdepth = strtol(optarg, NULL, 10);
                break;
            case 'e':
                extention = optarg;
                break;
            case '?':
            default:
                usage(argv[0]);
        }
    }

    if (argc > optind || maxdepth == 0)
        usage(argv[0]);

    if (output && !(out = fopen(output, "w")))
        ERR("fopen");

    optind = 0;
    while ((c = getopt(argc, argv, "p:d:e:o")) != -1) {
        switch (c) {
            case 'p':
                if(nftw(optarg, visit, MAXFD, FTW_PHYS) != 0)
                    fprintf(stderr, "%s: Permission denied!\n", optarg);
                break;
            case 'o':
            case 'd':
            case 'e':
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