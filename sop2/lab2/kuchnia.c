#include "utils.h"
#include <mqueue.h>

#define MSG_LEN 35
#define MAX_MSG 10

volatile sig_atomic_t should_end = 0;
void sigint_handler(int sig, siginfo_t *info, void *p)
{
    should_end = 1;
}

void handle_end(mqd_t kasa, mqd_t wydanie) {
    struct mq_attr attr;
    attr.mq_flags = O_NONBLOCK;
    mq_setattr(kasa, &attr, NULL);

    char kasa_str[MSG_LEN];
    char wydanie_str[MSG_LEN];
    for (;;) {
        if (TEMP_FAILURE_RETRY(mq_receive(kasa, kasa_str, MSG_LEN, NULL)) < 0) {
            if (errno == EAGAIN)
                break;
            else
                ERR("mq_receive");
        }
        printf("Kasa: %s\n", kasa_str);
        sleep(2);
        char * space = strchr(kasa_str, ' ');
        *space = '\0';
        unsigned price = strtol(kasa_str, NULL, 10);
        switch (price) {
            case 10:
                snprintf(wydanie_str, MSG_LEN, "%s frytki", space+1);
                break;
            case 12:
                snprintf(wydanie_str, MSG_LEN, "%s hotdog", space+1);
                break;
            case 5:
                snprintf(wydanie_str, MSG_LEN, "%s cola", space+1);
                break;
        }
        if(TEMP_FAILURE_RETRY(mq_send(wydanie, wydanie_str, MSG_LEN, 0)) < 0)
            ERR("mq send");
    }
}

void kitchen_work(mqd_t kasa, mqd_t wydanie) {
    char kasa_str[MSG_LEN];
    char wydanie_str[MSG_LEN];
    for(;;) {
        if(should_end) {
            handle_end(kasa, wydanie);
            break;
        }
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
            ERR("clock_gettime()");
        }

        ts.tv_sec += 10;
        if((mq_timedreceive(kasa, kasa_str, MSG_LEN, NULL, &ts)) < 0)
        {
            if(errno == ETIMEDOUT)
            {
                printf("Przekroczony czas oczekiwania\n");
                break;
            }
            if(errno == EINTR)
                continue;
            ERR("mq receive");
        }

        printf("Kasa: %s\n", kasa_str);
        sleep(2);
        char * space = strchr(kasa_str, ' ');
        *space = '\0';
        unsigned price = strtol(kasa_str, NULL, 10);
        switch (price) {
            case 10:
                snprintf(wydanie_str, MSG_LEN, "%s frytki", space+1);
                break;
            case 12:
                snprintf(wydanie_str, MSG_LEN, "%s hotdog", space+1);
                break;
            case 5:
                snprintf(wydanie_str, MSG_LEN, "%s cola", space+1);
                break;
        }
        if((mq_send(wydanie, wydanie_str, MSG_LEN, 0)) < 0)
            ERR("mq send");
    }
}

int main(int argc, char **argv)
{
    mqd_t menu, kasa, wydanie;

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_LEN;

    set_handler(sigint_handler, SIGINT);

    if ((menu = TEMP_FAILURE_RETRY(mq_open("/MENU", O_RDWR | O_CREAT, 0600, &attr))) == (mqd_t)-1)
        ERR("mq open menu");
    if ((kasa = TEMP_FAILURE_RETRY(mq_open("/KASA", O_RDWR | O_CREAT, 0600, &attr))) == (mqd_t)-1)
        ERR("mq open kasa");
    if ((wydanie = TEMP_FAILURE_RETRY(mq_open("/WYDANIE", O_RDWR | O_CREAT, 0600, &attr))) == (mqd_t)-1)
        ERR("mq open wydanie");

    char menu_str[MSG_LEN];
    snprintf(menu_str, MSG_LEN, "10 frytki\n12 hotdog\n5 cola\n");
    for(int i = 0; i < 2; ++i) {
        if(TEMP_FAILURE_RETRY(mq_send(menu, menu_str, MSG_LEN, 0)) < 0)
            ERR("mq send");
    }

    kitchen_work(kasa, wydanie);

    mq_close(menu);
    mq_close(kasa);
    mq_close(wydanie);
    if(mq_unlink("/MENU"))
        ERR("mq unlink");
    if(mq_unlink("/KASA"))
        ERR("mq unlink");
    if(mq_unlink("/WYDANIE"))
        ERR("mq unlink");

    return EXIT_SUCCESS;
}