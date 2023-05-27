// 在C++11及其之后的版本中，读写锁（Reader-Writer Lock）的实现是通过`std::shared_mutex`类来实现的。
// 读写锁允许多个线程同时对共享资源进行读访问，但只有一个线程能够进行写访问。这种锁可以提高并发性能，适用于读操作远远多于写操作的场景。

// 读写锁有两种模式：共享模式（shared mode）和独占模式（exclusive mode）。

// - 共享模式：多个线程可以同时获得读写锁的共享模式，允许对共享资源进行读操作。多个共享模式的锁可以同时持有，彼此不会阻塞。
// - 独占模式：只有一个线程能够获得读写锁的独占模式，允许对共享资源进行写操作。当有线程持有独占模式的锁时，其他线程无法获取独占模式或共享模式的锁，它们会被阻塞。

// 以下是一个简单示例，演示了如何使用`std::shared_mutex`：

#include <iostream>
#include <thread>
#include <shared_mutex>

std::shared_mutex mtx;
int sharedData = 0;

void readerFunction()
{
    std::shared_lock<std::shared_mutex> lock(mtx);
    std::cout << "Reader: " << sharedData << std::endl;
}

void writerFunction()
{
    std::unique_lock<std::shared_mutex> lock(mtx);
    sharedData++;
    std::cout << "Writer: " << sharedData << std::endl;
}

int main()
{
    std::thread readerThread1(readerFunction);
    std::thread readerThread2(readerFunction);
    std::thread writerThread(writerFunction);

    readerThread1.join();
    readerThread2.join();
    writerThread.join();

    return 0;
}

// 在上述示例中，`std::shared_mutex` 被用于保护共享数据 `sharedData` 的读写操作。
// `readerFunction` 使用共享模式的锁进行读操作，而 `writerFunction` 使用独占模式的锁进行写操作。

// 需要注意的是，`std::shared_lock` 用于共享模式的锁，`std::unique_lock` 用于独占模式的锁。
// 共享模式的锁允许多个线程同时获取锁，而独占模式的锁只允许一个线程获取锁。

// 使用读写锁时，请根据实际情况选择适当的模式（共享模式或独占模式），以实现对共享资源的合理读写访问，并提高并发性能。