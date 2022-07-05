clean:epollmoreclient libevent_mysql
	
epollmoreclient:epollmoreclient.c
	gcc $^ -o $@
libevent_mysql:libevent_mysql.c
	gcc $^ -o $@  -levent  -lmysqlclient

