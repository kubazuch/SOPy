#include "utils.h"
#include <mqueue.h>

#define D_NUM 3
#define MSG_LEN 6
#define DISH_LEN 8
#define MAX_MSG 10

void server_work(mqd_t *dishes, mqd_t wd) {

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

    char ordered[MSG_LEN];
    if(TEMP_FAILURE_RETRY(mq_receive(d[0], ordered, MSG_LEN, 0)) < 0)
        ERR("mq receive");
    printf("Order received from %s\n", ordered);
    mq_close(d[0]);
    if(mq_unlink("/DANIE1"))
        ERR("mq unlink");

    return EXIT_SUCCESS;
}