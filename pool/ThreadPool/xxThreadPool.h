#ifndef _RD_THREADPOOL_H_
#define _RD_THREADPOOL_H_

#include "ThreadPool.h"

#include <mutex>

/*
使用示例如:
// 1.用于保存线程的future,对线程退出进行判断
std::vector< std::future<int> > vecFuture;

// 2.获取实例
ThreadPool* threadPool = xxThreadPool::instance();
// MainWindow中的 parseFile放至线程池中操作, 
// this 为对象实例, filePath 为 parseFile函数所需参数
auto future = threadPool->enqueue(&MainWindow::parseFile, this, filePath);
vecFuture.emplace_back(future);

// 3.退出判断
for (auto&& fut : vecFuture)
{
	if (fut.valid())
	{
		fut.get();
	}
}

注意:
不要在工作线程中对主线程界面进行操作
*/


//////////////////////////////////////////////// 
class xxThreadPool
{
public:
	// CPU数量
	static ThreadPool* instance(int CpuNum = 4);

private:
	xxThreadPool();
	xxThreadPool(const xxThreadPool &);
	xxThreadPool & operator = (const xxThreadPool &);

	virtual ~xxThreadPool();

private:
	void init();

private: 
	static ThreadPool* threadPool;
	static std::mutex mutex;
};


#endif //_RD_THREADPOOL_H_
