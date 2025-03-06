// https://www.boost.org/doc/libs/1_79_0/libs/msm/doc/HTML/examples/SimpleTutorial.cpp

// Copyright 2010 Christophe Henry
// henry UNDERSCORE christophe AT hotmail DOT com
// This is an extended version of the state machine available in the boost::mpl library
// Distributed under the same license as the original.
// Copyright for the original version:
// Copyright 2005 David Abrahams and Aleksey Gurtovoy. Distributed
// under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
// back-end
#include <boost/msm/back/state_machine.hpp>
// front-end
#include <boost/msm/front/state_machine_def.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;

namespace
{
    //////////////////////////////////////////////////////////////////////
    // events
    struct playEvent
    {
    };
    struct end_pause_Event
    {
    };
    struct stopEvent
    {
    };
    struct pauseEvent
    {
    };
    struct open_close_Event
    {
    };

    // A "complicated" event type that carries some data.
    enum DiskTypeEnum
    {
        DISK_CD = 0,
        DISK_DVD = 1
    };

    struct cd_detected_Event
    {
        cd_detected_Event(std::string name, DiskTypeEnum diskType)
            : name(name),
            disc_type(diskType)
        {
        }

        std::string name;
        DiskTypeEnum disc_type;
    };

    //////////////////////////////////////////////////////////////////////

    //定义状态机 // front-end: define the FSM structure
    struct playerSM : public msm::front::state_machine_def<playerSM>
    {
        template <class Event, class FSM>
        void on_entry(Event const&, FSM&)
        {
            std::cout << "entering: Player" << std::endl;
        }

        template <class Event, class FSM>
        void on_exit(Event const&, FSM&)
        {
            std::cout << "leaving: Player\n" << std::endl;
        }

        //定义 MainState 状态 // The list of FSM states
        struct EmptyStat : public msm::front::state<>
        {
            // every (optional) entry/exit methods get the event passed.
            template <class Event, class FSM>
            void on_entry(Event const&, FSM&)
            {
                std::cout << "entering: EmptyStat" << std::endl;
            }

            template <class Event, class FSM>
            void on_exit(Event const&, FSM&)
            {
                std::cout << "leaving: EmptyStat\n" << std::endl;
            }
        };

        struct OpenState : public msm::front::state<>
        {
            template <class Event, class FSM>
            void on_entry(Event const&, FSM&)
            {
                std::cout << "entering: OpenState" << std::endl;
            }

            template <class Event, class FSM>
            void on_exit(Event const&, FSM&)
            {
                std::cout << "leaving: OpenState\n" << std::endl;
            }
        };

        // sm_ptr still supported but deprecated as functors are a much better way to do the same thing
        struct StoppedState : public msm::front::state<msm::front::default_base_state, msm::front::sm_ptr>
        {
            template <class Event, class FSM>
            void on_entry(Event const&, FSM&)
            {
                std::cout << "entering: StoppedState" << std::endl;
            }

            template <class Event, class FSM>
            void on_exit(Event const&, FSM&)
            {
                std::cout << "leaving: StoppedState\n" << std::endl;
            }

            void set_sm_ptr(playerSM* pl)
            {
                m_player = pl;
            }

            playerSM* m_player;
        };

        struct PlayingState : public msm::front::state<>
        {
            template <class Event, class FSM>
            void on_entry(Event const&, FSM&)
            {
                std::cout << "entering: PlayingState" << std::endl;
            }

            template <class Event, class FSM>
            void on_exit(Event const&, FSM&)
            {
                std::cout << "leaving: PlayingState\n" << std::endl;
            }
        };

        // state not defining any entry or exit
        struct Pausedtate : public msm::front::state<>
        {
        };

        // typedef xxx initial_state 表示状态机 从状态 xxx 开始。
        // the initial state of the player SM. Must be defined
        typedef EmptyStat initial_state;

        // 转换动作 transition actions
        void start_playback_Act(playEvent const&)
        {
            std::cout << "player::start_playback_Act\n";
        }
        void open_drawer_Act(open_close_Event const&)
        {
            std::cout << "player::open_drawer_Act\n";
        }
        void close_drawer_Act(open_close_Event const&)
        {
            std::cout << "player::close_drawer_Act\n";
        }

        void store_cd_info_Act(cd_detected_Event const&)
        {
            std::cout << "player::store_cd_info_Act\n";
        }

        void stop_playback_Act(stopEvent const&)
        {
            std::cout << "player::stop_playback_Act\n";
        }
        void pause_playback_Act(pauseEvent const&)
        {
            std::cout << "player::pause_playback_Act\n";
        }
        void resume_playback_Act(end_pause_Event const&)
        {
            std::cout << "player::resume_playback_Act\n";
        }

        void stop_and_open_Act(open_close_Event const&)
        {
            std::cout << "player::stop_and_open_Act\n";
        }
        void stopped_again_Act(stopEvent const&)
        {
            std::cout << "player::stopped_again_Act\n";
        }

        // 守护条件  guard conditions
        bool good_disk_format_Act(cd_detected_Event const& evt)
        {
            // to test a guard condition, let's say we understand only CDs, not DVD
            if (evt.disc_type != DISK_CD)
            {
                std::cout << "good_disk_format_Act wrong disk, sorry  != DISK_CD " << std::endl;
                return false;
            }
            std::cout << " good_disk_format_Act = DISK_CD " << std::endl;
            return true;
        }
        // used to show a transition conflict. This guard will simply deactivate one transition and thus
        // solve the conflict
        bool auto_start_Act(cd_detected_Event const&)
        {
            return false;
        }

        typedef playerSM p; // makes transition table cleaner

        // 转换表 Transition table for player
        // 守卫和动作是可调用的对象，将由状态机执行，以验证转换后的操作是否应该发生

        // 在Boost.Statechart库中，"Start"、"Event"、"Next"、"Action"和"Guard"是用于定义状态机转换表的概念。
        // 执行顺序：Guard -> Action -> Start -> Event -> Next

        // "Start"（起始状态）：
        // "Start"表示转换规则中的当前状态（源状态）。
        // 在转换发生之前，状态机必须处于"Start"状态。
        // 它指定了转换规则的起始状态。

        // "Event"（事件）：
        // "Event"表示触发状态转换的事件。
        // 当某个特定事件发生时，状态机可以根据转换规则进行状态转换。
        // 事件可以是用户定义的任何类型，通常是一个类或结构体。

        // "Next"（下一个状态）：
        // "Next"表示转换规则中的目标状态。
        // 当状态机接收到指定的事件时，它将从当前状态转移到"Next"状态。
        // 目标状态可以是任何有效的状态，通常是状态机中定义的另一个状态。

        // "Action"（动作）：
        // "Action"表示在状态转换发生时执行的操作。
        // 它可以是一个函数、成员函数、函数对象或Lambda表达式，用于在状态转换时执行特定的逻辑。
        // "Action"可以用于更新状态机的内部状态、执行某些任务或发出其他事件。

        // "Guard"（条件）：
        // "Guard"表示在执行状态转换之前进行的条件检查。
        // 它是一个函数、成员函数或函数对象，用于判断是否满足执行状态转换的条件。
        // 如果"Guard"返回true，则状态转换会发生；如果返回false，则状态转换被阻止。
        // 综上所述，"Start"指定了转换规则的起始状态，"Event"表示触发状态转换的事件，"Next"表示转换规则的目标状态，
        // "Action"表示状态转换时执行的动作，"Guard"表示执行状态转换之前的条件检查。

        // 通过定义转换表中的这些概念，可以规定状态机在特定事件发生时如何从一个状态转移到另一个状态，并在转换过程中执行相应的动作和条件检查。

        // 在Boost.Statechart库中，状态机转换中的各个部分（Start、Event、Next、Action、Guard）的执行顺序如下：

        // Guard（条件/守卫）：
        // 在执行状态转换之前，首先会评估条件（Guard）。
        // Guard函数将根据条件判断是否允许执行状态转换。
        // 如果Guard返回true，则允许执行状态转换；如果返回false，则阻止状态转换。

        // Action（动作）：
        // 如果Guard返回true，则在执行状态转换之前会执行相应的动作（Action）。
        // 动作可以是一个函数、成员函数、函数对象或Lambda表达式。
        // 它用于在状态转换时执行特定的逻辑，例如更新状态机的内部状态、执行某些任务或发出其他事件。

        // Start（起始状态）：
        // 在经过Guard检查和执行Action后，状态机将当前状态切换到转换规则中指定的起始状态（Start）。
        // 转换规则中的起始状态将成为当前状态，准备进行下一步的状态转换。

        // Event（事件）：
        // 当状态机处于起始状态时，它等待特定的事件（Event）发生。
        // 当接收到匹配的事件时，状态机将会根据转换规则进行状态转换。

        // Next（下一个状态）：
        // 如果收到匹配的事件，则状态机将从当前状态转移到转换规则中指定的目标状态（Next）。
        // 目标状态可以是状态机中定义的另一个状态。

        // 总结一下执行顺序：Guard -> Action -> Start -> Event -> Next

        // 请注意，Guard和Action都是可选的。如果转换规则中没有定义Guard，状态机将直接执行Action和状态转换。
        // 如果没有定义Action，状态转换将在Guard通过后立即发生。

        // row takes 5 arguments: start state, event, target state, action and guard.
        // • a_row (“a” for action) allows defining only the action and omit the guard condition.
        // • g_row (“g” for guard) allows omitting the action behavior and defining only the guard.
        // • _row allows omitting action and guard.
        // The transition table is actually a MPL vector (or list), which brings the limitation that the default maximum size of the table is 20

        // 在Boost.Statechart库中，a_row是一种特殊的宏，用于定义状态机的转换表。
        // 具体来说，a_row是一个宏，用于创建转换表中的单个转换规则。它提供了一种简洁的语法，使得定义转换表更加方便。
        // 使用a_row宏，可以按以下格式定义转换规则：
        // a_row<Event, Source, Target, Action, Guard>

        // 其中：
        // Event是触发状态转换的事件类型。
        // Source是当前状态。
        // Target是目标状态。
        // Action是在状态转换时执行的动作（可选）。
        // Guard是在执行状态转换之前进行的条件检查（可选）。

        // _row是a_row的别名，用于创建转换表中的单个转换规则。
        // _row宏与a_row宏在功能上是相同的，只是名称不同。
        // _row宏主要用于提供一种简短的写法，使代码更紧凑。

        struct transition_table : mpl::vector<
            //    Start          Event                Next           Action				 Guard
            //  +-------------+--------------------+-------------+---------------------+----------------------+
            a_row < StoppedState, playEvent, PlayingState, &p::start_playback_Act                         >,
            a_row < StoppedState, open_close_Event, OpenState, &p::open_drawer_Act                            >,
            _row < StoppedState, stopEvent, StoppedState                                              >,
            //  +---------+-------------+---------+---------------------+----------------------+
            a_row < OpenState, open_close_Event, EmptyStat, &p::close_drawer_Act                           >,
            //  +---------+-------------+---------+---------------------+----------------------+
            a_row < EmptyStat, open_close_Event, OpenState, &p::open_drawer_Act                            >,
            row < EmptyStat, cd_detected_Event, StoppedState, &p::store_cd_info_Act, &p::good_disk_format_Act  >,
            row < EmptyStat, cd_detected_Event, PlayingState, &p::store_cd_info_Act, &p::auto_start_Act        >,

            a_row < EmptyStat, playEvent, PlayingState, &p::start_playback_Act                             >,

            //  +---------+-------------+---------+---------------------+----------------------+
            a_row < PlayingState, stopEvent, StoppedState, &p::stop_playback_Act                          >,
            a_row < PlayingState, pauseEvent, Pausedtate, &p::pause_playback_Act                         >,
            a_row < PlayingState, open_close_Event, OpenState, &p::stop_and_open_Act                          >,
            //  +---------+-------------+---------+---------------------+----------------------+
            a_row < Pausedtate, end_pause_Event, PlayingState, &p::resume_playback_Act                        >,
            a_row < Pausedtate, stopEvent, StoppedState, &p::stop_playback_Act                          >,
            a_row < Pausedtate, open_close_Event, OpenState, &p::stop_and_open_Act                          >
            //  +---------+-------------+---------+---------------------+----------------------+
        >
        {
        };

        // Replaces the default no-transition response.
        template <class FSM, class Event>
        void no_transition(Event const& e, FSM&, int state)
        {
            std::cout << "no transition from state " << state
                << " on event " << typeid(e).name() << std::endl;
        }
    };

    // Pick a back-end
    typedef msm::back::state_machine<playerSM> playerStateMachine;

    //
    // Testing utilities.
    //
    static char const* const state_names[] =
    { "StoppedState", "OpenState", "EmptyState", "PlayingState", "PausedState" };

    void printState(playerStateMachine const& p)
    {
        std::cout << " current state is -----> " << state_names[p.current_state()[0]] << std::endl;
    }

    void test()
    {
        playerStateMachine p;
        // needed to start the highest-level SM. This will call on_entry and mark the start of the SM
        p.start(); // entering: Player     entering: EmptyStat

        // go to Open, call on_exit on EmptyStat, then action, then on_entry on Open
        p.process_event(open_close_Event());    // leaving: EmptyStat    player::open_drawer_Act    entering: OpenState
        printState(p);      // current state is -----> OpenState

        p.process_event(open_close_Event());    // leaving: OpenState    player::close_drawer_Act   entering: EmptyStat
        printState(p);      // current state is -----> EmptyState

        // 事件转换失败 will be rejected, wrong disk type
        p.process_event(cd_detected_Event("aa, bb", DISK_DVD));   // good_disk_format_Act wrong disk, sorry  != DISK_CD
        printState(p);  // current state is -----> EmptyState

        p.process_event(cd_detected_Event("xx, yy", DISK_CD));    //  good_disk_format_Act = DISK_CD  leaving: EmptyStat  player::store_cd_info_Act  entering: StoppedState
        printState(p);  // current state is -----> StoppedState

        p.process_event(playEvent());   // leaving: StoppedState        player::start_playback_Act       entering: PlayingState

        // at this point, Play is active
        p.process_event(pauseEvent());  // leaving: PlayingState    player::pause_playback_Act
        printState(p);                  //  current state is -----> PausedState

        // go back to PlayingState
        p.process_event(end_pause_Event()); // player::resume_playback_Act      entering: PlayingState
        printState(p);                  // current state is -----> PlayingState

        p.process_event(pauseEvent());  // leaving: PlayingState        player::pause_playback_Act
        printState(p);                  // current state is -----> PausedState

        p.process_event(stopEvent());   // player::stop_playback_Act    entering: StoppedState
        printState(p);                  //  current state is -----> StoppedState

        // event leading to the same state
        // no action method called as it is not present in the transition table
        p.process_event(stopEvent());   // leaving: StoppedState    entering: StoppedState
        printState(p);                  //  current state is -----> StoppedState

        std::cout << "stopEvent fsm" << std::endl;  // stopEvent fsm
        p.stop();                       // leaving: StoppedState    leaving: Player
    }
}

int main()
{
    test();
    return 0;
}