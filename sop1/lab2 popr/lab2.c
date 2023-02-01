#include "utils.h"

volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t signals = 0;
volatile sig_atomic_t sig2 = 0;

#define MAX_C 256

void sig_handler(int sig) {
    last_signal = sig;
    if(sig == SIGUSR1)
        signals++;
    if(sig == SIGUSR2)
        sig2 = 1;
}

void writefile(char* buf) {
    int out;
    char name[MAX_C];
    snprintf(name, MAX_C, "%d.txt", getpid());

    if ((out = TEMP_FAILURE_RETRY(open(name, O_WRONLY | O_CREAT | O_APPEND, 0777))) < 0)
        ERR("open");

    if (bulk_write(out, buf, strlen(buf)) < 0)
        ERR("bulk_write");

    if (TEMP_FAILURE_RETRY(close(out)))
        ERR("close");
}

void child_work(pid_t last_pid, sigset_t oldmask) {
    srand(getpid());

    printf("[%d] child created, pid received: %d\n", getpid(), last_pid);
    char buf[MAX_C];
    while(1) {
        last_signal = 0;
        sig2 = 0;
        while (last_signal != SIGUSR1 && last_signal != SIGUSR2 && last_signal != SIGTERM)
            sigsuspend(&oldmask);

        if(sig2) {
            snprintf(buf, MAX_C,"%d\n", signals);
            writefile(buf);
        }

        if(last_signal == SIGTERM) {
            printf("[%d] received SIGUSR1s: %d\n", getpid(), signals);
            kill(last_pid, SIGTERM);
            exit(EXIT_SUCCESS);
        }

        if(last_signal == SIGUSR1) {
            int k = randint(1, 10);
            if(k > 3)
                kill(last_pid, SIGUSR1);
        }
    }
}

pid_t create_child(pid_t last_pid, sigset_t oldmask) {
    pid_t s;
    switch (s = fork()) {
        case 0:
            set_handler(sig_handler, SIGUSR2);
            signal(SIGINT, SIG_IGN);
            child_work(last_pid, oldmask);
            exit(EXIT_SUCCESS);
        case -1:
            ERR("fork");
        default:
            return s;
    }
}

void usage(char *name) {
    fprintf(stderr, "USAGE: %s N\n", name);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int N;
    if (argc < 2)
        usage(argv[0]);
    N = atoi(argv[1]);
    if (N <= 0)
        usage(argv[0]);
    printf("[%d] Parent process\n", getpid());

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    set_handler(sig_handler, SIGTERM);
    set_handler(sig_handler, SIGUSR1);

    pid_t last_pid = getpid();
    for(int i = 0; i < N; ++i)
        last_pid = create_child(last_pid, oldmask);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    signal(SIGUSR2, SIG_IGN);
    set_handler(sig_handler, SIGINT);

    // parent work
    while(1) {
        last_signal = 0;
        sleepms(100);
        if(signals >= 10 || last_signal == SIGTERM)
            break;
        if(last_signal == SIGINT) {
            kill(last_pid, SIGUSR2);
        }
        kill(last_pid, SIGUSR1);
    }

    kill(last_pid, SIGTERM);
    while (TEMP_FAILURE_RETRY(wait(NULL)) > 0);
    printf("[%d] Parent process received SIGUSR1s: %d\n", getpid(), signals);

    return EXIT_SUCCESS;
}