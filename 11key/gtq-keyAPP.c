#include <stdio.h>
#include <unistd.h>
#include "stdlib.h"
#include "fcntl.h"
int main(int argc, char *argv[]){
    int fd, ret;
    char* filename;
    unsigned char keyvalue;

    if(argc != 2){
        printf("Usage Error!\r\n");
        return -1;
    }
    filename = argv[1];

    fd = open(filename, O_RDWR);

    while (1)
    {
        read(fd, &keyvalue, sizeof(keyvalue));
        if (keyvalue == 0x00) {
            printf("KEY Press, value = %#X\r\n", keyvalue);
        }
    }
    ret = close(fd);
    if(ret < 0){
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}