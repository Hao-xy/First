#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<strings.h>
#include<time.h>
#include<sys/time.h>
#include<string.h>
#include<unistd.h>
void print_time_usec()
{
    struct tm* ptm;
    struct timeval time;
    gettimeofday(&time,NULL);
    ptm=localtime(&time.tv_sec);
    if(ptm!=NULL)
    {
        printf("%d-%d-%d:%d:%d:%d\n:%ld\n",ptm->tm_year+1900,ptm->tm_mon+1,
            ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,
            time.tv_usec % 1000000/1000);
    }
}

void process_client(int connfd)
{
    char recvbuf [100];
    char sendbuf [100];
    int num,i;

    
    num=recv(connfd,recvbuf,100,0);
    
    printf("recv:%s\n",recvbuf); 

    for(int i=0;i<num-1;i++){
        sendbuf[i]=recvbuf[num-i-2];
    }
    sendbuf[num-1]='\0';
    send(connfd,sendbuf,strlen(sendbuf),0);
    sprintf(sendbuf,"welcome to server!");
    close(connfd);
}

void process_accept(int listenfd)
{
    int connfd;
    struct sockaddr_in client;
    int len=sizeof(client);

    connfd=accept(listenfd,(struct sockaddr *)&client,&len);
    if(connfd==-1)
    {
        perror("accept");
        exit(1);
    }
    printf("connfd=%d\n",connfd);

    print_time_usec();

    printf("client port:%d\n", client.sin_port );
    printf("client IP :%s\n",inet_ntoa(client.sin_addr));

    process_client(connfd);
}

int main(int argc,char* argv[])
{
    int listenfd;
    struct sockaddr_in server;
    int SERVEPORT=1234,val=1;
    int backlog=5;

    listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd==-1){
        perror("socket");
        exit(1);
    }
    printf("listenfd=%d\n",listenfd );

    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(char *)&val,sizeof(val));

    server.sin_family=AF_INET;
    server.sin_port=htons(SERVEPORT);
    server.sin_addr.s_addr=inet_addr("127.0.0.1");
    bzero(&(server.sin_zero),8);

    if(bind(listenfd,(struct sockaddr *)&server,sizeof(struct sockaddr))==-1)
    {
        perror("bind");
        exit(1);
    }

    if(listen(listenfd,backlog)==-1)
    {
        perror("listen");
        exit(1);
    }

    while(1)
    {
        process_accept(listenfd);
    }

    close(listenfd);

    return 0;
}