#include<stdio.h>
#include <mysql/mysql.h>
int main(int argc, char const *argv[])
{
    printf("Mysql client version：%s\n",mysql_get_client_info());
    return 0;
}
