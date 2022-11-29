#include "utils.h"

#define MAX_PATH 100

void usage(char *pname) {
    fprintf(stderr, "Usage: %s ...\n"
                    "    add <AUTHOR> <BOOK>\t\tdodaje nową książkę, jeśli katalog autora nie istnieje, tworzy go\n"
                    "    list <AUTHOR>\t\twypisuje wszystkie książki danego autora wraz z datą dodania \n"
                    "    stats\t\t\twypisuje wszystkich autorów wraz z liczbą przypadających im książek\n"
                    "    save <FILE> \t\tzapisuje statystyki z poprzedniej komendy do podanego pliku\n", pname);
    exit(EXIT_FAILURE);
}

void add(char *author, char *book) {
    struct stat filestat;
    FILE *f;

    if (stat(author, &filestat) == -1) {
        if (mkdir(author, 0777))
            ERR("mkdir");
    }

    if (chdir(author))
        ERR("chdir");
    if ((f = fopen(book, "w")) == NULL)
        ERR("fopen");
    if (fclose(f))
        ERR("fclose");
}

void list(char *author) {
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;

    if (chdir(author) == -1) {
        fprintf(stderr, "No such author: %s\n", author);
        exit(1);
    }

    if (NULL == (dirp = opendir(".")))
        ERR("opendir");

    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (dp->d_name[0] == '.')
                continue;

            if (stat(dp->d_name, &filestat))
                ERR("stat");

            printf("%s, %ld\n", dp->d_name, filestat.st_mtime);
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");
    if (closedir(dirp))
        ERR("closedir");
}

void stats(FILE *stream) {
    DIR *dirp, *author;
    struct dirent *dp, *adp;
    struct stat filestat;
    char cwd[MAX_PATH];
    int books = 0;

    if (getcwd(cwd, sizeof(cwd)) == NULL)
        ERR("getcwd");
    if (NULL == (dirp = opendir("."))) {
        ERR("opendir");
    }

    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (dp->d_name[0] == '.')
                continue;

            if (stat(dp->d_name, &filestat))
                ERR("stat");

            if (S_ISDIR(filestat.st_mode)) {
                books = 0;
                if (chdir(dp->d_name) == -1)
                    ERR("chdir");

                if ((author = opendir(".")) == NULL)
                    ERR("opendir");

                do {
                    errno = 0;
                    if ((adp = readdir(author)) != NULL) {
                        if (adp->d_name[0] == '.')
                            continue;
                        books++;
                    }
                } while (adp != NULL);

                if (errno != 0)
                    ERR("readdir");
                if (closedir(author))
                    ERR("closedir");
                if (chdir(cwd))
                    ERR("chdir");

                fprintf(stream, "%s: %d\n", dp->d_name, books);
            }
        }
    } while (dp != NULL);

    if (errno != 0)
        ERR("readdir");
    if (closedir(dirp))
        ERR("closedir");
}

int main(int argc, char **argv) {
    FILE *f = stdout;
    if (argc < 2)
        usage(argv[0]);

    if (strcmp(argv[1], "add") == 0) {
        char *author, *book;
        if (argc < 4)
            usage(argv[0]);
        author = argv[2];
        book = argv[3];
        add(author, book);
    } else if (strcmp(argv[1], "list") == 0) {
        char *author;
        if (argc < 3)
            usage(argv[0]);
        author = argv[2];
        list(author);
    } else if (strcmp(argv[1], "stats") == 0) {
        stats(f);
    } else if (strcmp(argv[1], "save") == 0) {
        char *file;
        if (argc < 3)
            usage(argv[0]);
        file = argv[2];
        if ((f = fopen(file, "w")) == NULL) {
            fprintf(stderr, "No such file or directory: %s\n", file);
            exit(1);
        }

        stats(f);
    } else {
        fprintf(stderr, "INVALID ARGUMENTS");
        return 1;
    }

    return EXIT_SUCCESS;
}