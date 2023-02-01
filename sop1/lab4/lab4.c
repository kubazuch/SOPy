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

#define BUFFERS 3
#define LEN 8

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define SHIFT(counter, x) ((counter + x) % BUFFERS)

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

    printf("Invalid char sequence: %c%c%c\n", a, b, c);
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
    for (int i = 0; i < BUFFERS; i++)
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

void fillaiostruct(struct aiocb *aiocbs, char **buffer, int fd, int blocksize) {
    for (int i = 0; i < BUFFERS; i++) {
        memset(&aiocbs[i], 0, sizeof(struct aiocb));
        aiocbs[i].aio_fildes = fd;
        aiocbs[i].aio_offset = 0;
        aiocbs[i].aio_nbytes = blocksize;
        aiocbs[i].aio_buf = (void *) buffer[i];
        aiocbs[i].aio_sigevent.sigev_notify = SIGEV_NONE;
    }
}

void readdata(struct aiocb *aiocbs, off_t offset, int in) {
    aiocbs->aio_offset = offset;
    aiocbs->aio_fildes = in;
    if (aio_read(aiocbs) == -1)
        ERR("Cannot read");
}

void writedata(struct aiocb *aiocbs, off_t offset, int out) {
    aiocbs->aio_offset = offset;
    aiocbs->aio_fildes = out;
    if (aio_write(aiocbs) == -1)
        ERR("Cannot write");
}

void syncdata(struct aiocb *aiocbs) {
    suspend(aiocbs);
    if (aio_fsync(O_SYNC, aiocbs) == -1)
        ERR("Cannot sync\n");
    suspend(aiocbs);
}

void work(struct aiocb *aiocbs, char **buffer, int in, int out, int count) {
    int curpos = 0;
    char *tmp = malloc(9 * sizeof(char));
    int cnt;
    readdata(&aiocbs[1], 0, in);
    cnt = suspend(&aiocbs[1]);
    int off = 8;
    for (int j = 0; j < count; j++) {
        if (j > 0)
            writedata(&aiocbs[curpos], off, out);
        if (j < count - 1)
            readdata(&aiocbs[SHIFT(curpos, 2)], off + 8, in);

        int working = SHIFT(curpos, 1);
        tmp[0] = convert('-', buffer[working][0], buffer[working][1]);
        tmp[9] = convert(buffer[working][cnt - 1], '-', '-');
        for (int i = 0; i < cnt; i++) {
            tmp[i + 1] = convert(buffer[working][i], buffer[working][i + 1], buffer[working][i + 2]);
        }

        memcpy(buffer[working], tmp, cnt);

        if (j > 0)
            syncdata(&aiocbs[curpos]);
        if (j < count - 1)
            cnt = suspend(&aiocbs[SHIFT(curpos, 2)]);
        curpos = SHIFT(curpos, 1);
        off += 8;
    }
    writedata(&aiocbs[curpos], off, out);
    suspend(&aiocbs[curpos]);

    free(tmp);
}

int main(int argc, char *argv[]) {
    char *inname, *outname, *buffer[BUFFERS];
    struct aiocb aiocbs[BUFFERS];
    int in, out;

    if (argc != 3)
        usage(argv[0]);
    inname = argv[1];
    outname = argv[2];

    if ((in = TEMP_FAILURE_RETRY(open(inname, O_RDONLY))) == -1)
        ERR("open");
    if ((out = TEMP_FAILURE_RETRY(open(outname, O_WRONLY | O_CREAT, 0644))) == -1)
        ERR("open");

    int blocks = ceil(getfilelength(in) / 8.0);

    for (int i = 0; i < BUFFERS; i++)
        if ((buffer[i] = (char *) calloc(LEN, sizeof(char))) == NULL)
            ERR("calloc");
    fillaiostruct(aiocbs, buffer, in, LEN);

    work(aiocbs, buffer, in, out, blocks);

    cleanup(buffer, out);
    if (TEMP_FAILURE_RETRY(close(in)) == -1)
        ERR("close");
    if (TEMP_FAILURE_RETRY(close(out)) == -1)
        ERR("close");
    return EXIT_SUCCESS;
}