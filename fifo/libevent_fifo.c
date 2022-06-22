#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include<event2/event.h>

#define BUFFER_SIZE 1024
const char * file_name="file";
const char * file_name2="file1";
char buffer[BUFFER_SIZE];

//回调函数
void read_callback(evutil_socket_t fd,short ev,void * user_arg){
    bzero(buffer,BUFFER_SIZE);
    int nbytes;
    while((nbytes=read(fd,buffer,BUFFER_SIZE))>0){
        fprintf(stderr,"read len= %d from:%d端口 buf:%s",nbytes,fd,buffer);
    }
    if(strncmp(buffer,"quit",4)==0){
        printf("fd(%d)quit...\n",fd);
        //event_base_loopbreak((struct event_base*)user_arg);
        //如果多个fifo处理，不要退出事件循环，将本事件从base中删除
        struct event* this_event = event_base_get_running_event((struct event_base* )user_arg);
        event_free(this_event);            
    }
}

//void write_callback(evutil_socket_t,short ev,void * user_arg){}

int main(int argc, char const *argv[])
{
    /*printf("%s\n",event_get_version());

    struct event_base *base;
    base=event_base_new();

    const char * method=event_base_get_method(base);
    printf("libevent uses %s\n",method);*/


    int fd=open(file_name,O_RDONLY | O_NONBLOCK);
    if(fd==-1){
        perror("open error");
    }
    int fd2=open(file_name2,O_RDONLY | O_NONBLOCK);
    if(fd2==-1){
        perror("open error");
    }

    //创建event_base对象
    struct  event_base *base;
    base=event_base_new();
        
    //创建事件
    struct event* read_event=event_new(base,fd,EV_READ |EV_PERSIST,read_callback ,(void *)base);
    event_add(read_event,NULL);

    //创建事件2
    struct event* read_event2=event_new(base,fd2,EV_READ |EV_PERSIST,read_callback ,(void *)base);
    event_add(read_event2,NULL);

    event_base_dispatch(base);

    event_base_free(base);


    return 0;
}