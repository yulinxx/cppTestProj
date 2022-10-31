#include <stdarg.h>
#include <stdio.h>

int sum(int cnt, ...)
{
    int sum = 0;
    va_list ap;
    va_start(ap, cnt);
    for (int i = 0; i < cnt; ++i)
    {
        sum += va_arg(ap, int);
    }
    va_end(ap);
    return sum;
}

int main()
{
    int result = sum(3, 1, 2, 3);
    printf("the result is %d\n", result);
    return 0;
}