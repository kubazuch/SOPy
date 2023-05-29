#include "utils.h"

#define TIMEOUT 10
#define MAX_LEN (30 * sizeof(int16_t))
volatile sig_atomic_t last_signal = 0;

void sigalrm_handler(int sig)
{
    last_signal = sig;
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s address port n\n", name);
}

int make_socket(void)
{
    int sock;
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        ERR("socket");
    return sock;
}

struct sockaddr_in make_address(char *address, char *port)
{
    int ret;
    struct sockaddr_in addr;
    struct addrinfo *result;
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    if ((ret = getaddrinfo(address, port, &hints, &result))) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        exit(EXIT_FAILURE);
    }
    addr = *(struct sockaddr_in *)(result->ai_addr);
    freeaddrinfo(result);
    return addr;
}

int connect_socket(char *name, char *port)
{
    struct sockaddr_in addr;
    int socketfd;
    socketfd = make_socket();
    addr = make_address(name, port);

    if (connect(socketfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
        if (errno != EINTR)
            ERR("connect");
        else {
            int status;
            socklen_t size = sizeof(int);
            if (getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &status, &size) < 0)
                ERR("getsockopt");
            if (status != 0)
                ERR("connect");
        }
    }

    return socketfd;
}


void communicate(int fd, int16_t num)
{
    if (TEMP_FAILURE_RETRY(write(fd, (char *)&num, sizeof(int16_t))) < 0)
        ERR("write");

    alarm(TIMEOUT);
    uint16_t count;
    int16_t *nums;

    while (recv(fd, (char *)&count, sizeof(int16_t), 0) < 0) {
        if (EINTR != errno)
            ERR("recv:");
        if (SIGALRM == last_signal)
            break;
    }

    nums = malloc(ntohs(count) * sizeof(int16_t));
    while (recv(fd, (char *)&nums, count * sizeof(int16_t), 0) < 0) {
        if (EINTR != errno)
            ERR("recv:");
        if (SIGALRM == last_signal)
            break;
    }

    printf("Got %d numbers: ", count);
    for(int i = 0; i < count; i++) {
        printf("%d", ntohs(nums[i]));
    }
    printf("\n");

    free(nums);

    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
        ERR("close");
}

int main(int argc, char **argv)
{
    int fd;
    if (argc != 3)
        usage(argv[0]);
    if (set_handler(SIG_IGN, SIGPIPE))
        ERR("Sigpipe ignore");
    if (set_handler(sigalrm_handler, SIGALRM))
        ERR("Seting SIGALRM:");
    fd = connect_socket(argv[1], argv[2]);
    communicate(fd, htons(atoi(argv[3])));
    return EXIT_SUCCESS;
}