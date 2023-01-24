#define _GNU_SOURCE

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#define BUFFERS 1

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

char convert(char a, char b, char c) {
    switch (a) {
        case '#':
            switch (b) {
                case '#':
                    return c == '#' ? '-' : '#';
                case '-':
                    return '#';
            }
        case '-':
            return b;
    }

    printf("Invalid char sequence: %c%c%c\n", a,b,c);
    exit(1);
}

off_t getfilelength(int fd) {
    struct stat buf;
    if (fstat(fd, &buf) == -1)
        ERR("fstat");
    return buf.st_size;
}

void usage(char *progname) {
    fprintf(stderr, "%s input output\n", progname);
    exit(EXIT_FAILURE);
}

void cleanup(char **buffers, int out) {
    for (int i = 0; i < 2; i++)
        free(buffers[i]);
    if (TEMP_FAILURE_RETRY(fsync(out)) == -1)
        ERR("fsync");
}

int suspend(struct aiocb *aiocbs) {
    struct aiocb *aiolist[1];
    aiolist[0] = aiocbs;

    while (aio_suspend((const struct aiocb *const *) aiolist, 1, NULL) == -1) {
        if (errno == EINTR)
            continue;
        ERR("aio_suspend");
    }
    if (aio_error(aiocbs) != 0)
        ERR("aio_suspend");
    int ret;
    if ((ret = aio_return(aiocbs)) == -1)
        ERR("aio_return");

    return ret;
}

void fillaiostruct(struct aiocb *aiocbs, char *buffer, int fd, int blocksize) {
    memset(aiocbs, 0, sizeof(struct aiocb));
    aiocbs->aio_fildes = fd;
    aiocbs->aio_offset = 0;
    aiocbs->aio_nbytes = blocksize;
    aiocbs->aio_buf = (void *) buffer;
    aiocbs->aio_sigevent.sigev_notify = SIGEV_NONE;
}

void work(struct aiocb *aiocbs, char **buffer, int bsize) {
    aio_read(&aiocbs[0]);
    suspend(&aiocbs[0]);

    buffer[0][0] = '-';
    buffer[0][bsize+1] = '-';
    buffer[0][bsize+2] = '-';
    for (int i = 0; i <= bsize; i++){
        buffer[1][i] = convert(buffer[0][i],buffer[0][i+1],buffer[0][i+2]);
    }

    aio_write(&aiocbs[1]);
    suspend(&aiocbs[1]);
}

int main(int argc, char *argv[]) {
    char *inname, *outname, *buffer[2];
    struct aiocb aiocbs[2];
    int in, out;

    if (argc != 3)
        usage(argv[0]);
    inname = argv[1];
    outname = argv[2];

    if ((in = TEMP_FAILURE_RETRY(open(inname, O_RDONLY))) == -1)
        ERR("open");
    if ((out = TEMP_FAILURE_RETRY(open(outname, O_WRONLY | O_CREAT, 0644))) == -1)
        ERR("open");
    int len = getfilelength(in);
    //int blocks = ceil(getfilelength(in) / 8.0);
    //fprintf(stderr, "Blocks: %d\n", blocks);

    for (int i = 0; i < 2; i++)
        if ((buffer[i] = (char *) calloc(len+3, sizeof(char))) == NULL)
            ERR("calloc");
    fillaiostruct(&aiocbs[0], buffer[0]+1, in, len);
    fillaiostruct(&aiocbs[1], buffer[1], out, len+1);

    work(aiocbs, buffer, len);

    cleanup(buffer, out);
    if (TEMP_FAILURE_RETRY(close(in)) == -1)
        ERR("close");
    if (TEMP_FAILURE_RETRY(close(out)) == -1)
        ERR("close");
    return EXIT_SUCCESS;
}