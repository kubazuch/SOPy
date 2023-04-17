#include "utils.h"
#include <mqueue.h>

#define MSG_LEN 35
#define MAX_MSG 10
#define DISHES 3

char* choose(char* menu_str) {
    int choice = rand() % DISHES;
    char *tmp;
    char *item = strtok_r( menu_str, "\n", &tmp);
    int i = 0;
    while( i != choice && item != NULL )
    {
        item = strtok_r( NULL, "\n", &tmp);
        i++;
    }
    return item;
}

void order(mqd_t kasa, mqd_t wydanie, char *choice) {
    char * space = strchr(choice, ' ');
    *space = '\0';

    char order_str[MSG_LEN];
    snprintf(order_str, MSG_LEN, "%s %d", choice, getpid());
    if(TEMP_FAILURE_RETRY(mq_send(kasa, order_str, MSG_LEN, 0)) < 0)
        ERR("mq send");
    printf( "[%d] Zamówiłem: %s\n", getpid(), choice );
    sleep(1);

    char receive_str[MSG_LEN];
    char process_str[MSG_LEN];
    for(;;) {
        if(TEMP_FAILURE_RETRY(mq_receive(wydanie, receive_str, MSG_LEN, NULL)) < 0)
            ERR("mq send");
        sleep(1);

        strcpy(process_str, receive_str);
        space = strchr(process_str, ' ');
        *space = '\0';
        pid_t pid = strtol(process_str, NULL, 10);
        if(pid == getpid()) break;
        printf("[%d] Zamówienie %s nie jest moje!\n", getpid(), receive_str);
        if(TEMP_FAILURE_RETRY(mq_send(wydanie, receive_str, MSG_LEN, 0)) < 0)
            ERR("mq send");
        sleep(1);
    }
    printf( "[%d] Moje zamówienie: %s\n", getpid(), receive_str );
}

int main(int argc, char **argv)
{
    srand(getpid()*time(NULL));
    mqd_t menu, kasa, wydanie;

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_LEN;

    if ((menu = TEMP_FAILURE_RETRY(mq_open("/MENU", O_RDWR, 0600, &attr))) == (mqd_t)-1)
        ERR("mq open in");
    if ((kasa = TEMP_FAILURE_RETRY(mq_open("/KASA", O_RDWR, 0600, &attr))) == (mqd_t)-1)
        ERR("mq open in");
    if ((wydanie = TEMP_FAILURE_RETRY(mq_open("/WYDANIE", O_RDWR | O_CREAT, 0600, &attr))) == (mqd_t)-1)
        ERR("mq open wydanie");

    char menu_str[MSG_LEN];
    char menu_cpy[MSG_LEN];
    char choice[MSG_LEN];
    printf("[%d] Czytam z menu\n", getpid());
    if(TEMP_FAILURE_RETRY(mq_receive(menu, menu_str, MSG_LEN, NULL)) < 0)
        ERR("mq send");
    printf("[%d] Menu:\n%s\n", getpid(), menu_str);

    strcpy(menu_cpy, menu_str);
    char *item = choose(menu_cpy);
    strcpy(choice, item);
    sleep(1);

    if(TEMP_FAILURE_RETRY(mq_send(menu, menu_str, MSG_LEN, 0)) < 0)
        ERR("mq send");
    sleep(1);
    printf("[%d] Odłożyłem menu do kolejki\n", getpid());

    order(kasa, wydanie, choice);

    mq_close(menu);
    mq_close(kasa);
    mq_close(wydanie);

    return EXIT_SUCCESS;
}