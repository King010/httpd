#include "http.h"

int startup(char *ip, int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		perror("socket");
		return -1;
	}
	int opt =1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);
	if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
	{
		perror("bind");
		return -2;
	}
	if(listen(sock,10)<0)
	{
		perror("listen");
		return -3;
	}
	return sock;
}
//读取一行报文，遇到\r\n 或\n 都转换为\n 结束
int get_line(int fd,char*buf,int len)
{
	int c = '\0';
	int i=0;
	while(c != '\n' && i < len-1)
	{
		ssize_t s = recv(fd,&c,1,0);
		if(s > 0)
		{
			//将\r\n 或\n 都转换为\n
			if(c == '\r')
			{
				s = recv(fd,&c,1,MSG_PEEK);// 窥探，判断是否为\r\n
				if(s > 0 && c == '\n')
					recv(fd,&c,1,0);
				else
					c = '\n';
			}
			buf[i++] = c;
		}
		else//读取失败，退出循环
		{
			c = '\n';
		}
	}

	buf[i] = '\0';

	return i;
}
void print_log(const char* msg,int level)
{
	const char *lev_msg[] = {"NOTICE", "WARNNING","FATAL",};
	printf("[%s] [%s]\n",msg,lev_msg[level]);
}
static void show_404(int fd)
{
	const char *head_line = "HTTP/1.0 404 Not Foundi\r\n";
	send(fd,head_line,strlen(head_line),0);
	const char *blank_line = "\r\n";
	send(fd,blank_line,strlen(head_line),0);
	const char* msg = "<html><h1>Page Not Found!</h1></html>\r\n";
	send(fd,msg,strlen(msg),0);
}

void echo_error(int fd,int error_num)
{
	switch(error_num)
	{
		case 200:
			break;
		case 400:
			break;
		case 401:
			break;
		case 404:
			show_404(fd);
			break;
		case 501:
			break;
		default:
			break;
	}
}
//发送静态文本给服务器
int echo_www(int fd,const char *path,int size)
{
	int new_fd = open(path,O_RDONLY);
	if(new_fd < 0)
	{
		print_log("open file fail!",FATAL);
		return 404;
	}
	//先写头部信息
	const char* head = "HTTP/1.0 200 OK\r\n";
	send(fd,head,strlen(head),0);
	const char* blank_line = "\r\n";
	send(fd,blank_line,strlen(blank_line),0);
	if(sendfile(fd,new_fd,NULL,size) < 0)
	{
		print_log("send file fail!",FATAL);
		return 400;
	}
	close(new_fd);
	return 200;
}

void drop_head(int fd)
{
	char buf[SIZE];
	int ret =-1;
	do{
		ret =get_line(fd,buf,sizeof(buf));
	}while(ret > 0 && strcmp(buf,"\n"));
}
void* handler_request(void *arg)
{
	int fd = (int)arg;
	int cgi = 0;//设置cgi标志，1表示执行cgi脚本程序
	int error_num = 200;//状态码
	char *query_string = NULL;//方法带的参数
#ifdef _DEBUG_
	printf("debug\n");
	printf("########################################\n");
	char buf[SIZE];
	int ret =-1;
	do{
		ret =get_line(fd,buf,sizeof(buf));
		printf("%s",buf);
	}while(ret >0 && strcmp(buf,"\n"));
	
	printf("########################################\n");

#else
	printf("release\n");
	// 获取method,url,
	char buff[SIZE];
	char method[SIZE/10];
	char url[SIZE];//请求资源
	char path[SIZE];//路径,url的一部分
	int i,j;
	struct stat st;//stat函数参数
	if(get_line(fd,buff,sizeof(buff)) <= 0)
	{
		print_log("get line fail",FATAL);
		error_num = 501;
	
		goto end;
	}
	//method 获取(Get Post)
	i=0,j=0;
	while(!isspace(buff[j]) && i < sizeof(method)-1)
	{
		method[i++] = buff[j++];
	}
	method[i] = '\0';
	printf("method:%s\n",method);
	//跳过method后面的空格
	while(isspace(buff[j]) && j < sizeof(buff))
		j++;

	i=0;
	//获取url
	while(!isspace(buff[j]) && i < sizeof(url)-1 && j < sizeof(buff))
	{
		url[i++] = buff[j++];
	}
	url[i] = '\0';
	printf("url:%s\n",url);

	//解析完成 得到method和url
	//判断method:GET,POST(只实现了这两个)
	//strcasecmp 忽略大小的比较，成功返回 0
	if(strcasecmp(method,"get") && strcasecmp(method,"post"))
	{
		error_num = 401;
		print_log("method is not ok!",FATAL);
		goto end;
	}
	//post方法需要执行cgi脚本，get方法带参数的话，需要执行cgi
	if(strcasecmp(method,"post") == 0)
		cgi = 1;
	//get 方法，在url中找到参数
	if(strcasecmp(method, "get") == 0)
	{
		query_string = url;
		while(*query_string != '?'&& *query_string != '\0')
		{
			query_string++;
		}
		if(*query_string == '?')
		{
			cgi = 1;
			*query_string = '\0';
			query_string++;
		}
	}

	//将此时的url留下就是一个路径，写到path
	sprintf(path,"wwwroot%s",url);//在此项目下根目录为wwwroot
	//如果path是一个目录，默认首页为index.html
	if(path[strlen(path)-1] == '/')
		strcat(path,"index.html");

	printf("path:%s\n",path);
	//判断资源是否存在，stat函数判断
	if(stat(path,&st) ==-1)
	{
		print_log("path not found!",FATAL);
		error_num = 404;
		goto end;
	}
	else//资源存在
	{
		//注意：这也可能是一个目录，末尾没有带/
		if(S_ISDIR(st.st_mode))
			strcat(path,"/index.html");
	
		//判断资源是否为可执行程序
		if((st.st_mode & S_IXUSR) ||
		   (st.st_mode & S_IXGRP) ||
		   (st.st_mode & S_IXOTH))
			cgi = 1;
		
		if(cgi)
		{
			execute_cgi(fd,method,path,query_string);
		}
		else//返回静态文本
		{
			drop_head(fd);
			error_num = echo_www(fd,path,st.st_size);
		}
			
	}

#endif

end:
	echo_error(fd,error_num);
	close(fd);
}
//执行cgi脚本
int execute_cgi(int fd,const char *method,\
		const char *path,const char *query_string)
{
	int content_len = -1;
	char METHOD[SIZE/10];
	char QUERY_STRING[SIZE];
	char CONTENT_LENGTH[SIZE];
	if(strcasecmp(method,"GET") == 0)
	{
		drop_head(fd);
	}
	else
	{
		//post 参数在请求正文，大小content-Lenght: 
		char buf[SIZE];
		int ret = -1;
		do{
			ret = get_line(fd,buf,sizeof(buf));
			if(strncasecmp(buf,"Content-Length: ",16) == 0)
			{
				content_len = atoi(&buf[16]);
			}
		}while(ret > 0 && strcmp(buf,"\n"));
		if(content_len <= 0)
		{
			echo_error(fd,401);
			return -1;
		}
	}
	printf("cgi path:%s\n",path);
	//2个匿名管道连接父子进程进行通信，in out是针对脚本而言的
	int output[2];
	int input[2];
	if(pipe(input) < 0)
	{
		echo_error(fd,401);
		return -2;
	}
	if(pipe(output) <0)
	{
		echo_error(fd,401);
		return -3;
	}

	//传送响应头部
	const char *head_line = "HTTP/1.0 200 OK\r\n";
	send(fd,head_line,strlen(head_line),0);
	const char *blank_line = "\r\n";
	send(fd,blank_line,strlen(blank_line),0);

	//创建子进程
	pid_t pid = fork();
	if(pid < 0)
	{
		echo_error(fd,501);
		return -4;
	}
	else if(pid == 0)//child
	{
		//子进程进行执行脚本，
		close(input[1]);//关闭cgi输入端的写
		close(output[0]);//关闭cgi输入端的读
		sprintf(METHOD,"METHOD=%s",method);
		putenv(METHOD);
		if(strcasecmp(method,"GET") == 0)
		{
			sprintf(QUERY_STRING,"QUERY_STRING=%s",query_string);
			putenv(QUERY_STRING);
		}
		else
		{
			sprintf(CONTENT_LENGTH,"CONTENT_LENGTH=%s",content_len);
			putenv(CONTENT_LENGTH);
		}
		//重定向，
		dup2(input[0],0);
		dup2(output[1],1);

		execl(path,path,NULL);
		exit(1);
	}
	else//father
	{
		//父进程与服务器端交互，相当于是子进程与服务器的中转站
		//父进程向子进程输入input[1](cgi的读取)，和读取output[0](cgi的输出)
		close(input[0]);
		close(output[1]);
		int i=0;
		char c = '\0';
		
		//读取参数，传递给子进程(cgi)
		for(;i < content_len;++i)
		{
			recv(fd,&c,1,0);
			write(input[1],&c,1);
		}
		//读取信息，从子进程(cgi)

		while(1)
		{
			ssize_t s = read(output[0],&c,1);
			if(s > 0)
				send(fd,&c,1,0);
			else
				break;
		}
		
		waitpid(pid,NULL,0);//阻塞式等待，每个线程执行一个
		close(input[1]);
		close(output[0]);		
	}
}
