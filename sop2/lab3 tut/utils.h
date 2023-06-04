#ifndef KUZU_UTILS
#define KUZU_UTILS

#define _GNU_SOURCE
#define _XOPEN_SOURCE 500

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ftw.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>

extern char *optarg;
extern int opterr, optind, optopt;

/// Logging utils
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))
#define HERR(source) (fprintf(stderr, "%s(%d) at %s:%d\n", source, h_errno, __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define PUTENV(string) if (putenv(string) != 0) perror("putenv")


/// Sockets
// Global
struct sockaddr_in make_address(char *address, char* port) {
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

// UNIX sockets
int make_socket_local(char* name, int type, struct sockaddr_un *addr) {
    int socketfd;
    if ((socketfd = socket(PF_UNIX, type, 0)) < 0)
        ERR("socket");

    memset(addr, 0, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strncpy(addr->sun_path, name, sizeof(addr->sun_path) - 1);

    return socketfd;
}

int bind_socket_local(char* name, int type, int backlog) {
    struct sockaddr_un addr;
    int socketfd;
    if (unlink(name) < 0 && errno != ENOENT)
        ERR("unlink");

    socketfd = make_socket_local(name, type, &addr);
    if (bind(socketfd, (struct sockaddr *)&addr, SUN_LEN(&addr)) < 0)
        ERR("bind");
    if (listen(socketfd, backlog) < 0)
        ERR("listen");

    return socketfd;
}

int connect_socket_local(char *name, int type) {
    struct sockaddr_un addr;
    int socketfd;
    socketfd = make_socket_local(name, type, &addr);

    if (connect(socketfd, (struct sockaddr *)&addr, SUN_LEN(&addr)) < 0) {
        if (errno != EINTR)
            ERR("connect");
        else {
            fd_set wfds;
            int status;
            socklen_t size = sizeof(int);

            FD_ZERO(&wfds);
            FD_SET(socketfd, &wfds);

            if (TEMP_FAILURE_RETRY(select(socketfd + 1, NULL, &wfds, NULL, NULL)) < 0)
                ERR("select");

            if (getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &status, &size) < 0)
                ERR("getsockopt");
            if (0 != status)
                ERR("connect");
        }
    }

    return socketfd;
}

// TCP Sockets
int make_socket_tcp(void) {
    int socketfd;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        ERR("socket");

    return socketfd;
}

int bind_socket_tcp(uint16_t port, int backlog) {
    struct sockaddr_in addr;
    int socketfd, t = 1;

    socketfd = make_socket_tcp();

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)))
        ERR("setsockopt");

    if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        ERR("bind");
    if (listen(socketfd, backlog) < 0)
        ERR("listen");

    return socketfd;
}

int connect_socket_tcp(char *name, char *port) {
    struct sockaddr_in addr;
    int socketfd;
    socketfd = make_socket_tcp();
    addr = make_address(name, port);

    if (connect(socketfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
        if (errno != EINTR)
            ERR("connect");
        else {
            fd_set wfds;
            int status;
            socklen_t size = sizeof(int);

            FD_ZERO(&wfds);
            FD_SET(socketfd, &wfds);

            if (TEMP_FAILURE_RETRY(select(socketfd + 1, NULL, &wfds, NULL, NULL)) < 0)
                ERR("select");

            if (getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &status, &size) < 0)
                ERR("getsockopt");
            if (0 != status)
                ERR("connect");
        }
    }

    return socketfd;
}

/// Thread utils
int safesleep(struct timespec ts) {
    int res;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int sleepms(unsigned milisec) {
    time_t sec = (int) (milisec / 1000);
    milisec -= sec * 1000;
    struct timespec req = {sec, milisec * 1e6};
    return safesleep(req);
}

int set_handler(void (*f)(int), int sigNo) {
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}

//void sigchld_handler(int sig, siginfo_t *s, void *p)
//{
//    pid_t pid;
//    kill(0, SIGINT);
//    for (;;) {
//        pid = waitpid(0, NULL, WNOHANG);
//        if (pid == 0)
//            return;
//        if (pid <= 0) {
//            if (errno == ECHILD)
//                return;
//            ERR("waitpid");
//        }
//    }
//}

ssize_t bulk_read(int fd, char *buf, size_t count) {
    ssize_t c;
    ssize_t len = 0;
    do {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;
        if (c == 0)
            return len; // EOF
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

ssize_t bulk_write(int fd, char *buf, size_t count) {
    ssize_t c;
    ssize_t len = 0;
    do {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

/// Random utils
/*
 * Returns random integer in the range [min, max]
 *
 * Uses rand(), so remember to use srand()!
 */
int randint(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

/*
 * Returns random double between [0, 1]
 */
double randdouble() {
    return (double) rand() / (double) RAND_MAX;
}

/*
 * Returns random double between [min, max]
 */
double randrange(double min, double max) {
    return min + (double) rand() / ((double) RAND_MAX / (max - min));
}

/// String utils
int startswith(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

int endswithext(char *name, char *extension) {
    const char *ext = strrchr(name, '.');
    return ext && strcmp(extension, ext + 1) == 0;
}

#endif