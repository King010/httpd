#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    //脚本从子进程中读取参数
	char *method = NULL;
	char *arg_string = NULL;
	char *content_len = NULL;	
	char buf[1024];//获得参数
	method = getenv("METHOD");
	//method 不为空，get方法参数从环境变量QUERY_STRING
	if(method && strcasecmp(method, "Get") == 0)
	{
		arg_string = getenv("QUERY_STRING");
		if(!arg_string)
		{
			printf("get method GET arg error\n");
			return 1;
		}
		strcpy(buf,arg_string);
	}
	//post 方法，先读取content_len，再读取参数
	else if(method && strcasecmp(method,"POST") == 0)
	{
		content_len = getenv("CONTENT_LENGTH");
		if(!content_len)
		{
			printf("get POST Content_Length error\n");
			return 2;
		}
		int nums = atoi(content_len);
		int i=0;
		char c='\0';
		for(;i < nums;++i)
		{
			//读取参数
			read(0,&c,1);//子进程的input[0]重定向到0
			buf[i] = c;
		}
		buf[i] = '\0';
	}
	else
	{
		printf("get method error\n");
		return 3;
	}
	//进行处理参数
	
	return 0;
}
