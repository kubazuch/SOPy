#include "utils.h"

typedef struct argThread {
    pthread_t tid;
    uint seed;
    sigset_t *pMask;
    int *pTable;
    int table_size;
    int *pGuess;
    pthread_mutex_t *mxTable;
    pthread_mutex_t *mxGuess;
} argThread_t;

void usage(char *name) {
    fprintf(stderr, "USAGE: %s n k\n", name);
    exit(EXIT_FAILURE);
}

void *routine(void *_args);
void handle_signals(sigset_t *pMask, int k, int n, pthread_mutex_t *mxTable,  int *pTable, pthread_t *tidList);
void create_threads(argThread_t *args_array, int *table, pthread_mutex_t *mxTable, int *guess, pthread_mutex_t *mxGuess,
                    sigset_t *pSet, int thread_no, int table_size, pthread_t *tidList);

int main(int argc, char **argv) {
    int n, k;
    if (argc < 2)
        usage(argv[0]);
    n = atoi(argv[1]);
    k = atoi(argv[2]);

    printf("PID: %d\n", getpid());


    int *pTable = malloc(k * sizeof(int));
    if (NULL == pTable)
        ERR("malloc");
    memset(pTable, 0, k * sizeof(int));
    int pGuess = 0;

    sigset_t pMask;
    sigemptyset(&pMask);
    sigaddset(&pMask, SIGINT);
    sigaddset(&pMask, SIGQUIT);

    pthread_mutex_t mxTable = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxGuess = PTHREAD_MUTEX_INITIALIZER;

    pthread_t *tidTable = malloc(n * sizeof(pthread_t));
    argThread_t *args_array = malloc(n * sizeof(argThread_t));
    create_threads(args_array, pTable, &mxTable, &pGuess, &mxGuess, &pMask, n, k, tidTable);

    if (pthread_sigmask(SIG_BLOCK, &pMask, NULL))
        ERR("SIG_BLOCK error");

    handle_signals(&pMask, k,n, &mxTable, pTable, tidTable);

    for (int i = 0; i < n; i++)
        if (pthread_join(tidTable[i], NULL))
            ERR("pthread_join");

    free(pTable);
    free(args_array);
    free(tidTable);
    return EXIT_SUCCESS;
}

void handle_signals(sigset_t *pMask, int k, int n, pthread_mutex_t *mxTable, int *pTable, pthread_t *tidList) {
    int last_sig;
    while(1) {
        last_sig = 0;
        while(last_sig != SIGINT && last_sig != SIGQUIT)
            sigwait(pMask, &last_sig);

        if(last_sig == SIGINT) {
            int i = randint(0, k-1);
            int val = randint(1, 255);
            pthread_mutex_lock(mxTable);
            pTable[i] = val;
            pthread_mutex_unlock(mxTable);
            printf("[MAIN] table[%d] set to %d.\n", i, val);
        }

        if(last_sig == SIGQUIT) {
            for (int i = 0; i < n; i++)
                pthread_cancel(tidList[i]);
            break;
        }
    }
}

void create_threads(argThread_t *args_array, int *table, pthread_mutex_t *mxTable, int *guess, pthread_mutex_t *mxGuess,
                    sigset_t *pMask, int thread_no, int table_size, pthread_t *tidList) {
    srand(time(NULL));
    for (int i = 0; i < thread_no; i++) {
        args_array[i].pTable = table;
        args_array[i].table_size = table_size;
        args_array[i].seed = rand();
        args_array[i].pGuess = guess;
        args_array[i].mxTable = mxTable;
        args_array[i].mxGuess = mxGuess;
        args_array[i].pMask = pMask;

        if (pthread_create(&args_array[i].tid, NULL, routine, &args_array[i]))
            ERR("pthread_create");
        tidList[i] = args_array[i].tid;
    }
}

void *routine(void *_args) {
    printf("Thread no. %lu started!\n", (unsigned long) pthread_self());
    argThread_t *args = _args;

    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    if (pthread_sigmask(SIG_BLOCK, args->pMask, NULL))
        ERR("SIG_BLOCK error");
    while(1) {
        int i = randint_r(&args->seed, 0, args->table_size-1);

        pthread_mutex_lock(args->mxGuess);
        pthread_mutex_lock(args->mxTable);
        if(0 == args->pTable[i]) {
            printf("[%lu] table[%d] = 0, doing nothing.\n", (unsigned long) pthread_self(), i);
            pthread_mutex_unlock(args->mxGuess);
            pthread_mutex_unlock(args->mxTable);
        } else if (*(args->pGuess) == args->pTable[i]) {
            printf("[%lu] table[%d] = %d (guess), ending.\n", (unsigned long) pthread_self(), i, *args->pGuess);

            pthread_mutex_unlock(args->mxGuess);
            pthread_mutex_unlock(args->mxTable);

            break;
        } else {
            printf("[%lu] table[%d] = %d (not guess), setting to %d.\n",
                   (unsigned long) pthread_self(), i, args->pTable[i], *args->pGuess);
            *args->pGuess = args->pTable[i];

            pthread_mutex_unlock(args->mxGuess);
            pthread_mutex_unlock(args->mxTable);
        }
        sleepms(100);
    }
    return NULL;
}