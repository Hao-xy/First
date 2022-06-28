#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
//文件状态
#include <sys/stat.h>
//定义了很多宏和open,fcntl函数原型
#include <fcntl.h>
//包含了许多UNIX系统服务的函数原型，例如read函数、write函数和getpid函数
#include <event2/event.h>
#include<mysql/mysql.h>

#define BUFFER_SIZE 5005
#define BACKLOG (10) //请求队列
#define mysql

char buffer[BUFFER_SIZE] = {0};
struct sockaddr_in clients[(BACKLOG)*10];


void read_callback(evutil_socket_t fd, short ev, void *user_arg);

void printTimeUsec()
{
    struct tm *ptm;
    struct timeval time;
    gettimeofday(&time, NULL);
    ptm = localtime(&time.tv_sec);
    if (ptm != NULL)
    {
        printf("%d-%d-%d %d:%d:%d:%ld\n",
               ptm->tm_year + 1900,
               ptm->tm_mon + 1,
               ptm->tm_mday,
               ptm->tm_hour,
               ptm->tm_min,
               ptm->tm_sec,
               time.tv_usec % 1000000 / 1000);
    }
}

void accept_client(evutil_socket_t fd, short ev, void *user_arg)
{
    //accept
    int connfd;
    struct sockaddr_in client;
    int len = sizeof(client);
    connfd = accept(fd, (struct sockaddr *)&client, &len);
    if (connfd == -1)
    {
        perror("accept");
        exit(1);
    }
    printf("一个客户端已经连接fd=%d\n", connfd);

    struct event* read_event = event_new(user_arg, connfd, EV_READ | EV_PERSIST, read_callback, (void*)user_arg);
    event_add(read_event, NULL);

    clients[connfd] = client;
}

void read_callback(evutil_socket_t fd, short ev, void *user_arg)
{
    int nbytes;
    char sendbuf[BUFFER_SIZE] = {0};
    //接收 client
    if ((nbytes = recv(fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        fprintf(stdout, "fd:%d port:%d read:%d(bytes) text:%s\n", fd, clients[fd].sin_port, nbytes, buffer);
    }
    if (strncmp(buffer, "bye", 3) == 0 || nbytes == 0)
    {
        printf("one client exit!!\n");
        event_del(event_base_get_running_event((struct event_base *)user_arg));
        close(fd);
        return;
    }

    for (int i = 0; i < strlen(buffer); i++)
    {
        sendbuf[i] = buffer[strlen(buffer) - 1 - i];
    }
    sendbuf[strlen(sendbuf)] = '\0';

    send(fd, sendbuf, strlen(sendbuf), 0);

    bzero(buffer, sizeof(buffer));
}


int main(int argc, char const *argv[])
{
    printf("server start!!!!\n");
    int listenfd;
    int nfds = 0;
    struct sockaddr_in server;

    int SERVERPORT = 10086;
    int val = 1;
    bzero(&clients, sizeof(clients));

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) //socket fail
    {
        perror("socket");
        exit(1);
    }
    printf("listenfd=%d\n", listenfd);

    //为服务器主线程绑定ip和port
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));
    server.sin_family = AF_INET;//设置为IP通信
    server.sin_port = htons(SERVERPORT);//服务器端口号
    server.sin_addr.s_addr = inet_addr("127.0.0.1");//服务器IP地址
    bzero(&(server.sin_zero), 8);

    //bind
    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    //listen
    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }
    printf("pid=%d\n", getpid());

    struct event_base* base = event_base_new();//新建事件
    struct event* read_event = event_new(base, listenfd, EV_READ | EV_PERSIST, accept_client, (void*)base);//监听事件创建
    event_add(read_event, NULL);//事件加入
    event_base_dispatch(base);//阻塞

    event_base_free(base);

    close(listenfd);
    printf("\n\n\n\n\n\n");
    return 0;
}
