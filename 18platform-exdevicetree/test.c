#include <stdio.h>
struct A
{
    int a;
    long b;
    char c;
};


int main()
{
    struct A gg = {
        .a = 1,
        .b = 123,
        .c = 'w',
    };
    return 0;
}