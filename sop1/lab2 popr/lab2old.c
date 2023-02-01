#include "utils.h"

#define MAX_C 256

volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t curr_last = 0;
volatile sig_atomic_t signals = 0;

void sig_handler(int sig) {
    last_signal = sig;
    if(sig == SIGUSR2)
        signals++;
}

void child_sigquit(int sig) {
    printf("[%d] child SIGQUIT\n", getpid());
    exit(EXIT_SUCCESS);
}

void child_work(sigset_t oldmask) {
    printf("[%d] child created\n", getpid());
    while(1) {
        last_signal = 0;
        while (last_signal != SIGUSR2)
            sigsuspend(&oldmask);
        printf("[%d] child SIGUSR2\n", getpid());

        while(last_signal != SIGINT) {
            int time = randint(100,200);
            sleepms(time);
            printf("[%d] child sending signal\n", getpid());
            kill(getppid(), SIGUSR2);
        }

        printf("[%d] child SIGINT\n", getpid());
    }
}

void create_child(sigset_t oldmask, pid_t *pids) {
    pid_t s;
    switch (s = fork()) {
        case 0:
            set_handler(child_sigquit, SIGQUIT);
            child_work(oldmask);
            exit(EXIT_SUCCESS);
        case -1:
            ERR("fork");
        default:
            pids[curr_last++] = s;
    }
}

void usage(char *name) {
    fprintf(stderr, "USAGE: %s 0<n\n", name);
    exit(EXIT_FAILURE);
}

void writefile(char* buf) {
    int out;
    char name[MAX_C];
    snprintf(name, MAX_C, "%d.txt", getpid());

    if ((out = TEMP_FAILURE_RETRY(open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777))) < 0)
        ERR("open");

    if (bulk_write(out, buf, strlen(buf)) < 0)
        ERR("bulk_write");

    if (TEMP_FAILURE_RETRY(close(out)))
        ERR("close");
}

int main(int argc, char **argv) {
    int N;
    if (argc < 2)
        usage(argv[0]);
    N = atoi(argv[1]);
    if (N <= 0)
        usage(argv[0]);
    pid_t pids[N];
    printf("[%d] Parent process\n", getpid());

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    set_handler(sig_handler, SIGUSR1);
    set_handler(sig_handler, SIGUSR2);
    set_handler(sig_handler, SIGINT);
    set_handler(sig_handler, SIGQUIT);

    char buf[MAX_C];
    while(1) {
        last_signal = 0;
        while(last_signal != SIGQUIT && last_signal != SIGUSR1 && last_signal != SIGINT && last_signal != SIGUSR2)
            sigsuspend(&oldmask);
        if (last_signal == SIGQUIT) {
            while(curr_last > 0)
                kill(pids[--curr_last], SIGQUIT);
            break;
        }
        if (last_signal == SIGUSR1) {
            if (curr_last == N)
                fprintf(stderr, "Cannot create a new process!\n");
            else
                create_child(oldmask, pids);
        }
        if (last_signal == SIGUSR2) {
            printf("[%d] %d signals\n", getpid(), signals);
            snprintf(buf, MAX_C,"%d\n", signals);
            writefile(buf);
        }
        if (last_signal == SIGINT) {
            if (curr_last == 0)
                fprintf(stderr, "There is no process to kill!\n");
            else
                kill(pids[--curr_last], SIGQUIT);
        }
    }

    while (TEMP_FAILURE_RETRY(wait(NULL)) > 0);
    printf("Got %d signals in total\n", signals);

    return EXIT_SUCCESS;
}