
#include "sql.h"

//脚本从子进程中读取参数
int ReadQuery_String(char buf[])
{
	char *method = NULL;
	char *arg_string = NULL;
	char *content_len = NULL;
	
	method = getenv("METHOD");
	if(method == NULL)
	{
		std::cout<<"get mothod error"<<std::endl;
		return -5;
	}
	//method 不为空，get方法参数从环境变量QUERY_STRING
	if(method && strcasecmp(method, "Get") == 0)
	{
		arg_string = getenv("QUERY_STRING");
		if(!arg_string)
		{
			return -1;
		}
		strcpy(buf,arg_string);
	}
	//post 方法，先读取content_len，再读取参数
	else if(method && strcasecmp(method,"POST") == 0)
	{
		content_len = getenv("CONTENT_LENGTH");
		if(!content_len)
		{
			return -2;
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
		return -3;
	}
	return 0;
}
void insert_data(char* buf)
{
	//处理数据
	int i = 0;
	char *start = buf;
	char* argv[4];
	while(*start)
	{
		if(*start == '=')
		{
			start++;
			argv[i++] = start;
			continue;
		}	
		if(*start == '&')
		{
			*start = '\0';
		}
			start++;
	}
	argv[i] = NULL;
	sql obj("root","","127.0.1","class",3306);
	obj.connect();
	obj.insert(argv[0],argv[1],argv[2]);
	
}
int main()
{
	char buf[1024];
    int ret =  ReadQuery_String(buf);
	std::cout<<ret<<std::endl;
	if(ret < 0)
		return 1;
	insert_data(buf);
	return 0;

}
