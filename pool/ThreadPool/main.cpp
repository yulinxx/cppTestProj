#include "xxThreadPool.h"
#include <iostream>
#include <future>
#include <vector>

class testObj
{
public:
    int test(int i)
    {
        std::cout << i << "---" << std::endl;
        return 0;
    }
};

int main()
{
    std::cout << "---ThreadTest---" << std::endl;

    auto pool = xxThreadPool::instance(8);

    int i = 0;
    std::vector<std::future<int>> vecFuture;

    for (int i = 0; i < 20; i++)
    {
        testObj obj;

        vecFuture.emplace_back(
            pool->enqueue(&testObj::test, &obj, i));
    }

    // 等待线程结束
    for (auto&& fut : vecFuture)
    {
        if (fut.valid())
            fut.get();
    }

    vecFuture.clear();
    std::vector<std::future<int>>().swap(vecFuture);

    return 0;
}