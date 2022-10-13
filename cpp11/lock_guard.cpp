// lock_guard 类是一个mutex封装者，它为了拥有一个或多个mutex而提供了一种方便的 RAII style 机制

// RAII，全称为Resource Acquisition Is Initialization，汉语是“资源获取即初始化”。
// 但是这个直译并没有很好地解释这个词组的含义。其实含义并不高深复杂，简单说来就是，
// 在资源获取的时候将其封装在某类的object中，利用"栈资源会在相应object的生命周期结束时自动销毁"来自动释放资源，
// 即将资源释放写在析构函数中。
// 所以这个RAII其实就是和智能指针的实现是类似的。

#include <thread>
#include <mutex>
#include <iostream>
 
int g_i = 0;
std::mutex g_i_mutex;  // protects g_i
 
void safe_increment()
{
    std::lock_guard<std::mutex> lock(g_i_mutex);
    ++g_i;
 
    std::cout << std::this_thread::get_id() << ": " << g_i << '\n';
 
    // g_i_mutex is automatically released when lock goes out of scope
}
 
int main()
{
    std::cout << __func__ << ": " << g_i << '\n';
 
    std::thread t1(safe_increment);
    std::thread t2(safe_increment);
 
    t1.join();
    t2.join();
 
    std::cout << __func__ << ": " << g_i << '\n';
}