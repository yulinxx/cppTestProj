// 在多线程编程中，std::promise 通常与 std::future 一起使用，以实现线程之间的数据传递和同步。
// 下面是一个示例，演示了如何在两个线程之间使用 std::promise 和 std::future 进行数据传递：

#include <iostream>
#include <thread>
#include <future>

// 线程函数，将计算结果存储到 std::promise 对象中
void calculateResult(std::promise<int>& promiseObj, int a, int b)
{
    int result = a + b;

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // 将结果存储到 std::promise 对象中
    promiseObj.set_value(result);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

int main()
{
    std::cout << "Begin " << std::endl;
    // 创建一个 std::promise 对象和与之关联的 std::future 对象
    std::promise<int> promiseObj;
    std::future<int> futureObj = promiseObj.get_future();

    // 启动一个线程，并将 std::promise 对象传递给线程函数
    std::thread threadObj(calculateResult, std::ref(promiseObj), 10, 20);
    std::cout << "Start Thread" << std::endl;

    // 在主线程中获取结果
    int result = futureObj.get();

    std::cout << "Result: " << result << std::endl;

    // 等待线程结束并加入
    threadObj.join();

    std::cout << "End" << std::endl;
    return 0;
}