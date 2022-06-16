#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    printf("client start!!!!\n");
    int sockfd;
    struct sockaddr_in server_addr;

    //socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) //socket fail
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(10086);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(server_addr.sin_zero), 8);
    //connect
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        exit(1);
    }

    int recvbytes;
    char buf[5005] = {0};
    char sendbuf[5005] = {0};

    while (1)
    {
        printf("input a string:");
        fgets(sendbuf, sizeof(sendbuf), stdin);
        sendbuf[strlen(sendbuf)-1] = '\0';
        send(sockfd, sendbuf, strlen(sendbuf), 0);
        if (strcmp(sendbuf, "bye") == 0)
        {
            break;
        }

        int num;
        num = recv(sockfd, buf, 5005, 0);
        printf("recv:%s recvLen:%d\n", buf, num);
        bzero(sendbuf, strlen(sendbuf));
        bzero(buf, strlen(buf));
    }

    close(sockfd);
    printf("\n\n\n\n\n");
    return 0;
}
