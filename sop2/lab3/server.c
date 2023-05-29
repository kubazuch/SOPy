#include "utils.h"

#define THREAD_NUM 5
#define BACKLOG 3


typedef struct {
    int socket;
} thread_arg;

int make_socket(int domain, int type)
{
    int sock;
    sock = socket(domain, type, 0);
    if (sock < 0)
        ERR("socket");
    return sock;
}

int bind_tcp_socket(uint16_t port)
{
    struct sockaddr_in addr;
    int socketfd, t = 1;
    socketfd = make_socket(PF_INET, SOCK_STREAM);
    memset(&addr, 0x00, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)))
        ERR("setsockopt");
    if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        ERR("bind");
    if (listen(socketfd, BACKLOG) < 0)
        ERR("listen");
    return socketfd;
}

int add_new_client(int sfd)
{
    int nfd;
    if ((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0) {
        if (EAGAIN == errno || EWOULDBLOCK == errno)
            return -1;
        ERR("accept");
    }
    return nfd;
}

void *threadfunc(void *arg)
{
    thread_arg targ;
    memcpy(&targ, arg, sizeof(targ));
    free(arg);

    if (TEMP_FAILURE_RETRY(send(targ.socket, "HELLO\n", 6, 0)) == -1)
        ERR("send");

    if (TEMP_FAILURE_RETRY(close(targ.socket)) < 0)
        ERR("close");
    return NULL;
}

void dowork(int socket)
{
    int clientfd;
    thread_arg *args;
    pthread_t thread;

    while (1) {
        if ((clientfd = add_new_client(socket)) == -1)
            continue;

        if ((args = (thread_arg*)malloc(sizeof(thread_arg))) == NULL)
            ERR("malloc:");

        args->socket = clientfd;

        if (pthread_create(&thread, NULL, threadfunc, (void *)args) != 0)
            ERR("pthread_create");

        if (pthread_detach(thread) != 0)
            ERR("pthread_detach");
    }
}

int main(int argc, char **argv)
{
    int socket;
    pthread_t thread[THREAD_NUM];

    set_handler(SIG_IGN, SIGPIPE);
    socket = bind_tcp_socket(2000);

    dowork(socket);

    if (TEMP_FAILURE_RETRY(close(socket)) < 0)
        ERR("close");
    return EXIT_SUCCESS;
}