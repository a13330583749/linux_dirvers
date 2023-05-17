#include <iostream>
#include <signal.h>
#include <stdlib.h>

void sign_handler(int num)
{
    std::cout << "\r\nSIGINT signal\r\n";
    exit(0);
}

int main(){
    signal(SIGINT, sign_handler);
    while (1)
    {
        /* code */
    }
    std::cout << "endl\n";
    return 0;
}