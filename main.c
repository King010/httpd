#include "http.h"


int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("Usage:%s[ip] [port]\n",argv[0]);
		return 1;
	}
	int listen_sock = startup(argv[1],atoi(argv[2]));
	

	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	while(1)
	{
		int new_sock = accept(listen_sock,(struct sockaddr*)&client,&len);
		if(new_sock < 0)
		{
			perror("accpet");
			continue;
		}
		printf("get client: ip %s, port:%d\n",\
				inet_ntoa(client.sin_addr),ntohs(client.sin_port));
		pthread_t id;
		pthread_create(&id,NULL,handler_request,(void *)new_sock);
		pthread_detach(id);
	}

	return 0;
}
