// Copyright 2010 Christophe Henry
// henry UNDERSCORE christophe AT hotmail DOT com
// This is an extended version of the state machine available in the boost::mpl library
// Distributed under the same license as the original.
// Copyright for the original version:
// Copyright 2005 David Abrahams and Aleksey Gurtovoy. Distributed
// under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


// 教程 https://www.boost.org/doc/libs/1_78_0/libs/msm/doc/HTML/index.html

#include <iostream>
// back-end
#include <boost/msm/back/state_machine.hpp>
//front-end
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/row2.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

namespace
{
    //////////////////////////////////////////////////////////////////////
    // events
    struct playEvent {};
    struct end_pauseEvent {};
    struct stopEvent {};
    struct pauseEvent {};
    struct open_closeEvent {};

    // A "complicated" event type that carries some data.
    enum DiskTypeEnum
    {
        DISK_CD=0,
        DISK_DVD=1
    };
    struct cd_detected
    {
        cd_detected(std::string name, DiskTypeEnum diskType)
            : name(name),
            disc_type(diskType)
        {}

        std::string name;
        DiskTypeEnum disc_type;
    };

    //////////////////////////////////////////////////////////////////////
    // front-end: define the FSM structure 
    struct playerSM : public msm::front::state_machine_def<playerSM>
    {
        template <class Event,class FSM>
        void on_entry(Event const& ,FSM&) 
        {
            std::cout << "entering: Player  playerSM" << std::endl;
        }

        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) 
        {
            std::cout << "leaving: Player playerSM" << std::endl;
        }

        // The list of FSM states
        struct EmptyState : public msm::front::state<> 
        {
            // every (optional) entry/exit methods get the event passed.
            template <class Event,class FSM>
            void on_entry(Event const&,FSM& ) {std::cout << "entering: EmptyState" << std::endl;}
            template <class Event,class FSM>
            void on_exit(Event const&,FSM& ) {std::cout << "leaving: EmptyState" << std::endl;}
            void open_drawer(open_closeEvent const&)    { std::cout << "EmptyState::open_drawer\n"; }
            // actions for EmptyState's internal transitions
            void internal_action(cd_detected const&){ std::cout << "EmptyState::internal action\n"; }
            bool internal_guard(cd_detected const&)
            {
                std::cout << "EmptyState::internal guard\n";
                return false;
            }
        };
        struct OpenState : public msm::front::state<> 
        { 
            template <class Event,class FSM>
            void on_entry(Event const& ,FSM&) {std::cout << "entering: OpenState" << std::endl;}
            template <class Event,class FSM>
            void on_exit(Event const&,FSM& ) {std::cout << "leaving: OpenState" << std::endl;}
            void close_drawer(open_closeEvent const&)   { std::cout << "OpenState::close_drawer\n"; }
            void stop_and_open(open_closeEvent const&)  { std::cout << "OpenState::stop_and_open\n"; }
        };

        // sm_ptr still supported but deprecated as functors are a much better way to do the same thing
        struct StoppedState : public msm::front::state<msm::front::default_base_state,msm::front::sm_ptr> 
        { 
            template <class Event,class FSM>
            void on_entry(Event const& ,FSM&) {std::cout << "entering: StoppedState" << std::endl;}
            template <class Event,class FSM>
            void on_exit(Event const&,FSM& ) {std::cout << "leaving: StoppedState" << std::endl;}
            void set_sm_ptr(playerSM* pl)
            {
                m_player=pl;
            }
            playerSM* m_player;
            void start_playback(playEvent const&)       { std::cout << "StoppedState::start_playback\n"; }
            void stop_playback(stopEvent const&)        { std::cout << "StoppedState::stop_playback\n"; }
        };

        struct PlayingState : public msm::front::state<>
        {
            template <class Event,class FSM>
            void on_entry(Event const&,FSM& ) {std::cout << "entering: PlayingState" << std::endl;}
            template <class Event,class FSM>
            void on_exit(Event const&,FSM& ) {std::cout << "leaving: PlayingState" << std::endl;}
            // guard conditions
            // used to show a transition conflict. This guard will simply deactivate one transition and thus
            // solve the conflict
            bool auto_start(cd_detected const&)
            {
                return false;
            }
        };

        // state not defining any entry or exit
        struct PausedState : public msm::front::state<>
        {
            void pause_playback(pauseEvent const&)      { std::cout << "PausedState::pause_playback\n"; }
            void resume_playback(end_pauseEvent const&)      { std::cout << "PausedState::resume_playback\n"; }
        };

        //////////////////////////////////////////////////////////////////////
        // action
        void store_cd_infoAct(cd_detected const&) { std::cout << "Player::store_cd_infoAct\n"; }

        //////////////////////////////////////////////////////////////////////
        // guard
        bool good_disk_formatGuard(cd_detected const& evt)
        {
            // to test a guard condition, let's say we understand only CDs, not DVD
            if (evt.disc_type != DISK_CD)
            {
                std::cout << "!= DISK_CD wrong disk, sorry false" << std::endl;
                return false;
            }
            std::cout << "== DISK_CD   true" << std::endl;
            return true;
        }

        // the initial state of the playerStateMachine SM. Must be defined
        typedef EmptyState initial_state;

        // Transition table for playerStateMachine
        struct transition_table : mpl::vector<
            //    Start             Event               Next        Action/Guard
            //  +---------------+-------------+-----------------+---------------------+----------------------+
         a_row2 < StoppedState , playEvent        , PlayingState , StoppedState , &StoppedState::start_playback         >,
         a_row2 < StoppedState , open_closeEvent  , OpenState    , EmptyState   , &EmptyState::open_drawer              >,
           _row < StoppedState , stopEvent        , StoppedState                                              >,
            //  +---------+-------------+---------+---------------------+----------------------+
         a_row2 < OpenState    , open_closeEvent  , EmptyState   , OpenState    , &OpenState::close_drawer              >,
            //  +---------+-------------+---------+---------------------+----------------------+
         a_row2 < EmptyState   , open_closeEvent  , OpenState    , EmptyState  ,  &EmptyState::open_drawer               >,
           row2 < EmptyState   , cd_detected ,      StoppedState , playerSM ,     &playerSM::store_cd_infoAct   
                                                  , playerSM ,     &playerSM::good_disk_formatGuard        >,
           row2 < EmptyState   , cd_detected ,      PlayingState , playerSM ,     &playerSM::store_cd_infoAct   
                                                  , PlayingState , &PlayingState::auto_start              >,
         // conflict with some internal rows
          irow2 < EmptyState   , cd_detected      , EmptyState   , &EmptyState::internal_action 
                                                  , EmptyState   , &EmptyState::internal_guard            >,
        g_irow2 < EmptyState   , cd_detected      , EmptyState   , &EmptyState::internal_guard           >,
            //  +---------+-------------+---------+---------------------+----------------------+
         a_row2 < PlayingState , stopEvent        , StoppedState , StoppedState ,&StoppedState::stop_playback           >,
         a_row2 < PlayingState , pauseEvent       , PausedState  , PausedState  ,&PausedState::pause_playback           >,
         a_row2 < PlayingState , open_closeEvent  , OpenState    , OpenState    ,&OpenState::stop_and_open              >,
            //  +---------+-------------+---------+---------------------+----------------------+
         a_row2 < PausedState  , end_pauseEvent   , PlayingState , PausedState  ,&PausedState::resume_playback          >,
         a_row2 < PausedState  , stopEvent        , StoppedState , StoppedState ,&StoppedState::stop_playback           >,
         a_row2 < PausedState  , open_closeEvent  , OpenState    , OpenState    ,&OpenState::stop_and_open              >
            //  +---------+-------------+---------+---------------------+----------------------+
        > {};

        // Replaces the default no-transition response.
        template <class FSM,class Event>
        void no_transition(Event const& e, FSM&,int state)
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
    static char const* const state_names[] = { "StoppedState", "OpenState", "EmptyState", "PlayingState", "PausedState" };
    void printState(playerStateMachine const& p)
    {
        std::cout << " current state is -----> " << state_names[p.current_state()[0]] << std::endl;
    }

    void test()
    {        
        playerStateMachine p;

        // needed to start the highest-level SM. This will call on_entry and mark the start of the SM
        p.start();  // entering: Player        entering: EmptyState

        // go to OpenState, call on_exit on EmptyState, then action, then on_entry on OpenState
        p.process_event(open_closeEvent());     // leaving: EmptyState     EmptyState::open_drawer     entering: OpenState
        printState(p);  //  current state is -----> OpenState

        p.process_event(open_closeEvent());     // leaving: OpenState      OpenState::close_drawer     entering: EmptyState
        printState(p);  // current state is -----> EmptyState

        // will be rejected, wrong disk type
        p.process_event(cd_detected("aa, bb", DISK_DVD));    // EmptyState::internal guard    EmptyState::internal guard
        printState(p);  //  current state is -----> EmptyState

        p.process_event(cd_detected("cc, dd", DISK_CD));    // EmptyState::internal guard       EmptyState::internal guard      
        printState(p);  //  current state is -----> StoppedState

        p.process_event(playEvent());

        // at this point, Play is active      
        p.process_event(pauseEvent()); 
        printState(p);

        // go back to PlayingState
        p.process_event(end_pauseEvent());  
        printState(p);

        p.process_event(pauseEvent()); 
        printState(p);

        p.process_event(stopEvent());  
        printState(p);

        // event leading to the same state
        // no action method called as it is not present in the transition table
        p.process_event(stopEvent());  
        printState(p);

        std::cout << "stop fsm" << std::endl;

        p.stop();

    }
}

int main()
{
    test();
    return 0;
}
