// VA_LIST的用法：
// （1）首先在函数里定义一具 VA_LIST 型的变量，这个变量是指向参数的指针；
// （2）然后用 VA_START 宏初始化变量刚定义的 VA_LIST 变量；
// （3）然后用 VA_ARG 返回可变的参数，VA_ARG的第二个参数是你要返回的参数的类型（如果函数有多个可变参数的，依次调用 VA_ARG 获取各个参数）；
// （4）最后用 VA_END 宏结束可变参数的获取。

// va_list ap; 定义一个va_list变量ap
// va_start(ap,v)；执行ap = (va_list)&v + _INTSIZEOF(v)，ap指向参数v之后的那个参数的地址，即ap指向第一个可变参数在堆栈的地址。
// va_arg(ap,t) ， ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )取出当前ap指针所指的值，并使ap指向下一个参数。

// ap＋= sizeof(t类型)，让ap指向下一个参数的地址。然后返回ap-sizeof(t类型)的t类型指针，这正是第一个可变参数在堆栈里的地址。然后
// 用取得这个地址的内容。

// va_end(ap); 清空va_list ap。

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

void fun(int a, ...)
{
    va_list pp;
    int n = 1;
    va_start(pp, a);
    do
    {
        printf("第 %d 个参数 =%d\n", n++, a);
        a = va_arg(pp, int);
    } while (a != 0);
    va_end(pp);
}

#include <string>
#include <stdio.h>
int main()
{
    char *pCh = nullptr;
    int sz = 27755 * 13582;
    pCh = (char *)malloc(sz);
    auto szA = strlen(pCh);
    memset(pCh, 3, sz);
    auto szB = strlen(pCh);

    // pCh += sz;
    pCh[sz - 2] = 'a';
    pCh[sz - 1] = 'b';

    for (int i = 0; i < sz; i++)
    {
        pCh = (char *)i;
        pCh++;
    }
    // pCh = 'a';
    auto szC = strlen(pCh);

    int result = sum(3, 1, 2, 3);
    printf("the result is %d\n", result);

    fun(1, 3, 24, -2, 32, 66, 0);
    return 0;
}