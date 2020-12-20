#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/shm.h>
#include <strings.h>
#include <arpa/inet.h>
#include "netutils.h"
int get_serv_sock(int portnum, int sock_type)
{
    int sock;
    struct hostent* hp;
    struct sockaddr_in saddr;
    char hostname[BUFSIZ];
    gethostname(hostname, BUFSIZ);
    hp = gethostbyname(hostname);
    //bcopy((void*)hp->h_addr, (void*)&saddr.sin_addr, sizeof(hp->h_addr));//这其实是错误的
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);/*same for s_addr = inet_addr("0.0.0.0");*/
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(portnum);
    sock = socket(AF_INET, sock_type, 0);
    bind(sock, (struct sockaddr*)&saddr, sizeof(saddr));
    listen(sock, 1);
    return sock;
}
