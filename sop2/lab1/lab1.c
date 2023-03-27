#include "utils.h"

#define MSG_SIZE 250

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s N\n", name);
    fprintf(stderr, "0<n<=20 - number of children\n");
    exit(EXIT_FAILURE);
}

void child_work(int n, int *fd)
{
    srand(getpid());
    char old_buf[MSG_SIZE];
    char buf[MSG_SIZE];
    int count;

    for(;;) {
        if ((count = TEMP_FAILURE_RETRY(read(fd[0], old_buf, MSG_SIZE))) < 0)
            ERR("read data from R");

        if(old_buf[MSG_SIZE - 1] == '\0')
            count = strlen(old_buf);

        int i = rand() % count;
        int j = rand() % count;

        memcpy(buf, old_buf, MSG_SIZE);
        buf[i] = old_buf[j];
        buf[j] = old_buf[i];

        printf("[%d] Node %d got message %s and sending %s\n", getpid(), n, old_buf, buf);

        if (write(fd[1], buf, MSG_SIZE) < 0)
            ERR("write to R");
    }
}

void parent_work(int *fd)
{
    char buf[MSG_SIZE + 1];
    int fifo;

    if ((fifo = open("bus.fifo", O_RDONLY)) < 0)
        ERR("open");

    int count;
    for(;;) {
        if ((count = TEMP_FAILURE_RETRY(read(fifo, buf, MSG_SIZE))) < 0)
            ERR("read data from FIFO");
        buf[strcspn(buf, "\n")] = 0;

        if (count < MSG_SIZE)
            memset(buf + count, 0, MSG_SIZE - count);

        printf("[%d] Sending %s\n", getpid(), buf);
        if (write(fd[1], buf, MSG_SIZE) < 0)
            ERR("write to R");

        if (TEMP_FAILURE_RETRY(read(fd[0], buf, MSG_SIZE)) < MSG_SIZE)
            ERR("read data from R");
        printf("[%d] Node %d got message %s\n", getpid(), 0, buf);
    }

    if (close(fifo) < 0)
        ERR("close fifo:");

    if (unlink("bus.fifo") < 0)
        ERR("remove fifo:");
}

void create_children_and_pipes(int n, int *fds, int *R)
{
    int tmpfd[2];

    for(int i = 0; i < n; ++i)
        if (pipe(fds + 2*i))
            ERR("pipe");

    for (int i = 0; i < n - 1; ++i) {
        switch (fork()) {
            case 0:
                tmpfd[0] = fds[2*i];
                tmpfd[1] = fds[2*i+3];
                for(int j = 0; j < 2*n; ++j)
                {
                    if(j == 2*i || j == 2*i+3) continue;
                    if (TEMP_FAILURE_RETRY(close(fds[j])))
                        ERR("close");
                }
                free(fds);
                child_work(i+1, tmpfd);
                if (TEMP_FAILURE_RETRY(close(tmpfd[0])))
                    ERR("close");
                if (TEMP_FAILURE_RETRY(close(tmpfd[1])))
                    ERR("close");
                exit(EXIT_SUCCESS);

            case -1:
                ERR("Fork:");
        }
    }
    for(int j = 0; j < 2*n; ++j)
        if (j != 1 && j!= 2*(n-1) && TEMP_FAILURE_RETRY(close(fds[j])))
            ERR("close");

    R[0] = fds[2*(n-1)];
    R[1] = fds[1];
}

int main(int argc, char **argv)
{
    int n, *fds, R[2];

    if (2 != argc)
        usage(argv[0]);
    n = atoi(argv[1]);
    if (n <= 0 || n > 20)
        usage(argv[0]);

    if (NULL == (fds = (int *)malloc(sizeof(int) * 2 * (n+1))))
        ERR("malloc");
    if (set_handler(sigchld_handler, SIGCHLD))
        ERR("Seting parent SIGCHLD:");

    if (mkfifo("bus.fifo", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0)
        if (errno != EEXIST)
            ERR("create fifo");

    create_children_and_pipes(n + 1, fds, R);
    parent_work(R);

    if (TEMP_FAILURE_RETRY(close(fds[1])))
        ERR("close");
    if (TEMP_FAILURE_RETRY(close(fds[2*n])))
        ERR("close");
    free(fds);
    return EXIT_SUCCESS;
}