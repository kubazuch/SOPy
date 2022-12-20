#include "utils.h"

#define BUFSIZE 1024

volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t alarm_received = 0;
volatile sig_atomic_t USR1_received = 0;
volatile sig_atomic_t sigCount = 0;
volatile sig_atomic_t block_count = 0;

void sig_handler(int sig) {
    if (SIGUSR1 == sig)
    {
        sigCount++;
        USR1_received = 1;
    }
    if (SIGALRM == sig)
        alarm_received = 1;
    last_signal = sig;
//    printf("[%d] Signal: %d\n", getpid(), sig);
}

void child_work(int n, sigset_t oldmask) {
    srand(getpid());
    int s = (rand() % 91 + 10) * 1024;

    // tworzenie pliku
    int out;
    char name[BUFSIZE];
    snprintf(name, BUFSIZE, "%d.txt", getpid());
    if ((out = TEMP_FAILURE_RETRY(open(name,O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,0777))) < 0)
        ERR("open");

    // przygotowanie bloku
    char *block = malloc(sizeof(char) * s);
    for (int i = 0; i < s; i++) {
        block[i] = '0' + n;
    }
    /* sigsuspend nas interesuje tylko dla SIGUSR1, wiec nie ma sensu czekac
     * na alarm, tylko sprawdzamy czy juz byl, czyli nie ustawiamy dla niego
     * maski */
    alarm(1);
    while (1) {
        USR1_received = 0;
        sigsuspend(&oldmask);
        if (USR1_received) {
            // wypisywanie do pliku
            for (int i = block_count; i < sigCount; i++) {
                ssize_t count;
                if ((count = bulk_write(out, block, s)) < 0)
                    ERR("write");
                block_count++;
            }
        }
        if (alarm_received)
            break;
    }

    // zamkniecie pliku
    free(block);
    if(TEMP_FAILURE_RETRY(close(out)))
        ERR("close");
}

void parent_work()
{
    struct timespec tk = { 0, 1e8};
    int sigUSR1Sent = 0;
    alarm(1);
    while (!alarm_received) {
        if (kill(0, SIGUSR1) < 0)
            ERR("kill");
        sigUSR1Sent++;
        nanosleep(&tk, NULL);
    }

    printf("[PARENT] Sent %d SIGUSR1.\n", sigUSR1Sent);
}

void create_children(int n, int numbers[]) {
    for (int i = 0; i < n; i++) {
        switch (fork()) {
            case 0:
                set_handler(sig_handler, SIGUSR1);
                sigset_t mask, oldmask;
                sigemptyset(&mask);
                sigaddset(&mask, SIGUSR1);
                sigprocmask(SIG_BLOCK, &mask, &oldmask);

                child_work(numbers[i], oldmask);
                sigprocmask(SIG_UNBLOCK, &mask, NULL);
                printf("[%d] Child dies with %d USR1 received and %d blocks written\n", getpid(), sigCount, block_count);
                exit(EXIT_SUCCESS);
            case -1:
                ERR("fork");
        }
    }
}

void usage(char* pname) {
    fprintf(stderr, "Usage: %s [args]...\n"
                    "Opis zadania\n",
            pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    if (argc == 1)
        usage(argv[0]);

    int digits[argc-1];
    for (int i = 1; i < argc; i++) {
        digits[i-1] = atoi(argv[i]);
        if (digits[i-1] > 9 || digits[i-1] < 0)
            usage(argv[0]);
    }

    set_handler(sig_handler, SIGALRM);
    set_handler(SIG_IGN, SIGUSR1);
    create_children(argc-1, digits);
    parent_work();

    while (wait(NULL) > 0);
}