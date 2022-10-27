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
        void on_entry(Event const &, FSM &)
        {
            std::cout << "entering: Player" << std::endl;
        }

        template <class Event, class FSM>
        void on_exit(Event const &, FSM &)
        {
            std::cout << "leaving: Player\n" << std::endl;
        }

        
        //定义 MainState 状态 // The list of FSM states
        struct EmptyStat : public msm::front::state<>
        {
            // every (optional) entry/exit methods get the event passed.
            template <class Event, class FSM>
            void on_entry(Event const &, FSM &)
            {
                std::cout << "entering: EmptyStat" << std::endl;
            }

            template <class Event, class FSM>
            void on_exit(Event const &, FSM &)
            {
                std::cout << "leaving: EmptyStat\n" << std::endl;
            }
        };

        struct OpenState : public msm::front::state<>
        {
            template <class Event, class FSM>
            void on_entry(Event const &, FSM &)
            {
                std::cout << "entering: OpenState" << std::endl;
            }

            template <class Event, class FSM>
            void on_exit(Event const &, FSM &)
            {
                std::cout << "leaving: OpenState\n" << std::endl;
            }
        };

        // sm_ptr still supported but deprecated as functors are a much better way to do the same thing
        struct StoppedState : public msm::front::state<msm::front::default_base_state, msm::front::sm_ptr>
        {
            template <class Event, class FSM>
            void on_entry(Event const &, FSM &)
            {
                std::cout << "entering: StoppedState" << std::endl;
            }

            template <class Event, class FSM>
            void on_exit(Event const &, FSM &)
            {
                std::cout << "leaving: StoppedState\n" << std::endl;
            }

            void set_sm_ptr(playerSM *pl)
            {
                m_player = pl;
            }

            playerSM *m_player;
        };

        struct PlayingState : public msm::front::state<>
        {
            template <class Event, class FSM>
            void on_entry(Event const &, FSM &)
            {
                std::cout << "entering: PlayingState" << std::endl;
            }

            template <class Event, class FSM>
            void on_exit(Event const &, FSM &)
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
        void start_playback_Act(playEvent const &)
        {
            std::cout << "player::start_playback_Act\n";
        }
        void open_drawer_Act(open_close_Event const &)
        {
            std::cout << "player::open_drawer_Act\n";
        }
        void close_drawer_Act(open_close_Event const &)
        {
            std::cout << "player::close_drawer_Act\n";
        }

        void store_cd_info_Act(cd_detected_Event const &)
        {
            std::cout << "player::store_cd_info_Act\n";
        }

        void stop_playback_Act(stopEvent const &)
        {
            std::cout << "player::stop_playback_Act\n";
        }
        void pause_playback_Act(pauseEvent const &)
        {
            std::cout << "player::pause_playback_Act\n";
        }
        void resume_playback_Act(end_pause_Event const &)
        {
            std::cout << "player::resume_playback_Act\n";
        }

        void stop_and_open_Act(open_close_Event const &)
        {
            std::cout << "player::stop_and_open_Act\n";
        }
        void stopped_again_Act(stopEvent const &)
        {
            std::cout << "player::stopped_again_Act\n";
        }

        // 守护条件  guard conditions
        bool good_disk_format_Act(cd_detected_Event const &evt)
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
        bool auto_start_Act(cd_detected_Event const &)
        {
            return false;
        }

        typedef playerSM p; // makes transition table cleaner

        // 转换表 Transition table for player
        // 守卫和动作是可调用的对象，将由状态机执行，以验证转换后的操作是否应该发生
        // row takes 5 arguments: start state, event, target state, action and guard.
        // • a_row (“a” for action) allows defining only the action and omit the guard condition.
        // • g_row (“g” for guard) allows omitting the action behavior and defining only the guard.
        // • _row allows omitting action and guard.
        // The transition table is actually a MPL vector (or list), which brings the limitation that the default maximum size of the table is 20
        struct transition_table : mpl::vector<
                    //    Start          Event                Next           Action				 Guard
                    //  +-------------+--------------------+-------------+---------------------+----------------------+
                  a_row < StoppedState , playEvent        ,  PlayingState ,  &p::start_playback_Act                         >,
                  a_row < StoppedState , open_close_Event , OpenState,       &p::open_drawer_Act                            >,
                   _row < StoppedState , stopEvent        , StoppedState                                              >,
                    //  +---------+-------------+---------+---------------------+----------------------+
                  a_row < OpenState    , open_close_Event  , EmptyStat   ,   &p::close_drawer_Act                           >,
                    //  +---------+-------------+---------+---------------------+----------------------+
                  a_row < EmptyStat   , open_close_Event  , OpenState    ,   &p::open_drawer_Act                            >,
                    row < EmptyStat   , cd_detected_Event , StoppedState ,   &p::store_cd_info_Act   , &p::good_disk_format_Act  >,
                    row < EmptyStat   , cd_detected_Event , PlayingState ,   &p::store_cd_info_Act   , &p::auto_start_Act        >,

                    a_row < EmptyStat   , playEvent , PlayingState ,         &p::start_playback_Act                             >,

                    //  +---------+-------------+---------+---------------------+----------------------+
                  a_row < PlayingState ,     stopEvent     ,  StoppedState ,  &p::stop_playback_Act                          >,
                  a_row < PlayingState ,     pauseEvent    ,  Pausedtate  ,   &p::pause_playback_Act                         >,
                  a_row < PlayingState ,     open_close_Event , OpenState  ,  &p::stop_and_open_Act                          >,
                    //  +---------+-------------+---------+---------------------+----------------------+
                  a_row < Pausedtate  ,     end_pause_Event   , PlayingState , &p::resume_playback_Act                        >,
                  a_row < Pausedtate  ,     stopEvent         , StoppedState , &p::stop_playback_Act                          >,
                  a_row < Pausedtate  ,     open_close_Event  , OpenState    , &p::stop_and_open_Act                          >
                    //  +---------+-------------+---------+---------------------+----------------------+
                > {};

        // Replaces the default no-transition response.
        template <class FSM, class Event>
        void no_transition(Event const &e, FSM &, int state)
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
    static char const *const state_names[] = 
            {"StoppedState", "OpenState", "EmptyState", "PlayingState", "PausedState"};

    void printState(playerStateMachine const &p)
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