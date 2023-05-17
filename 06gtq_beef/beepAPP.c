#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char ** argv){
    int fd, ret;
    char *filename;
    int date;

    if(argc != 3){
        printf("Eroor Usage!\r\n");
        return -1;
    }
    filename = argv[1];

    fd = open(filename, O_RDWR);
    if(fd < 0){
        printf("Fail Open!\r\n");
        return -1;
    }else{
        printf("Open success, fd = %d\r\n", fd);
    }
    
    date = atoi(argv[2]);
    printf("date = %d\r\n", date);
    ret = write(fd, date, sizeof(date));
    if(ret < 0){
        printf("BEEP Control Failed!\r\n");
        close(fd);
        return -1;
    }
    ret = close(fd);
    if(ret < 0){
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}