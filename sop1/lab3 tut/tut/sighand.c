/* sighand.c - illustrates use of thread masks.
Send SIGINT in different phases of process operation.
Can you explain process behaviour? */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

struct sigaction act;

void inthandler(int sig)
{
	printf("thread %lu caught signal %d\n", (unsigned long)pthread_self(), sig);
}

void *sigthread(void *p)
{
    sigset_t new, old;
    sigemptyset(&new);
    sigaddset(&new, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &new, &old);
	printf("Press CTRL-C to deliver SIGINT signal to the process\n");
	int i;
    sigwait(&new, &i);
	printf("sigthread is done\n");
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tid;
	act.sa_handler = inthandler;
	sigaction(SIGINT, &act, NULL);
    printf("main thread # %lu is w8tin\n", (unsigned long)pthread_self());
	pthread_create(&tid, NULL, sigthread, NULL);
	pthread_join(tid, NULL);
	printf("main thread # %lu is done\n", (unsigned long)pthread_self());
	return 0;
}
