#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
int link_serv(int port, char* ipv4);
int main(int argc, char* argv[])
{
	char buf[BUFSIZ];
	char cmd_in[BUFSIZ];
	int sock;
	sock = link_serv(atoi(argv[1]), argv[2]);
	while(1)
	{
		printf("$ ");
		fgets(cmd_in, BUFSIZ, stdin);
		cmd_in[strlen(cmd_in)-1] = '\0';
		if(strcmp("quit", cmd_in) == 0)
        {
            close(sock);
            exit(0);
        }
		write(sock, cmd_in, strlen(cmd_in));
		read(sock, buf, BUFSIZ);
        for(int i = 0; i < BUFSIZ; i++)
        {
            cmd_in[i] = 0;
            buf[i] = 0;
        }
        printf("\n");
	}
	close(sock);
	return 0;
}
int link_serv(int port, char* ipv4)
{
	struct sockaddr_in saddr;
	int sock, ret;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ipv4);
	saddr.sin_family = AF_INET;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	ret = connect(sock, (struct sockaddr*)&saddr, sizeof(saddr));
	if(ret == 0) return sock;
	else return -1;
}



