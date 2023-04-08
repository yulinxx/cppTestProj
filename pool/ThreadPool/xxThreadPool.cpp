#include "xxThreadPool.h"

ThreadPool* xxThreadPool::threadPool = nullptr;
std::mutex xxThreadPool::mutex;

xxThreadPool::xxThreadPool()
{
    init();
}

xxThreadPool::xxThreadPool(const xxThreadPool &)
{
    init();
}

xxThreadPool& xxThreadPool::operator = (const xxThreadPool &)
{
    return *this;
}

xxThreadPool::~xxThreadPool()
{
}

void xxThreadPool::init() 
{
}

ThreadPool* xxThreadPool::instance(int CpuNum /*=4*/)
{
    if (!threadPool)
    {
        const std::lock_guard<std::mutex> lock(mutex);

        // CPU数量
        threadPool = new ThreadPool(CpuNum);
    }
    return threadPool;
}
