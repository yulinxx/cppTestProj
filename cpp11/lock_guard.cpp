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
 
 
 // unique_lock取代lock_guard
// unique_lock是个类模板，工作中，一般lock_guard(推荐使用)；lock_guard取代了mutex的lock()和unlock();
// unique_lock比lock_guard灵活很多，效率上差一点，内存占用多一点。
// ock_guard可以带第二个参数：
// std::lock_guard<std::mutex> sbguard1(my_mutex1, std::adopt_lock);// std::adopt_lock标记作用；

// std::recursive_mutex 是C++标准库中提供的一种递归锁（Recursive Lock）的实现。
// 递归锁允许同一个线程多次获得锁，这在某些情况下非常有用，例如当同一个线程在递归函数中多次调用某个被锁保护的代码段时。

// std::recursive_mutex 的用法与互斥锁（std::mutex）非常类似，但是可以被同一个线程多次加锁，而不会导致死锁。
// 它提供了与互斥锁相同的成员函数，如lock()、unlock()等。

#include <iostream>
#include <thread>
#include <mutex>

std::recursive_mutex mtx;

void recursiveFunction(int count)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);

    if (count > 0)
    {
        std::cout << "Count: " << count << std::endl;
        recursiveFunction(count - 1);
    }
}

// 在C++11及其之后的版本中，读写锁（Reader-Writer Lock）的实现是通过std::shared_mutex类来实现的。
// 读写锁允许多个线程同时对共享资源进行读访问，但只有一个线程能够进行写访问。这种锁可以提高并发性能，适用于读操作远远多于写操作的场景。

// 读写锁有两种模式：共享模式（shared mode）和独占模式（exclusive mode）。

// 共享模式：多个线程可以同时获得读写锁的共享模式，允许对共享资源进行读操作。多个共享模式的锁可以同时持有，彼此不会阻塞。
// 独占模式：只有一个线程能够获得读写锁的独占模式，允许对共享资源进行写操作。当有线程持有独占模式的锁时，其他线程无法获取独占模式或共享模式的锁，它们会被阻塞。

// std::shared_mutex 被用于保护共享数据 sharedData 的读写操作。
// readerFunction 使用共享模式的锁进行读操作，而 writerFunction 使用独占模式的锁进行写操作。

// 需要注意的是，std::shared_lock 用于共享模式的锁，std::unique_lock 用于独占模式的锁。
// 共享模式的锁允许多个线程同时获取锁，而独占模式的锁只允许一个线程获取锁。

// 使用读写锁时，请根据实际情况选择适当的模式（共享模式或独占模式），以实现对共享资源的合理读写访问，并提高并发性能。

#include <iostream>
#include <thread>
#include <shared_mutex>

std::shared_mutex mtxRW;
int sharedData = 0;

void readerFunction()
{
    std::shared_lock<std::shared_mutex> lock(mtxRW);
    std::cout << "Reader: " << sharedData << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void writerFunction()
{
    std::unique_lock<std::shared_mutex> lock(mtxRW);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    sharedData++;
    std::cout << "Writer: " << sharedData << std::endl;
}

//////////////////////////////////////////////////////

int main()
{
    std::cout << __func__ << ": " << g_i << '\n';
 
    std::thread t1(safe_increment);
    std::thread t2(safe_increment);
 
    t1.join();
    t2.join();
 
    std::cout << __func__ << ": " << g_i << '\n';
    
    // 递归锁
    std::thread t(recursiveFunction, 5);
    t.join();

    // 读写锁
    std::thread readerThread1(readerFunction);
    std::thread readerThread2(readerFunction);
    std::thread writerThread(writerFunction);

    readerThread1.join();
    readerThread2.join();
    writerThread.join();
    

    return 0;
}

