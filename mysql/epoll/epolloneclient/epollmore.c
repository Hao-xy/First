#include<assert.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

 
#define BUFFERSIZE    2048          //模拟客户端数量
#define IP_ADDR     "127.0.0.1"       //默认服务器IP
#define PORT          1234             //默认服务器端口

char sendstring[20]={0}; //向服务器发送的字符串

int setnonblocking( int fd )  
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

int main(int argc, char const *argv[])
{
  int epoll_fd=epoll_create(100);
  int ret=0;
  struct sockaddr_in address;

  bzero(&address,sizeof(address));
  address.sin_family=AF_INET;
  inet_pton(AF_INET,IP_ADDR,&address.sin_addr);
  address.sin_port=htons(PORT);
  int sockfd=socket(PF_INET,SOCK_STREAM,0);
  printf("sockfd=%d\n",sockfd);
  if(sockfd<0){
    perror("socket");
    exit(1);
  }
  if (connect( sockfd,( struct sockaddr* )&address,sizeof( address ) ) == 0 ){
    struct epoll_event event;
    event.data.fd=sockfd;
    event.events=EPOLLOUT|EPOLLET|EPOLLERR;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,sockfd,&event);
    setnonblocking(sockfd);
  }    
  struct epoll_event events[1000];
  char buffer[BUFFERSIZE];
  while(1){
    int fds = epoll_wait( epoll_fd, events, 1000, 2000 );
    printf("fds=%d\n",fds);
        for ( int i = 0; i < fds; i++ )
        {   
            int tmpfd = events[i].data.fd;
            if ( events[i].events & EPOLLIN )//收到数据
            {  
              int bytes_read=0;
              bzero(buffer,sizeof(buffer));
              bytes_read=recv(tmpfd,buffer,BUFFERSIZE,0);
              if((bytes_read==-1)||(bytes_read==0)){
                epoll_ctl(epoll_fd,EPOLL_CTL_DEL,tmpfd,0);
                close(tmpfd);
              }
              printf("read in %d bytes from socket %d with content: %s\n",bytes_read,tmpfd,buffer);

              epoll_ctl(epoll_fd,EPOLL_CTL_DEL,tmpfd,0);
              close(tmpfd);
            }
            else if(events[i].events & EPOLLOUT){//数据可写
              sleep(2);
              sprintf(sendstring,"xyh314user%d/xyh314pass%d",sockfd,sockfd);
              printf("sending:*****%s***%ld\n",sendstring,strlen(sendstring));

              int len=strlen(sendstring);
              int bytes_write=0;
              char *psend=sendstring;
               printf( "write out %d bytes to socket %d\n", len, sockfd );
              while(1){
                bytes_write=send(tmpfd,psend,len,0);
                if((bytes_write==-1)||(bytes_write==0)){
                  epoll_ctl(epoll_fd,EPOLL_CTL_DEL,tmpfd,0);
                  close(tmpfd);
              }
              len-=bytes_write;
              psend=psend+bytes_write;
              if(len<=0){
                break;
              }
              }
              struct epoll_event event;
              event.events=EPOLLIN|EPOLLET|EPOLLERR;
              event.data.fd=tmpfd;
              epoll_ctl(epoll_fd,EPOLL_CTL_MOD,tmpfd,&event);

            }
            else if( events[i].events & EPOLLERR ) //错误
            {
                epoll_ctl(epoll_fd,EPOLL_CTL_DEL,tmpfd,0);
                close(tmpfd);
            }
           
  }
  
  }
  return 0;
   
}