#ifndef _HTTP_H_
#define _HTTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/sendfile.h>

#define SIZE 1024
#define NOTICE 0
#define WARNING 1
#define FATAL 2

int startup(char *ip, int port);
void* handler_request(void *arg);
int get_line(int fd,char*buf,int len);
void print_log(const char* msg,int level);
void drop_head(int fd);
void echo_error(int fd,int error_num);
int echo_www(int fd,const char* path,int size);
int execute_cgi(int fd, const char *method,\
		const char *path, const char* query_string);
#endif


