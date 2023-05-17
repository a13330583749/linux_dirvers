#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#define Port 8879
#define fd_maxsize 100

int main(void)
{
    char buf[1024];
    fd_set rfds;
    fd_set allset; // all of fd in this set
    struct timeval tv;
    int retval, client[fd_maxsize];

    // creat sockets
    struct sockaddr_in cliaddr, servaddr;
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof servaddr);
    memset(client, -1, fd_maxsize * sizeof(int));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(Port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind
    bind(socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(socketfd, 10);

    int maxfd = socketfd; // 记录fd_socket的最大是
    FD_ZERO(&allset);
    FD_SET(socketfd, &allset);

    int nReady, connfd(0), i(0), sockfd, n;
    while (1)
    {
        nReady = select(maxfd + 1, &allset, NULL, NULL, NULL);
        if (FD_ISSET(socketfd, &allset))
        {
            socklen_t clienlen = sizeof(cliaddr);
            connfd = accept(socketfd, (struct sockaddr *)&servaddr, &clienlen);
            printf("a new connecting\n");
            for (i = 0; i <= fd_maxsize; i++)
            {
                if (client[i] == -1)
                {
                    client[i] = connfd;
                    break;
                }
            }
            if (i == fd_maxsize)
                exit(0);
            FD_SET(connfd, &allset);
            maxfd = connfd > maxfd ? connfd : maxfd;
            --nReady;
        }
    }

    for (int j = 0; j < maxfd; j++)
    {
        if ((sockfd = client[j]) < -1)
            continue;
        if (FD_ISSET(sockfd, &allset))
        {
            if (n = read(sockfd, buf, 1024) == 0)
            {
                close(sockfd);
                printf("closed\n");
                client[j] = -1;
                FD_CLR(sockfd, &allset);
            }
            else{
                write(sockfd, buf, n);
                printf("%s\n", buf);
            }
            if(--nReady)
                break;
        }
    }
}

