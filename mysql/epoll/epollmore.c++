// 这是可以模拟多个客户端连接的客户端程序
// 运行时，默认连接 127.0.0.1（8001），也可以通过命令行参数指定其他端口，目前不能从命令行指定ip
// g++  clientepoll.c  -o  clientepoll 

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
 
#define CONNECT_NUM  1000              //模拟客户端数量
#define IP_ADDR       "127.0.0.1"       //默认服务器IP
#define PORT          1234              //默认服务器端口
 

char sendstring[20]={0}; //向服务器发送的字符串
 
 
int setnonblocking( int fd )  //设置非阻塞，参考https://www.cnblogs.com/charlesblc/p/6202402.html
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}
 
void addfd( int epoll_fd, int fd )  //向epoll_fd注册文件描述符
{
    epoll_event  event;
    event.data.fd = fd;
    event.events = EPOLLOUT | EPOLLET | EPOLLERR;
    epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}
 
bool write_nbytes( int sockfd, const char* buffer, int len )  //向指定文件描述符发送字符串
{
    int bytes_write = 0;
    printf( "write out %d bytes to socket %d\n", len, sockfd );
    while( 1 ) 
    {   
        bytes_write = send( sockfd, buffer, len, 0 );
        if ( bytes_write == -1 )
        {   
            return false;
        }   
        else if ( bytes_write == 0 ) 
        {   
            return false;
        }   
 
        len -= bytes_write;
        buffer = buffer + bytes_write;
        if ( len <= 0 ) 
        {   
            return true;
        }   
    }   
}
 
bool read_once( int sockfd, char* buffer, int len )  //读一次
{
    int bytes_read = 0;
    memset( buffer, '\0', len );
    bytes_read = recv( sockfd, buffer, len, 0 );
    if ( bytes_read == -1 )
    {
        return false;
    }
    else if ( bytes_read == 0 )
    {
        return false;
    }
	printf( "read in %d bytes from socket %d with content: %s\n", bytes_read, sockfd, buffer );
 
    return true;
}
 
void start_conn( int epoll_fd, int num, const char* ip, int port )  //向服务器发起连接
{
    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );
 
    for ( int i = 0; i < num; ++i ) //建立num个套接字，并向服务器发起连接请求
    {
        //sleep( 1 );
        int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
        printf( "create 1 sock\n" );
        if( sockfd < 0 )
        {
            continue;
        }
 
        if (  connect( sockfd, ( struct sockaddr* )&address, sizeof( address ) ) == 0  )
        {
            printf( "build connection %d\n", i );
            addfd( epoll_fd, sockfd );  //将连接成功的描述符注册到epoll_fd中
        }
    }
}
 
void close_conn( int epoll_fd, int sockfd )
{
    epoll_ctl( epoll_fd, EPOLL_CTL_DEL, sockfd, 0 );
    close( sockfd );
}
 /*
 关于epoll的注释来自于以下网址
  https://blog.csdn.net/wlf_go/article/details/81736933
 
 */
int main( int argc, char* argv[] )
{
	//man  epoll可查看更多信息
 	//生成一个epoll专用的文件描述符。它其实是在内核申请一空间，用来存放你想关注的socket fd上是否发生以及发生了什么事件。size就是你在这个epoll fd上能关注的最大socket fd数。
    int epoll_fd = epoll_create( 100 );
	//edit server connect number ,ip and port
    
    int port;
	if(argc==2)
		port=atoi(argv[1]);
	else
		port=PORT;
    start_conn( epoll_fd,CONNECT_NUM,IP_ADDR, port );//向服务器发起CONNECT_NUM个连接
    epoll_event events[ 1000 ];
    char buffer[ 2048 ];
    while ( 1 )
   	 {

   	 	/*
   	 	函数声明:int epoll_wait(int epfd,struct epoll_event * events,int maxevents,int timeout)
		该函数用于轮询I/O事件的发生；
		参数：
   	 	epfd:由epoll_create 生成的epoll专用的文件描述符；
		epoll_event:用于回传代处理事件的数组；
		maxevents:每次能处理的事件数；
		timeout:等待I/O事件发生的超时值(单位我也不太清楚)；-1相当于阻塞，0相当于非阻塞。一般用-1即可，
		返回值为发生事件数。
   	 	
   	 	epoll_wait运行的原理是：
   	 	等侍注册在epfd上的socket fd的事件的发生，如果发生则将发生的sokct fd和事件类型放入到events数组中。
		并且将注册在epfd上的socket fd的事件类型给清空，
		所以如果下一个循环还要关注这个socket fd的话，则需要用epoll_ctl(epfd,EPOLL_CTL_MOD,listenfd,&ev)来重新设置socket fd的事件类型。
		这时不用EPOLL_CTL_ADD,因为socket fd并未清空，只是事件类型清空。这一步非常重要。
   	 	
   	 	*/
   	 	
   	 	////等待EPOLL事件的发生，相当于监听，至于相关的端口，需要在初始化EPOLL的时候绑定。
        int fds = epoll_wait( epoll_fd, events, 1000, 2000 );//events用于回传代处理事件的数组
        for ( int i = 0; i < fds; i++ ) //根据epoll_wait的返回值确定循环次数
        {   
            int sockfd = events[i].data.fd;
            if ( events[i].events & EPOLLIN )//如果是已经连接的用户，并且收到数据，那么进行读入
            {   
                if ( ! read_once( sockfd, buffer, 2048 ) )
                {
                    close_conn( epoll_fd, sockfd );//处理读取失败
                }
                /*
                常用的事件类型:
				EPOLLIN ：表示对应的文件描述符可以读；
				EPOLLOUT：表示对应的文件描述符可以写；
				EPOLLPRI：表示对应的文件描述符有紧急的数据可读
				EPOLLERR：表示对应的文件描述符发生错误；
				EPOLLHUP：表示对应的文件描述符被挂断；
				EPOLLET：表示对应的文件描述符有事件发生；
                */
                struct epoll_event event;
                event.events = EPOLLOUT | EPOLLET | EPOLLERR;//设置要处理的事件类型
                event.data.fd = sockfd;////设置与要处理的事件相关的文件描述符
                epoll_ctl( epoll_fd, EPOLL_CTL_MOD, sockfd, &event );
                //控制某个epoll文件描述符上的事件，可以注册事件EPOLL_CTL_ADD，修改事件EPOLL_CTL_MOD，删除事件EPOLL_CTL_DEL

            }
            else if( events[i].events & EPOLLOUT ) //有数据可写
           	{
            	sleep(2);
            	sprintf(sendstring,"user%d/pass%d",sockfd,sockfd);
            	//printf("*****%s***%ds\n",sendstring,strlen(sendstring));

                if ( ! write_nbytes( sockfd, sendstring, strlen( sendstring ) ) ) //发送数据给服务器
                {
                    close_conn( epoll_fd, sockfd );
                }
                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET | EPOLLERR;
                event.data.fd = sockfd;
                epoll_ctl( epoll_fd, EPOLL_CTL_MOD, sockfd, &event );
            }
            else if( events[i].events & EPOLLERR ) //错误
            {
                close_conn( epoll_fd, sockfd );
            }
        }
    }
}
 

