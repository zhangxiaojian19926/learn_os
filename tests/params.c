#include <stdio.h>


int add(int a, int b)
{
    int z = a + b;
    return z;
}

int main()
{
    int a = 5;
    int b = 3;
    int c = add(a, b);
    return 0;
}