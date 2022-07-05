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
#include <sys/stat.h>
#include <fcntl.h>
#include <event2/event.h>
#include <mysql/mysql.h>

#define BUFFER_SIZE 5005
#define BACKLOG (10) //请求队列

char buffer[BUFFER_SIZE] = {0};
struct sockaddr_in clients[(BACKLOG)*10];
MYSQL *con;

void read_callback(evutil_socket_t fd, short ev, void *user_arg);
void finish_with_error(MYSQL *con);
void split(char *src, const char *separator, char **dest, int *num) {
    char *pNext;
    int count = 0;
    if (src == NULL || strlen(src) == 0) //如果传入的地址为空或长度为0，直接终止
        return;
    if (separator == NULL || strlen(separator) == 0) //如未指定分割的字符串，直接终止
        return;
    pNext = (char *)strtok(src, separator); //必须使用(char *)进行强制类型转换(虽然不写有的编译器中不会出现指针错误)
    while (pNext != NULL) {
        *dest++ = pNext;
        ++count;
        pNext = (char *)strtok(NULL, separator);  //必须使用(char *)进行强制类型转换
    }
    *num = count;
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
    printf("accept success\n");
    printf("客户端fd=%d已经连接\n", connfd);
    
    struct event* read_event = event_new(user_arg, connfd, EV_READ | EV_PERSIST, read_callback, (void*)user_arg);
    event_add(read_event, NULL);
    
    clients[connfd] = client;

    con = mysql_init(NULL);
    if(con == NULL){
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }
    //连接
    if(mysql_real_connect(con, "localhost", "root", "11","xyhdb", 0, NULL, 0) == NULL){
        finish_with_error(con);
    } 
    if (mysql_query(con, "DROP TABLE IF EXISTS xyhtable")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "CREATE TABLE xyhtable(xyhid INT PRIMARY KEY AUTO_INCREMENT, xyhuser VARCHAR(255),xyhpass VARCHAR(255),xyhname VARCHAR(255))")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(1,'xyhuser001','xyhpass001','xyhname001')")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(2,'xyhuser002','xyhpass002','xyhname002')")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(3,'xyhuser003','xyhpass003','xyhname003')")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(4,'xyhuser004','xyhpass004','xyhname004')")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(5,'xyhuser005','xyhpass005','xyhname005')")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(6,'xyhuser006','xyhpass006','xyhname006')")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(7,'xyhuser007','xyhpass007','xyhname007')")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(8,'xyhuser008','xyhpass008','xyhname008')")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(9,'xyhuser009','xyhpass009','xyhname009')")) {
        finish_with_error(con);
    } 
    if (mysql_query(con, "INSERT INTO xyhtable VALUES(10,'xyhuser010','xyhpass010','xyhname010')")) {
        finish_with_error(con);
    } 
   

}

void finish_with_error(MYSQL *con)
{
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
} 
void read_callback(evutil_socket_t fd, short ev, void *user_arg)
{
    int nbytes;
    char sendbuf[BUFFER_SIZE] = {0};
    char * i_query = {0};
    char dest[100] = {""};
    //接收 client
    if ((nbytes = recv(fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        fprintf(stdout, "fd:%d <ip,port><%s : %d> read:%d(bytes) content:%s\n", fd,inet_ntoa(clients[fd].sin_addr),clients[fd].sin_port, nbytes, buffer);
        
    }
    if (strncmp(buffer, "bye", 3) == 0 || nbytes == 0)
    {
        printf("服务器与<ip,port>为<%s : %d> 且fd为%d的客户端断开连接\n", inet_ntoa(clients[fd].sin_addr),clients[fd].sin_port,fd);
        event_del(event_base_get_running_event((struct event_base *)user_arg));
        struct event* this_event=event_base_get_running_event((struct event_base*)user_arg);
        event_free(this_event);
        close(fd);
        return;
    }
     
    char *revbuf[BUFFER_SIZE] = { 0 }; //存放分割后的子字符串 ，数组元素要设置的比分割后的子字符串个数大
    int num = 0;//分割后子字符串的个数
    split(buffer, "/", revbuf, &num); //调用函数进行分割 
    i_query = "SELECT xyhpass FROM xyhtable where xyhuser='";
    strcat(dest,i_query);
    strcat(dest,revbuf[0]);
    strcat(dest,"'");
    //查询
    if(mysql_query(con, dest))
    {
        finish_with_error(con);
    } 
    //结果集
    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL)
    {
        finish_with_error(con);
    } 
    //判断操作匹配
    MYSQL_ROW row;
    unsigned int num_fields;
    num_fields = mysql_num_fields(result);
    int flag=0;
    printf("%s %s\n",revbuf[0],revbuf[1]);
    while ((row = mysql_fetch_row(result)))
    {
        for (int i = 0; i < num_fields; ++i)
        {
            int t=strncmp(row[0],revbuf[1],strlen(row[0]));
            if(t==0){
                flag=1;
            }else{
                flag=2;
            }
            
        }
       
    }
    if (flag==1)
    {
       sendbuf[0]='1';
    }else if(row<=0){
       sendbuf[0]='2';
    }else if (flag==2)
    {
        sendbuf[0]='3';
    }
    
    sendbuf[strlen(sendbuf)] = '\0';
    send(fd, sendbuf, strlen(sendbuf), 0);
    printf("send\n");

    bzero(buffer, sizeof(buffer));
    bzero(revbuf[0], strlen(revbuf[0]));
    bzero(revbuf[1], strlen(revbuf[1]));
    //释放关闭
    mysql_free_result(result);
    // mysql_close(con);
}

int main(int argc, char const *argv[])
{
    printf("server start!\n");
    int listenfd;
    int nfds = 0;
    struct sockaddr_in server;

    int SERVERPORT = 10086;
    int val = 1;
    bzero(&clients, sizeof(clients));
    printf("socket create\n");  
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) //socket fail
    {
        perror("socket");
        exit(1);
    }
    printf("socket success\n");  
    printf("listenfd=%d\n", listenfd);
    printf("ip configure\n");
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
    printf("bind success\n");
    //listen
    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }
    printf("listen start\n");
    struct event_base* base = event_base_new();//新建事件
    struct event* read_event = event_new(base, listenfd, EV_READ | EV_PERSIST, accept_client, (void*)base);//监听事件创建
    event_add(read_event, NULL);//事件加入
    event_base_dispatch(base);//阻塞

    event_base_free(base);

    close(listenfd);
    printf("\n");
    return 0;
}