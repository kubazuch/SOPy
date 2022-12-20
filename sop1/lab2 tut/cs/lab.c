#include "utils.h"

#define MAX_N 256

volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t counter = 0;
int N;

void sig_handler(int sig) {
    last_signal = sig;
    if(last_signal == SIGUSR1)
        counter++;
}

void child_work() {
    srand(getpid());
    int k = randint(0, 10);
    printf("[%d] k = %d\n", getpid(), k);

    for(int s = 1; s <= 10; s++) {
        struct timespec ts = {1, 0};
        safesleep(ts);

        if (s < k)
            kill(0, SIGUSR1);
    }

    printf("[%d] i = %d, k = %d\n", getpid(), counter, k);
    exit(((counter % 15) << 4) | (k % 15));
}

void create_children() {
    pid_t s;
    for (int i = 0; i < N; i++) {
        if ((s = fork()) < 0)
            ERR("Fork:");
        if (!s) {
            child_work();
        }
    }
}

void usage(char *name) {
    fprintf(stderr, "USAGE: %s n\n", name);
    fprintf(stderr, "n - liczba procesów dzieci 0 -- %d\n", MAX_N);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc != 2)
        usage(argv[0]);
    N = atoi(argv[1]);
    if (N <= 0 || N > MAX_N)
        usage(argv[0]);

    printf("Parent PID: %d\n", getpid());

    //set_handler(sigchld_handler, SIGCHLD);
    set_handler(sig_handler, SIGUSR1);

    create_children();

    int pid;
    int status;
    while ((pid = TEMP_FAILURE_RETRY(waitpid(0, &status, 0))) > 0)
    {
        status = WEXITSTATUS(status);
        printf("[%d] Child %d exited with status: %d, meaning k = %d; i = %d\n", getpid(), pid,
               status, status & 15, (status >> 4) & 15);
    }
    return EXIT_SUCCESS;
}

