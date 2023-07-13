// define宏定义中的 #，##，@# 及 / 符号
// 单个# ：将后续的内容字符串化。
// 双个# ：## 仅将两内容连接在一起。

// # 字符串化操作符。
// 其作用是：
// 将宏定义中的传入参数名转换成用一对双引号括起来参数名字符串。
// 其只能用于有传入参数的宏定义中，且必须置于宏定义体中的参数名前。

// ## 符号连接操作符
// 宏定义中：参数名，即为形参，如#define sum(a,b) (a+b)；中a和b均为某一参数的代表符号，即形式参数。
// 而##的作用则是将宏定义的多个形参成一个实际参数名。

// @# （charizing）字符化操作符。
// 只能用于有传入参数的宏定义中，且必须置于宏定义体中的参数名前。
// 作用是将传的单字符参数名转换成字符，以一对单引用括起来。

//  /
// 当定义的宏不能用一行表达完整时，可以用"/"表示下一行继续此宏的定义。

#include <iostream>
#include <stdio.h>

int token9 = 9;
#define paster(n) printf("token" #n " = %d", token##n)

#define STR(str) #str

#define to_str(s) #s

#define test_print(n) printf("value"#n" = %d\n ", value##n )  

// 测试不带 # 号的效果
#define NO_NUMBER_SIGN(x) x

// 测试一个 # 号的效果
#define SINGLE_NUMBER_SIGN(x) #x

// 测试两个 # 号的效果    ##前后可随意加上一些空格>
#define DOUBLE_NUMBER_SIGN(x, y) x  ##   y

// Linux下编译不过
// 连接符#@：它将单字符标记符变换为单字符，即加单引号。例如：
// #define B(x) #@x
// 则B(a)即'a'，B(1)即'1'，但B(abc)却不甚有效。
// #define ToChar(x) #@x 

// #define TESTX(x) \
//     std::cout << ##x##x #x #x << std::endl;

int main()
{
    paster(9); // printf_s("token" "9" " = %d", token9)

    std::cout << STR("str test") << std::endl; // "\"str test\""

    std::cout << to_str("abcdefg") << std::endl; // "\"abcdefg\""

    int value32 = 999;
    test_print(32); // printf("value""32"" = %d\n ", value32 )


    printf("%s\n", NO_NUMBER_SIGN("hello"));

    // 测试一个 # 号的效果： 因为 # 将其字符串化，加引号和比较引号打印有区别
    printf("%s\n", SINGLE_NUMBER_SIGN("world"));    // "\"world\""

    printf("%s\n", SINGLE_NUMBER_SIGN(world));  // "world"

    char xxyy[] = "hello world";

    // 测试两个 # 号的效果：连接两个对象
    printf("%s\n", DOUBLE_NUMBER_SIGN(xx, yy)); //  xxyy

    // char a = ToChar(1); // '1'
    // char b = ToChar(123);   // '123'

    // TESTX("abcdef");    // std::cout <<"abcdef""abcdef" "\"abcdef\"" "\"abcdef\"" << std::endl;

    return 0;
}

/*

token9 = 9"str test"        
"abcdefg"
value32 = 999
 hello
"world"
world
hello world
abcdefabcdef"abcdef""abcdef"

*/