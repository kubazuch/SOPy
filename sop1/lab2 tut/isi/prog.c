#include "utils.h"

#define MAX_C 256

volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t counter = 0;
int pids[MAX_C];
int N;

void sig_handler(int sig) {
    last_signal = sig;
}

void child_sigint_handler(int sig) {
    last_signal = sig;

    int out;
    ssize_t count = 0;
    char name[MAX_C];
    char buf[MAX_C];

    sprintf(buf, "%d", counter);
    sprintf(name, "d.txt", getpid());

    if ((out = TEMP_FAILURE_RETRY(open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777))) < 0)
        ERR("open");

    if ((count = bulk_write(out, buf, strlen(buf))) < 0)
        ERR("bulk_write");

    if (TEMP_FAILURE_RETRY(close(out)))
        ERR("close");

    exit(EXIT_SUCCESS);
}

void child_work(sigset_t *oldmask) {
    set_handler(child_sigint_handler, SIGINT);
    printf("[%d] Child created!\n", getpid());
    srand(getpid());
    while (1) {
        last_signal = 0;
        while(last_signal != SIGUSR1)
            sigsuspend(oldmask);
        while(last_signal != SIGUSR2) {
            int time = randint(100, 200);
            struct timespec t = {0, time * 1e6};
            nanosleep(&t, NULL);
            printf("[%d] %d\n", getpid(), ++counter);
        }
    }
}

void parent_sigint_handler(int sig) {
    last_signal = sig;
    for(int i = 0; i < N; i++)
        kill(pids[i], SIGINT);
}

void parent_work(int N, sigset_t *oldmask) {
    set_handler(parent_sigint_handler, SIGINT);
    kill(pids[0], SIGUSR1);

    int i = 0;
    while(last_signal != SIGINT) {
        last_signal = 0;
        while(last_signal != SIGUSR1 && last_signal != SIGINT)
            sigsuspend(oldmask);
        kill(pids[i % N], SIGUSR2);
        if(last_signal == SIGINT) break;
        kill(pids[++i % N], SIGUSR1);
    }
}

void create_children(int n, sigset_t *oldmask) {
    pid_t s;
    for (int i = 0; i < n; i++) {
        if ((s = fork()) < 0)
            ERR("Fork:");
        if (!s) {
            child_work(oldmask);
            exit(EXIT_SUCCESS);
        } else {
            pids[i] = s;
        }
    }
}

void usage(char *name) {
    fprintf(stderr, "USAGE: %s M\n", name);
    fprintf(stderr, "N - liczba procesów dzieci 0 -- %d\n", MAX_C);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc != 2)
        usage(argv[0]);
    N = atoi(argv[1]);
    if (N <= 0 || N > MAX_C)
        usage(argv[0]);

    printf("Parent PID: %d\n", getpid());

    set_handler(sigchld_handler, SIGCHLD);
    set_handler(sig_handler, SIGUSR1);
    set_handler(sig_handler, SIGUSR2);

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    create_children(N, &oldmask);
    parent_work(N, &oldmask);

    while (wait(NULL) > 0);
    return EXIT_SUCCESS;
}