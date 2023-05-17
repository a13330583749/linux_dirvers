#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
int main(int argc, char ** argv){
    int fd;
    char *filename;
    int ret = 0;
    unsigned char data;
    filename = argv[1];
    if (argc != 2){
        printf("Usage Error!\r\n");
        return -1;
    }

    fd = open(filename, O_RDWR);
    if(fd < 0){
        printf("can't find %s\r\n", filename);
        return -1;
    }

    while(1){
        ret = read(fd, &data, sizeof data);
        if( ret < 0){
        
        }else{
            if(data){
                // printf("key value = %#X\r\n", data);
            }
        }
    }
    close(fd);
    return 0;
}