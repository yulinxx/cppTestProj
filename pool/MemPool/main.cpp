#include "MemoryPool.h"

#include <iostream>
#include <chrono>
#include <random>

class TestClass
{
public:
    double dA = 0.0;
    double dB = 0.0;
    double dC = 0.0;
    double dD = 0.0;
};

int main()
{
    std::cout << "---- Mem Pool Test ----" << std::endl;
    int testItemNum = 100000;

    unsigned seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 randGen(seed);
    std::uniform_int_distribution<int> ptCoord(0, 1800);

    std::vector<TestClass*> vecNewItems;
    vecNewItems.reserve(testItemNum);

    std::vector<TestClass*> vecPoolItems;
    vecPoolItems.reserve(testItemNum);

    std::chrono::time_point<std::chrono::system_clock> t1;
    std::chrono::time_point<std::chrono::system_clock> t2;
    std::chrono::time_point<std::chrono::system_clock> t3;

    t1 = std::chrono::system_clock::now();
    {
        // 使用new
        for (int i = 0; i < testItemNum; i++)
        {
            TestClass* pItem = new TestClass();	// 使用 new 分配内存
            vecNewItems.emplace_back(pItem);
        }

        for (auto& item : vecNewItems)
        {
            delete item;		// 使用 delete 分配内存
        }
    }

    t2 = std::chrono::system_clock::now();
    {
        // 使用内存池
        MemoryPool<TestClass> pathItemPool;	// 分配 TestClass* 类型的内存池

        for (int i = 0; i < testItemNum; i++)
        {
            TestClass* pItem = pathItemPool.newElement();	// 使用 newElement 分配内存
            vecPoolItems.emplace_back(pItem);
        }

        for (auto& item : vecPoolItems)
        {
            pathItemPool.deleteElement(item);		// 使用 deleteElement 分配内存
        }
    }

    t3 = std::chrono::system_clock::now();

    // 打印时间
    std::chrono::duration<double, std::milli> msNew = t2 - t1;	// new
    std::chrono::duration<double, std::milli> msPool = t3 - t2;	// pool

    double dA = msNew.count();
    double dB = msPool.count();

    std::cout << " ---new:" << dA << " ----- mem pool:" << dB << std::endl;

    return 0;
}