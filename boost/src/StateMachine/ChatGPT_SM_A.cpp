// https://www.boost.org/doc/libs/1_82_0/libs/statechart/doc/tutorial.html

// 使用Boost Statechart Library构建状态机的过程涉及以下步骤：

// 1. 包含头文件：在使用Boost Statechart之前，需要包含相关的头文件。
// 常见的头文件包括`boost/statechart/state_machine.hpp`、`boost/statechart/simple_state.hpp`和`boost/statechart/transition.hpp`。

// 2. 定义状态：创建表示状态的类。你可以通过继承`boost::statechart::simple_state`或`boost::statechart::state`类来定义状态。
// 状态类可以包含成员函数和成员变量，用于表示状态的行为和属性。

// 3. 定义事件：创建表示事件的类。你可以使用`boost::statechart::event`类或其派生类来定义事件。事件类可以包含必要的数据和方法。

// 4. 定义转换规则：使用`boost::statechart::transition`宏或`BOOST_STATECHART_TRANSITION`宏来定义状态之间的转换规则。
// 转换规则指定了触发转换的事件类型、源状态和目标状态，以及可选的行为。

// 5. 定义状态机：创建表示状态机的类。状态机类应继承自`boost::statechart::state_machine`类，并指定初始状态。

// 6. 初始化状态机：创建状态机对象，并使用`initiate()`函数初始化状态机。这将触发初始状态的进入动作，并使状态机准备好处理事件。

// 7. 处理事件：通过调用状态机对象的`process_event()`函数，将事件传递给状态机以触发相应的状态转换。状态机将自动查找匹配的转换规则并执行相应的行为。

// 8. 运行状态机：状态机将根据当前状态和触发的事件自动执行状态转换和行为。你可以使用循环或其他机制来不断提供事件，并让状态机保持运行状态。

// 9. 其他功能：Boost Statechart Library还提供了其他功能，如状态退出动作、并行状态和历史状态。可以参考Boost文档以了解更多详细信息和高级用法。

// 使用Boost Statechart Library可以创建复杂的状态机，并通过状态之间的转换和事件处理来控制系统的行为。
// 该库提供了清晰的接口和丰富的功能，帮助开发人员构建可靠和可扩展的状态机应用程序。


// Boost 提供了一个功能强大的状态机库，称为 Boost.Statechart。
// 使用 Boost.Statechart，你可以方便地定义和管理复杂的状态机，以实现状态的转换和行为的控制。

// 下面是一个简单的示例，展示了如何使用 Boost.Statechart 来创建一个基本的状态机：
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <iostream>

namespace sc = boost::statechart;

// 定义状态
struct State1;
struct State2;
struct State3;

// 定义事件
struct Event1 : sc::event<Event1> {};
struct Event2 : sc::event<Event2> {};
struct Event3 : sc::event<Event3> {};

// 定义状态机
struct StateMachine : sc::state_machine<StateMachine, State1> {};

// 定义状态1
struct State1 : sc::simple_state<State1, StateMachine>
{
    State1()
    {
        std::cout << "Entering State1" << std::endl;
    }

    // 定义从State1到State2的转换规则和行为
    typedef sc::transition<Event1, State2> reactions;
};

// 定义状态2
struct State2 : sc::simple_state<State2, StateMachine>
{
    State2()
    {
        std::cout << "Entering State2" << std::endl;
    }

    // 定义从State2到State3的转换规则和行为
    typedef sc::transition<Event2, State3> reactions;
};

// 定义状态3
struct State3 : sc::simple_state<State3, StateMachine>
{
    State3()
    {
        std::cout << "Entering State3" << std::endl;
    }

    // 定义从State3到State1的转换规则和行为
    typedef sc::transition<Event3, State1> reactions;
};

int main()
{
    StateMachine sm;
    sm.initiate();

    // 触发事件并执行状态转换
    sm.process_event(Event1());
    sm.process_event(Event2());
    sm.process_event(Event3());

    return 0;
}

// 在上述示例中，我们首先包含了 Boost.Statechart 库的相关头文件。
// 然后，我们定义了三个状态：State1、State2 和 State3，以及三个事件：Event1、Event2 和 Event3。

// 接下来，我们定义了一个名为 StateMachine 的状态机，继承自 sc::state_machine。
// 通过指定初始状态为 State1，我们创建了一个 StateMachine 对象。

// 在各个状态中，我们使用 typedef 来定义从一个状态到另一个状态的转换规则，并指定触发转换的事件类型。
// 每个状态的构造函数中，我们打印相应的进入状态的消息。

// 在 main 函数中，我们创建了 StateMachine 对象 sm，并使用 sm.initiate() 初始化状态机。
// 然后，通过调用 sm.process_event() 函数，我们触发了一系列事件，从而执行状态转换。

// 通过运行上述代码，你将看到状态的进入信息被打印出来，表示状态机的转换和状态的管理已经成功。
// 你可以根据需要定义更多的状态和转换规则，以构建更复杂的状态机。

// 请注意，这只是一个简单的示例，实际使用中可能需要根据具体需求进行