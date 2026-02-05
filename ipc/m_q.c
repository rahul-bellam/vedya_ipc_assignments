#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
 
int main() {
    const char *queue_name = "/myqueue";
    mqd_t mqd;
    struct mq_attr attr;
    mqd = mq_open(queue_name, O_RDONLY);
    char buffer[1024];
    while (1){
    mq_receive(mqd, buffer, 1024, NULL);
    printf("%s", buffer);
        }
    mq_close(mqd);
 
   //polling mode
    return 0;
}