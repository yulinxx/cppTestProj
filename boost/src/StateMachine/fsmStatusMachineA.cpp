/*
 * @Author: xx xx@ubuntu.com
 * @Date: 2022-06-15 22:53:40
 * @LastEditors: xx xx@ubuntu.com
 * @LastEditTime: 2022-06-15 23:03:10
 * @FilePath: /boostTestProj/src/fsmStatusMachine
 * @Description:
 * boost 有限状态机（FSM）_Jff316948714的博客-CSDN博客_boost fsm
 * https://blog.csdn.net/xiefeifei316948714/article/details/24192303
 */

#include <iostream>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>

namespace sc = boost::statechart;

struct Greeting;

/*
boost::statechart 大量应用模板模式。 派生类必须将自己作为基类模板的第一个参数。
状态机必须知道当其初始化后进行的第一个状态。 这里将 Greeting 定义为其进入后的第一个状态。
state_machine
*/
struct Machine : sc::state_machine< Machine, Greeting >
{
};

/*
对于每一个状态，我们需要为其指明：它属于哪一个状态机  simple_state<> 指定
*/
struct Greeting : sc::simple_state< Greeting, Machine >
{
    /*
    状态机进入一个状态，就会创建一个相应的该状态的对象 (Constructor)。
    保持当前状态，这个对象才会一直存在。 状态机离开该状态时，对象被销毁 (Destructor)。
    */
    Greeting()
    {
        std::cout << "Greeting Constructor !\n";
    }

    ~Greeting()
    {
        std::cout << "Greeting Destructor !" << std::endl;
    }
};

int main()
{
    //构造完状态机后，它并未开始运行。我们要通过调用它的initiate()来启动它。
    //同时，它也将触发它的初始状态（Greeting）的构造。
    Machine myMachine;

    // 当我们离开main()函数时，myMachine将被销毁，这将导致它销毁它内部的所有活动的状态类。
    // (译者注：为什么会说所有？这是因为一个状态机可以同时 保持在多个状态中)
    myMachine.initiate(); // 初始化后进入第一个状态

    return 0;
}