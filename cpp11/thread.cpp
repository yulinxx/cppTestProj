// https://blog.csdn.net/weixin_42193704/article/details/113920419

#include <iostream>
#include <thread>
using namespace std;

void myThread() //定义线程入口函数
{
    std::cout << "test test" << endl;
}

void myThread1(int t) //带参数传递的方式
{
    for (int i = 0; i < 100; i++)
        std::cout << i << " test: " << t << endl;
}

class MyThread
{
public:
    void operator()()
    {
        std::cout << "thread start " << endl;
    }

    void myThread_in()
    {
        std::cout<<"thread start "<<endl;
    }
};

// C++11线程创建的方式

int main()
{
    // 1.函数传递方式
    std::thread thread_test(myThread); //定义 线程变量 定义的时候线程就会开始执行
    thread_test.join();                //等待线程结束
    std::thread thread_test1(myThread1, 2);
    thread_test1.join();

    // 2.lambda表达式
    std::thread threadLambda([]()
                             {
                            for(int i = 0;i<10;i++)
                                std::cout<<i<<endl; });
    threadLambda.join();

    // 3.重载运算符
    MyThread threadOperator;
    std::thread threadOpe(threadOperator);
    threadOpe.join();

    // 4.成员函数做为线程入口
    MyThread  threadMembFunc;
    std::thread threadFunc(&MyThread::myThread_in,&threadMembFunc);
    threadFunc.join();

    return 0;
}