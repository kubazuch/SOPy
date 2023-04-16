#include "utils.h"
#include <mqueue.h>

#define D_NUM 3
#define MSG_LEN 6
#define DISH_LEN 8
#define MAX_MSG 10

struct queue_data {
    mqd_t *d;
    mqd_t *wd;
};

volatile sig_atomic_t should_end = 0;
void sigint_handler(int sig, siginfo_t *info, void *p)
{
    should_end = 1;
}

void mq_handler(int sig, siginfo_t *info, void *p)
{
    struct queue_data *data;

    data = (struct queue_data *)info->si_value.sival_ptr;

    static struct sigevent noti;
    noti.sigev_notify = SIGEV_SIGNAL;
    noti.sigev_signo = SIGRTMIN;
    noti.sigev_value.sival_ptr = data;
    if (mq_notify(*(data->d), &noti) < 0)
        ERR("mq_notify");

    char ordered[MSG_LEN];
    while (!should_end) {
        if (mq_receive(*(data->d), ordered, MSG_LEN, NULL) < 1) {
            if (errno == EAGAIN)
                break;
            else
                ERR("mq_receive");
        }
        sleep(1);
        if(TEMP_FAILURE_RETRY(mq_send(*(data->wd), ordered, MSG_LEN, 0)) < 0)
            ERR("mq send");
        printf("%s\n", ordered);
    }
}

void server_work(mqd_t *dishes, mqd_t wd) {
    struct timespec req = {10, 0};
    for(;;) {
        if(nanosleep(&req, NULL) == 0 || should_end)
            break;
        else if (errno == EINTR)
            continue;
        else
            ERR("nanosleep");
    }
    printf("Kończenie\n");
    char ordered[MSG_LEN];
    for(int i = 0; i < D_NUM; ++i) {
        for (;;) {
            if (TEMP_FAILURE_RETRY(mq_receive(dishes[i], ordered, MSG_LEN, NULL)) < 1) {
                if (errno == EAGAIN)
                    break;
                else
                    ERR("mq_receive");
            }
            printf("Nie realizuję zamówienia %s\n", ordered);
        }
    }

}

void create_queues(mqd_t* wd, mqd_t d[]) {
    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_LEN;

    if ((*wd = TEMP_FAILURE_RETRY(mq_open("/WYDANIE", O_RDWR | O_CREAT, 0600, &attr))) == (mqd_t)-1)
        ERR("mq open in");

    char dish_num[DISH_LEN];

    static struct queue_data data[DISH_LEN];
    static struct sigevent noti[DISH_LEN];
    for(int i = 0; i < D_NUM; i++) {
        snprintf(dish_num, DISH_LEN, "/DANIE%d", i + 1);
        if ((d[i] = TEMP_FAILURE_RETRY(mq_open(dish_num, O_RDWR | O_NONBLOCK | O_CREAT, 0600, &attr))) == (mqd_t)-1)
            ERR("DANIE open in");

        data[i].d = &d[i];
        data[i].wd = wd;

        noti[i].sigev_notify = SIGEV_SIGNAL;
        noti[i].sigev_signo = SIGRTMIN;
        noti[i].sigev_value.sival_ptr = &(data[i]);
        if (mq_notify(d[i], &(noti[i])) < 0)
            ERR("mq_notify");
    }
}

int main(int argc, char **argv)
{
    mqd_t wd, d[D_NUM];

    set_handler(mq_handler, SIGRTMIN);
    set_handler(sigint_handler, SIGINT);

    create_queues(&wd, d);
    server_work(d, wd);

    mq_close(wd);
    char dish_num[DISH_LEN];
    for(int i = 0; i < D_NUM; i++) {
        mq_close(d[i]);
        snprintf(dish_num, DISH_LEN, "/DANIE%d", i + 1);
        if(mq_unlink(dish_num))
            ERR("mq unlink");
    }

    if(mq_unlink("/WYDANIE"))
        ERR("mq unlink");

    return EXIT_SUCCESS;
}