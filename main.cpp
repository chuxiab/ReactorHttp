#include <stdio.h>
#include <unistd.h>
#include "TcpServer.h"

int main(int argc, char* argv[])
{
#if 0
	if (argc < 3)
	{
		printf("./xxx port path\n");
		exit(0);
	}
	unsigned short port = atoi(argv[1]);
	chdir(argv[2]);

#else
	unsigned short port = 8000;
	chdir("/home/bai/webserver/web-http");
#endif
	
	TcpServer* tcpServer = new TcpServer(port, 5);
	printf("hello\n");
	tcpServer->run();

	return 0;
}