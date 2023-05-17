#include <iostream>
#include <stdio.h>
using namespace std;
int main(){
    int i = 1;
    unsigned char ch = i;
    printf("ch = %x\t \r\n", ch);
    printf("ch = %s\r\n", &ch);

    return 0;
}