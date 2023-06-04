#include "../utils.h"

#define BACKLOG 3
#define MAXBUF 576
#define MAXADDR 5

struct connections {
    int free;
    int32_t chunkNo;
    struct sockaddr_in addr;
};

int findIndex(struct sockaddr_in addr, struct connections con[MAXADDR])
{
    int i, empty = -1, pos = -1;
    for (i = 0; i < MAXADDR; i++) {
        if (con[i].free)
            empty = i;
        else if (0 == memcmp(&addr, &(con[i].addr), sizeof(struct sockaddr_in))) {
            pos = i;
            break;
        }
    }
    if (-1 == pos && empty != -1) {
        con[empty].free = 0;
        con[empty].chunkNo = 0;
        con[empty].addr = addr;
        pos = empty;
    }
    return pos;
}

void doServer(int fd)
{
    struct sockaddr_in addr;
    struct connections con[MAXADDR];
    char buf[MAXBUF+1];
    socklen_t size = sizeof(addr);
    int i;
    int32_t chunkNo, last;
    for (i = 0; i < MAXADDR; i++)
        con[i].free = 1;
    for (;;) {
        if (TEMP_FAILURE_RETRY(recvfrom(fd, buf, MAXBUF, 0, &addr, &size) < 0))
            ERR("read:");
        if ((i = findIndex(addr, con)) >= 0) {
            chunkNo = ntohl(*((int32_t *)buf));
            last = ntohl(*(((int32_t *)buf) + 1));
            if (chunkNo > con[i].chunkNo + 1)
                continue;
            else if (chunkNo == con[i].chunkNo + 1) {
                if (last) {
                    printf("Last Part %d\n%s\n", chunkNo, buf + 2 * sizeof(int32_t));
                    con[i].free = 1;
                } else
                    printf("Part %d\n%s\n", chunkNo, buf + 2 * sizeof(int32_t));
                con[i].chunkNo++;
            }
            if (TEMP_FAILURE_RETRY(sendto(fd, buf, MAXBUF, 0, &addr, size)) < 0) {
                if (EPIPE == errno)
                    con[i].free = 1;
                else
                    ERR("send:");
            }
        }
    }
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s port\n", name);
}

int main(int argc, char **argv)
{
    int fd;
    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (set_handler(SIG_IGN, SIGPIPE))
        ERR("Seting SIGPIPE:");
    fd = bind_socket_udp(atoi(argv[1]));
    doServer(fd);
    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
        ERR("close");
    fprintf(stderr, "Server has terminated.\n");
    return EXIT_SUCCESS;
}