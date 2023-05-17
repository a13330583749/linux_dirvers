#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
int main(int argc, char ** argv){
    int fd;
    char *filename;
    int ret = 0;
    unsigned char data;
    struct pollfd fds[1];
    int times = (int)8e3;
    filename = argv[1];
    if (argc != 2){
        printf("Usage Error!\r\n");
        return -1;
    }

    fd = open(filename, O_RDWR | O_NONBLOCK);
    if(fd < 0){
        printf("can't find %s\r\n", filename);
        return -1;
    }
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    while(1){
        // ret = read(fd, &data, sizeof data);
        printf("4321\r\n");
        ret = poll(fds, 1, times);
        printf("1234\r\n");
        if( ret < 0){
            printf("Error!\r\n");
            return -1;
        }else{
            if(ret)
                printf("No news come!\r\n");
            else if (fds[0].revents & POLLIN)
            {
                read(fd, &data, sizeof data);
                printf("get data = %#X\r\n", data);
            }else{
                printf("ret = %d\r\n", ret);
            }
        }
    }
    close(fd);
    return 0;
}