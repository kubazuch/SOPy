#include "utils.h"

typedef struct argThread
{
    pthread_t tid;
} argThread_t;

void usage(char *name) {
    fprintf(stderr, "USAGE: %s n k\n", name);
    exit(EXIT_FAILURE);
}

void *routine(void *_args)
{
    printf("Thread no. %lu started!\n", (unsigned long)pthread_self());
    return NULL;
}

int main(int argc, char **argv)
{
    int n, k;
    if (argc < 2)
        usage(argv[0]);
    n = atoi(argv[1]);
    k = atoi(argv[1]);

    argThread_t *args_array = malloc(n * sizeof(argThread_t));
    for (int i = 0; i < n; i++)
    {
        if (pthread_create(&args_array[i].tid, NULL, routine, &args_array[i]))
            ERR("pthread_create");
    }

    for (int i = 0; i < n; i++)
        if (pthread_join(args_array[i].tid, NULL))
            ERR("pthread_join");

    free(args_array);
    return EXIT_SUCCESS;
}