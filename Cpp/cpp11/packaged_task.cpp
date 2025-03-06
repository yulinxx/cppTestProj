// std::packaged_task 是 C++11 中的一个类模板，它用于包装一个可调用对象（比如函数、函数对象或 Lambda 表达式），
// 并将其与一个异步任务关联起来,可以获取任务的结果。
// std::packaged_task 提供了一种在异步操作中获取返回值的方式，
// 可以将其看作是一个异步任务的“未来(future)”。

// 下面是使用 std::packaged_task 的一般步骤：
// 1.
// 创建 std::packaged_task 对象，并指定可调用对象的类型作为模板参数。
// 例如，如果要封装一个返回 int 类型、参数为 int 的函数，可以使用 std::packaged_task<int(int)> task 来创建对象。
// 2.
// 将可调用对象与 std::packaged_task 关联起来。
// 可以通过构造函数、赋值操作符或移动构造函数来实现关联。
// 例如，可以使用 std::packaged_task<int(int)> task(func) 将名为 func 的函数关联到 task 对象。
// 3.
// 调用 std::packaged_task 对象的 operator() 方法来执行异步任务。
// 可以为可调用对象传递参数，例如 task(42)，这将调用关联的可调用对象并传递参数 42。
// 4.
// 如果需要获取异步任务的结果，可以通过调用 std::packaged_task 对象的 get_future() 方法
// 来获取与任务关联的 std::future 对象。std::future 提供了一种在异步任务完成后获取结果的机制。
// 5.
// 可以使用 std::future 对象的 get() 方法来阻塞当前线程，直到异步任务完成并返回结果。
// 例如，可以使用 int result = future.get() 来获取异步任务的结果，并将其存储在 result 变量中。

#include <iostream> // std:cout
#include <future>
#include <thread>

int main()
{
    // 声明一个名为 foo 的 packaged_task 对象，并使用默认构造函数进行初始化。
    // 这样做会创建一个无效的 对象，因为它没有与任何可调用对象相关联。
    std::packaged_task<int(int)> foo;

    // 然后，创建另一个 packaged_task 对象 bar，并将一个匿名函数作为参数传递给它的构造函数。
    // 这个匿名函数是一个简单的 lambda 表达式，它将输入参数 x 乘以 2 并返回结果。
    std::packaged_task<int(int)> bar(
        [](int x) {
            return x * 2;
        });

    // 将 bar 对象的状态转移到 foo 对象中。
    // foo 现在拥有了与 bar 相同的可调用对象（即匿名函数）。转移后，bar 的状态变为无效。
    // 这样做的好处是，可以重用已经创建的 packaged_task 对象，
    // 并将新的可调用对象与之关联，而无需重新分配内存或创建新的对象。
    foo = std::move(bar);

    std::future<int> res = foo.get_future();

    // 将 packaged_task 作为一个异步任务在另一个线程中执行
    std::thread(std::move(foo), 10).detach();

    // 等待异步任务完成并获取结果
    std::cout << "res: " << res.get();
    return 0;
}