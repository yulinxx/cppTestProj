// https://blog.csdn.net/xiefeifei316948714/article/details/24192303

#include <iostream>
#include <ctime>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

namespace sc = boost::statechart;

////////////////////////////////////////////////////////////////////////////
//定义事件    《根据事件 切换 状态》
// 状态机对外部或内部事件感兴趣，引起状态转移。例如：当人处于混混欲睡状态时，突然收到一个好消息事件，人就改变为兴奋状态了。
// struct Evt: sc::event< Evt > {}; //事件类。当然可以加入自己想传达的任何信息。
// struct Evt: sc::event< Evt > { int raised_salary; }; //可以加一些成员变量进去。
class EvtStartStop : public sc::event< EvtStartStop >
{
};

class EvtReset : public sc::event< EvtReset >
{
};

class EvtGo : public sc::event< EvtGo >
{
};

////////////////////////////////////////////////////////////////////////////
class MainState;
class StopState;
class RunState;
class TwoState;

////////////////////////////////////////////////////////////////////////////
//定义状态机                                      初始化状态 MainState
// struct M : state_machine< M, A > {}; // 定义一个状态机类M。初始状态在A
class XXMachine : public sc::state_machine< XXMachine, MainState >
{
};

////////////////////////////////////////////////////////////////////////////
//定义 MainState 状态 ， 它属于Machine状态机， 它的初始子状态为 StopState

// struct A : simple_state<A, M>{}; // 定义一个状态A，属于M状态机。
// struct A : simple_state<A, M, A1>{}; // 定义一个状态A，属于M状态机。A状态有初始子状态A1
// 一般状态对象里不存放任何变量。我们把数据存放到状态机M里
class MainState : public sc::simple_state< MainState, XXMachine, StopState >
{
public:
    // 当状态感觉到一个事件发生时，可以对这个事件做出反应。状态转移就是一条“规则”，指明如何反应。例如：
    // 当有人骂你这个事件发生时，你可以根据“恶意度”，骂人者与你的“亲密程度”，选择忽略这个事件，或者转移到“暴怒状态”，或者做出一定回击。
    // struct A1 : simple_state<A1, A> {
    // typedef transition< Evt, B  > reactions; }; //状态转移规则用typedef说明，注意reactions不能拼错。

    // typedef transition< Evt, B  > reactions; //当遇到事件Evt时，就转移到状态B。
    // 状态转移能从一个状态跳转到任意的一个状态，即使状态是嵌套的深层子状态。

    typedef sc::transition< EvtReset, MainState > reactReset;	//状态切换
    typedef sc::transition< EvtGo, TwoState > reactGo;			//状态切换

    // 多事件 一个状态可以定义任意数量的动作。这就是为什么当多于一个时，我们不得不将它们放到一个mpl::list<> 里。
    typedef boost::mpl::list< reactReset, reactGo > reactions;  //reactions 切不可拼写错误

    MainState(void)
    {
        std::cout << "---Enter MainState" << std::endl;
        mTime = 0;
    }

    ~MainState(void)
    {
        std::cout << "Exit MainState" << std::endl;
    }

    double mTime;
};

////////////////////////////////////////////////////////////////////////////
// 该状态属于无用状态，用于测试mpl::list的多transition用法
class TwoState : public sc::simple_state< TwoState, XXMachine >
{
public:
    typedef sc::transition< EvtGo, MainState > reactions; //状态切换

    TwoState(void)
    {
        std::cout << "---Enter TwoState" << std::endl;
    }

    ~TwoState(void)
    {
        std::cout << "Exit TwoState" << std::endl;
    }
};

////////////////////////////////////////////////////////////////////////////
class StopState : public sc::simple_state< StopState, MainState >
{
public:
    typedef sc::transition< EvtStartStop, RunState > reactions; //状态切换

    StopState(void)
    {
        std::cout << "---Enter StopState" << std::endl;
    }

    ~StopState(void)
    {
        std::cout << "Exit StopState" << std::endl;
    }
};

////////////////////////////////////////////////////////////////////////////
class RunState : public sc::simple_state< RunState, MainState >
{
public:
    typedef sc::transition< EvtStartStop, StopState > reactions;
    RunState(void)
    {
        std::cout << "---Enter RunState" << std::endl;
        mStartTime = 0;
    }

    ~RunState(void)
    {
        std::cout << "Exit RunState" << std::endl;
        context<MainState>().mTime += std::difftime(std::time(0), mStartTime);
    }

    std::time_t mStartTime;
};

///////////////////////////////////////////////////////////////////////////

int main()
{
    std::cout << "-------------- 1 --------------" << std::endl;
    // 事件首先被投递给最深层的当前子状态，让后向外层投递。
    XXMachine mc;
    mc.initiate();  // 进入到 MainState

    std::cout << "\n-------------- 2 --------------" << std::endl;
    // 投递事件 最简单是process_event函数
    mc.process_event(EvtStartStop());
    std::cout << std::endl;

    std::cout << "-------------- 3 --------------" << std::endl;
    mc.process_event(EvtStartStop());
    std::cout << std::endl;

    std::cout << "-------------- 4 --------------" << std::endl;
    mc.process_event(EvtReset());
    std::cout << std::endl;

    std::cout << "-------------- 5 --------------" << std::endl;
    mc.process_event(EvtGo());
    std::cout << std::endl;

    std::cout << "-------------- 6 --------------" << std::endl;
    mc.process_event(EvtGo());

    std::cout << "-------------- End --------------" << std::endl;
    return 0;
}