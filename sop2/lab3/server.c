#include "utils.h"

#define THREAD_NUM 5
#define BACKLOG 3

typedef struct node{
    int16_t data;
    struct node *ptr;
} node;

node* insert(node* head, int16_t num) {
    node *temp, *prev, *next;
    temp = (node*)malloc(sizeof(node));
    temp->data = num;
    temp->ptr = NULL;
    if(!head){
        head=temp;
    } else{
        prev = NULL;
        next = head;
        while(next && next->data<=num){
            prev = next;
            next = next->ptr;
        }
        if(!next){
            prev->ptr = temp;
        } else{
            if(prev) {
                temp->ptr = prev->ptr;
                prev-> ptr = temp;
            } else {
                temp->ptr = head;
                head = temp;
            }
        }
    }
    return head;
}

void free_list(node *head) {
    node *prev = head;
    node *cur = head;
    while(cur) {
        prev = cur;
        cur = prev->ptr;
        free(prev);
    }
}

typedef struct {
    int socket;
    node** head;
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

    int16_t num;
    if(TEMP_FAILURE_RETRY(recv(targ.socket, (char *)&num, sizeof(int16_t), 0)) < 0)
        ERR("recv:");
    num = ntohs(num);

    printf("Got: %d\n", num);
    *(targ.head) = insert(*(targ.head), num);

    int16_t n = 0;
    printf("List: ");
    node *p = *(targ.head);
    while(p) {
        printf(" %d", p->data);
        p = p->ptr;
        n++;
    }
    printf("\n");

    if (TEMP_FAILURE_RETRY(write(targ.socket, (char *)&n, sizeof(int16_t))) < 0)
        ERR("write");

    int16_t *buff = malloc(n * sizeof(int16_t));

    uint16_t i = 0;
    p = *(targ.head);
    while(p) {
        buff[i++] = htons(p->data);
        p = p->ptr;
    }

    if (TEMP_FAILURE_RETRY(write(targ.socket, (char *)buff, i*sizeof(int16_t))) < 0)
        ERR("write");

    if (TEMP_FAILURE_RETRY(close(targ.socket)) < 0)
        ERR("close");
    return NULL;
}

void dowork(int socket, node* head)
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
        args->head = &head;

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

    node *head, *p;
    head = NULL;

    dowork(socket, head);

    free_list(head);
    if (TEMP_FAILURE_RETRY(close(socket)) < 0)
        ERR("close");
    return EXIT_SUCCESS;
}