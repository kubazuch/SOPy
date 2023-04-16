#include "utils.h"
#include <mqueue.h>

#define D_NUM 3
#define MSG_LEN 6
#define DISH_LEN 8
#define MAX_MSG 10

int main(int argc, char **argv)
{
    mqd_t d[D_NUM];
    mqd_t wd;

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_LEN;

    if ((d[0] = TEMP_FAILURE_RETRY(mq_open("/DANIE1", O_RDWR, 0600, &attr))) == (mqd_t)-1)
        ERR("DANIE open in");

    char ordered[MSG_LEN];
    snprintf(ordered, MSG_LEN, "%d", getpid());
    if(TEMP_FAILURE_RETRY(mq_send(d[0], ordered, MSG_LEN, 0)) < 0)
        ERR("mq send");
    mq_close(d[0]);

    return EXIT_SUCCESS;
}