#include "utils.h"
#include <mqueue.h>

#define D_NUM 3
#define MSG_LEN 6
#define DISH_LEN 8
#define MAX_MSG 10

void client_work(mqd_t *dishes, mqd_t wd) {
    int pid = getpid();
    char mypid[MSG_LEN];
    snprintf(mypid, MSG_LEN, "%d", pid);
    if(TEMP_FAILURE_RETRY(mq_send(dishes[0], mypid, MSG_LEN, 0)))
        ERR("mq_send");

    char wydanie[MSG_LEN];
    int times = 0;
    for(;;) {
        if (times >= 5) {
            printf("Ide do baru mlecznego\n");
            exit(EXIT_SUCCESS);
        }

        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
            ERR("clock_gettime()");
        }

        ts.tv_sec += 5;
        if(TEMP_FAILURE_RETRY(mq_timedreceive(wd, wydanie, MSG_LEN, NULL, &ts)) < 0)
        {
            if(errno == ETIMEDOUT)
            {
                printf("Ide do baru mlecznego\n");
                exit(EXIT_SUCCESS);
            }
            ERR("mq receive");
        }

        if(strcmp(mypid, wydanie) == 0)
            break;

        sleep(1);
        times++;
    }
    printf("MNIAM\n");
}

int main(int argc, char **argv)
{
    mqd_t d[D_NUM];
    mqd_t wd;

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_LEN;

    if ((d[0] = TEMP_FAILURE_RETRY(mq_open("/DANIE1", O_RDWR | O_CREAT, 0600, &attr))) == (mqd_t)-1)
        ERR("DANIE open in");

    if ((wd = TEMP_FAILURE_RETRY(mq_open("/WYDANIE", O_RDWR | O_CREAT, 0600, &attr))) == (mqd_t)-1)
        ERR("mq open in");

    client_work(d, wd);
    mq_close(d[0]);
    mq_close(wd);

    return EXIT_SUCCESS;
}