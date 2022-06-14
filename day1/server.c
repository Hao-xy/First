#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

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

void processClient(int connfd)
{
    char recvbuf[8192] = {0};
    char sendbuf[8192] = {0};
    int num;
    while (1)
    {
        num = recv(connfd, recvbuf, 8192, 0);
        recvbuf[num] = '\0';

        printf("recv:%s \nrecvLen:%d\n", recvbuf, num);
        if (strcmp(recvbuf, "bye") == 0)
        {
            break;
        }
  
        fgets(sendbuf, sizeof(sendbuf), stdin);
        sendbuf[strlen(sendbuf)-1] = '\0';
        send(connfd, sendbuf, strlen(sendbuf), 0);
    }

    close(connfd);
    printf("one client exit!!\n\n");
}

int processAccept(int lisentfd)
{
    int connfd;
    struct sockaddr_in client;
    int len = sizeof(client);

    //accept
    printf("accept zuse ing...\n");
    connfd = accept(lisentfd, (struct sockaddr *)&client, &len);
    if (connfd == -1)
    {
        perror("accept");
        return -1;
    }

    printTimeUsec();
    processClient(connfd);
    printf("connfd=%d\n port:%d\n addr:%s\n\n", connfd,
           client.sin_port,
           inet_ntoa(client.sin_addr));
}

int main(int argc, char const *argv[])
{
    printf("server start!!!!-\n");
    int listenfd;
    struct sockaddr_in server;
    int SERVERPORT = 10086;
    int val = 1;
    int backlog = 10; //qing qiou dui lie

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
    while (1)
    {
        printf("waiting fo Clienter");
        processAccept(listenfd);
    }

    close(listenfd);
    printf("\n\n\n\n\n\n");
    return 0;
}
