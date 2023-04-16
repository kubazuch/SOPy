#include "utils.h"
#include <mqueue.h>

#define D_NUM 3
#define MSG_LEN 6
#define DISH_LEN 8
#define MAX_MSG 10

void server_work(mqd_t *dishes, mqd_t wd) {
    char ordered[MSG_LEN];
    for(;;) {
        if(TEMP_FAILURE_RETRY(mq_receive(dishes[0], ordered, MSG_LEN, 0)) < 0)
            ERR("mq receive");
        printf("Order received from %s\n", ordered);
        if(TEMP_FAILURE_RETRY(mq_send(wd, ordered, MSG_LEN, 0)) < 0)
            ERR("mq send");
    }
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

    server_work(d, wd);

    mq_close(d[0]);
    mq_close(wd);
    if(mq_unlink("/DANIE1"))
        ERR("mq unlink");
    if(mq_unlink("/WYDANIE"))
        ERR("mq unlink");

    return EXIT_SUCCESS;
}