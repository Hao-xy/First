#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
//文件状态
#include <sys/stat.h>
//定义了很多宏和open,fcntl函数原型
#include <fcntl.h>
//包含了许多UNIX系统服务的函数原型，例如read函数、write函数和getpid函数
#include <unistd.h>

int processClient(int connfd, struct sockaddr_in client)
{
    char recvbuf[5005] = {0};
    char sendbuf[5005] = {0};
    int num;
    bzero(recvbuf, sizeof(recvbuf));
    bzero(sendbuf, sizeof(sendbuf));
    num = recv(connfd, recvbuf, 5005, 0);
    recvbuf[num] = '\0';

    printf("\nclient_port:%d recv:%s recvLen:%d\n", client.sin_port, recvbuf, num);
    if (strcmp(recvbuf, "bye") == 0){
        printf("one client exit!!\n num:%d\n", client.sin_port);
        return 1;
    }

    for (int i = 0; i < strlen(recvbuf); i++)
        sendbuf[i] = recvbuf[strlen(recvbuf) - 1 - i];
    sendbuf[strlen(sendbuf)] = '\0';

    send(connfd, sendbuf, strlen(sendbuf), 0);
}

int main(int argc, char const *argv[])
{
    printf("server start!!!!\n");
    int listenfd;
    int nfds = 0;
    struct sockaddr_in server;

    int SERVERPORT = 10086;
    int val = 1;
    int backlog = 10; //请求队列
    struct sockaddr_in clients[backlog+3];
    bzero(&clients, sizeof(clients));

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) //socket fail
    {
        perror("socket");
        exit(1);
    }
    printf("listenfd=%d\n", listenfd);

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVERPORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(server.sin_zero), 8);

    //bind
    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    //listen
    if (listen(listenfd, backlog) == -1)
    {
        perror("listen");
        exit(1);
    }

    if (listenfd > nfds)
        nfds = listenfd + 1;

    fd_set static_fd_set;
    fd_set read_fd_set;
    FD_ZERO(&static_fd_set);
    FD_SET(listenfd, &static_fd_set);

    printf("nfds=%d\n", nfds);
    while (1)
    {
        read_fd_set = static_fd_set;
        int num;
        if (num = select(nfds, &read_fd_set, NULL, NULL, NULL) > 0)
        {
            printf("mun=%d ", num);
            printf("nfds=%d\n", nfds);
            for (int i = 0; i < nfds; i++)
            {
                if (FD_ISSET(i, &read_fd_set))
                {
                    if (i == listenfd)
                    {
                        //printTime
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

                        int connfd;
                        struct sockaddr_in client;
                        int len = sizeof(client);

                        //accept
                        connfd = accept(listenfd, (struct sockaddr *)&client, &len);
                        if (connfd == -1)
                        {
                            perror("accept");
                            return -1;
                        }
                        printf("一个客户端已经连接fd=%d\n", connfd);
                        FD_SET(connfd, &static_fd_set);
                        clients[connfd] = client;
                        nfds++;
                    }

                    else
                    {
                        printf("\n收到消息fd=%d", i);
                        if(processClient(i, clients[i]) == 1){
                            close(i);
                            FD_CLR(i, &static_fd_set);
                        }
                    }
                }
            }
        }
    }

    close(listenfd);
    printf("\n\n\n\n\n");
    return 0;
}
