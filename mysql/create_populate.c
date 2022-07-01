#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>

void finish_with_error(MYSQL *con)
{
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}

int main(int argc, char **argv)
{
    MYSQL *con = mysql_init(NULL);
    if (con == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }
    if (mysql_real_connect(con, "localhost", "root", "11", "zsdb1", 0, NULL, CLIENT_MULTI_STATEMENTS) == NULL)
    {
        finish_with_error(con);
    }

    if (mysql_query(con, "DROP TABLE IF EXISTS cars"))
    {
        finish_with_error(con);
    }
    //创建数据表cars
    if (mysql_query(con, "CREATE TABLE cars(id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(255), price INT)"))
    {
        finish_with_error(con);
    }
    //插入数据
    if (mysql_query(con, "INSERT INTO cars VALUES(1,'Audi314xyh',52642);\
                        INSERT INTO cars VALUES(2,'Mercedes314xyh',57127);\
                        INSERT INTO cars VALUES(3,'Skoda314xyh',9000);\
                        INSERT INTO cars VALUES(4,'Volvo314xyh',29000);\
                        INSERT INTO cars VALUES(5,'Bentley314xyh',350000);\
                        INSERT INTO cars VALUES(6,'Citroen314xyh',21000);\
                        INSERT INTO cars VALUES(7,'Hummer314xyh',41400);\
                        INSERT INTO cars VALUES(8,'Volkswagen314xyh',21600);"))
    {
        finish_with_error(con);
    }
   
    exit(0);
}