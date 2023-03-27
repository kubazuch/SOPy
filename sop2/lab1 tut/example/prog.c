#include "utils.h"

#define MAX_BUFF 200

volatile sig_atomic_t last_signal = 0;

void sig_handler(int sig)
{
    last_signal = sig;
}

void child_work(int *fd)
{
    char buf[MAX_BUFF + 1];
    srand(getpid());
    char c;
    int status;
    while (SIGINT != last_signal) {
        status = read(fd[0], &c, 1);
        if (status < 0 && errno == EINTR)
            continue;
        if (status < 0)
            ERR("read header from R");
        if (0 == status)
            break;
        if (TEMP_FAILURE_RETRY(read(fd[0], buf, c)) < c)
            ERR("read data from R");
        buf[(int) c] = 0;
        printf("[%d] Got: %s\n", getpid(), buf);

        int k = strtol(buf, NULL, 10);
        if (k == 0)
            break;

        int r = rand() % 21 - 10;
        int count = snprintf(buf + 1, MAX_BUFF, "%d", k + r);
        buf[0] = count;
        if (write(fd[1], buf, count + 1) < 0)
            ERR("write to R");
    }
}

void parent_work(int *fd)
{
    char buf[2];
    buf[0] = 1;
    buf[1] = '1';
    if (write(fd[1], buf, 2) < 0)
        ERR("write to R");
    printf("[%d] Sent: %s\n", getpid(), buf+1);
    child_work(fd);
}

void create_children_and_pipes(int n, int *fds, int *R)
{
    int tmpfd[2];

    for(int i = 0; i < n; ++i) {
        if (pipe(fds + 2*i))
            ERR("pipe");
        printf("[%d] created %d\n", getpid(), fds[2*i]);
        printf("[%d] created %d\n", getpid(), fds[2*i + 1]);
    }

    for (int i = 0; i < n - 1; ++i) {
        switch (fork()) {
            case 0:
                tmpfd[0] = fds[2*(i+1)];
                tmpfd[1] = fds[2*i+1];
                for(int j = 0; j < 2*n; ++j)
                {
                    if(j == 2*i+1 || j == 2*(i+1)) continue;
                    printf("[%d] closing %d\n", getpid(), fds[j]);
                    if (TEMP_FAILURE_RETRY(close(fds[j])))
                        ERR("close");
                }
                free(fds);
                child_work(tmpfd);
                printf("[%d] closing %d\n", getpid(), tmpfd[0]);
                if (TEMP_FAILURE_RETRY(close(tmpfd[0])))
                    ERR("close");
                printf("[%d] closing %d\n", getpid(), tmpfd[1]);
                if (TEMP_FAILURE_RETRY(close(tmpfd[1])))
                    ERR("close");
                exit(EXIT_SUCCESS);

            case -1:
                ERR("Fork:");
        }
    }
    for(int j = 1; j < 2*n-1; ++j){
        printf("[%d] closing %d\n", getpid(), R[0]);
        if (TEMP_FAILURE_RETRY(close(fds[j])))
            ERR("close");
    }
    R[0] = fds[0];
    R[1] = fds[2*n - 1];
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n\n", name);
    fprintf(stderr, "0<n<=10 - number of children\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int n, *fds, R[2];
    n = 3;
    if (NULL == (fds = (int *)malloc(sizeof(int) * 2 * n)))
        ERR("malloc");
    if (set_handler(sigchld_handler, SIGCHLD))
        ERR("Seting parent SIGCHLD:");
    if (set_handler(sig_handler, SIGINT))
        ERR("Setting SIGINT");
    create_children_and_pipes(n, fds, R);
    parent_work(R);
    printf("[%d] closing %d\n", getpid(), R[0]);
    printf("[%d] closing %d\n", getpid(), R[1]);
    if (TEMP_FAILURE_RETRY(close(fds[0])))
        ERR("close");
    if (TEMP_FAILURE_RETRY(close(fds[2*n-1])))
        ERR("close");
    free(fds);
    return EXIT_SUCCESS;
}