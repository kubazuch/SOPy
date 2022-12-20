#include "utils.h"

#define MAX_N 256

volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t curr_last = 0;

void sig_handler(int sig) {
    last_signal = sig;
}

void child_work(sigset_t oldmask) {
    printf("[%d] child created\n", getpid());
    while(1) {
        last_signal = 0;
        while (last_signal != SIGQUIT)
            sigsuspend(&oldmask);

        if (last_signal == SIGQUIT){
            printf("[%d] child SIGQUIT\n", getpid());
            exit(EXIT_SUCCESS);
        }
    }
}

void create_child(sigset_t oldmask, pid_t *pids) {
    pid_t s;
    switch (s = fork()) {
        case 0:
            free(pids);
            child_work(oldmask);
            exit(EXIT_SUCCESS);
        case -1:
            ERR("fork");
        default:
            pids[curr_last++] = s;
    }
}

void usage(char *name) {
    fprintf(stderr, "USAGE: %s 0<n<%d\n", name, MAX_N);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int N;
    if (argc < 2)
        usage(argv[0]);
    N = atoi(argv[1]);
    if (N <= 0 || N > MAX_N)
        usage(argv[0]);
    pid_t *pids = malloc(N * sizeof(pid_t));
    printf("[%d] Parent process\n", getpid());

    set_handler(sig_handler, SIGUSR1);
    set_handler(sig_handler, SIGINT);
    set_handler(sig_handler, SIGQUIT);

    sigset_t mask;
    sigprocmask(SIG_BLOCK, NULL, &mask);

    while(1) {
        last_signal = 0;
        while(last_signal != SIGUSR1 && last_signal != SIGINT)
            sigsuspend(&mask);
        if (last_signal == SIGUSR1) {
            if (curr_last == N)
                fprintf(stderr, "Cannot create a new process!\n");
            else
                create_child(mask, pids);
        } else if (last_signal == SIGINT) {
            if (curr_last == 0)
                fprintf(stderr, "There is no process to kill!\n");
            else
                kill(pids[--curr_last], SIGQUIT);
        }
    }

    while (wait(NULL) > 0);
    free(pids);

    return EXIT_SUCCESS;
}