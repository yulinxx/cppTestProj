// Copyright 2010 Christophe Henry
// henry UNDERSCORE christophe AT hotmail DOT com
// This is an extended version of the state machine available in the boost::mpl library
// Distributed under the same license as the original.
// Copyright for the original version:
// Copyright 2005 David Abrahams and Aleksey Gurtovoy. Distributed
// under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <iostream>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/euml/euml.hpp>

using namespace std;
using namespace boost::msm::front::euml;
namespace msm = boost::msm;

// entry/exit/action/guard logging functors
#include "logging_functors.h"

namespace  // Concrete FSM implementation
{
    // events
    BOOST_MSM_EUML_EVENT(play)
        BOOST_MSM_EUML_EVENT(end_pause)
        BOOST_MSM_EUML_EVENT(stop)
        BOOST_MSM_EUML_EVENT(pause)
        BOOST_MSM_EUML_EVENT(open_close)

        // A "complicated" event type that carries some data.
        BOOST_MSM_EUML_DECLARE_ATTRIBUTE(std::string, cd_name)
        BOOST_MSM_EUML_DECLARE_ATTRIBUTE(DiskTypeEnum, cd_type)
        BOOST_MSM_EUML_ATTRIBUTES((attributes_ << cd_name << cd_type), cd_detected_attributes)
        BOOST_MSM_EUML_EVENT_WITH_ATTRIBUTES(cd_detected, cd_detected_attributes)

        // Concrete FSM implementation
        // The list of FSM states
        // state not needing any entry or exit
        BOOST_MSM_EUML_STATE((), Paused)

        // it is also possible to define a state which you can implement normally
        // just make it a state, as usual, and also a grammar terminal, euml_state
        struct Empty_impl : public msm::front::state<>, public euml_state<Empty_impl>
    {
        // this allows us to add some functions
        void activate_empty()
        {
            std::cout << "switching to Empty " << std::endl;
        }
        // standard entry behavior
        template <class Event, class FSM>
        void on_entry(Event const& evt, FSM& fsm)
        {
            std::cout << "entering: Empty" << std::endl;
        }
        template <class Event, class FSM>
        void on_exit(Event const& evt, FSM& fsm)
        {
            std::cout << "leaving: Empty" << std::endl;
        }
    };
    //instance for use in the transition table
    Empty_impl const Empty;

    // create a functor and a eUML function for the activate_empty method from Entry
    BOOST_MSM_EUML_METHOD(ActivateEmpty_, activate_empty, activate_empty_, void, void)

        // define more states
        BOOST_MSM_EUML_STATE((Open_Entry, Open_Exit), Open)
        BOOST_MSM_EUML_STATE((Stopped_Entry, Stopped_Exit), Stopped)
        BOOST_MSM_EUML_STATE((Playing_Entry, Playing_Exit), Playing)

        // guard conditions
        BOOST_MSM_EUML_ACTION(good_disk_format)
    {
        template <class FSM, class EVT, class SourceState, class TargetState>
        bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&)
        {
            // to test a guard condition, let's say we understand only CDs, not DVD
            if (evt.get_attribute(cd_type) != DISK_CD)
            {
                std::cout << "wrong disk, sorry" << std::endl;
                // just for logging, does not block any transition
                return true;
            }
            std::cout << "good disk" << std::endl;
            return true;
        }
    };
    // it is also possible to use a plain functor, with default-constructor in the transition table
    struct start_play
    {
        template <class FSM, class EVT, class SourceState, class TargetState>
        void operator()(EVT const&, FSM&, SourceState&, TargetState&)
        {
            cout << "player::start_play" << endl;
        }
    };
    // replaces the old transition table
    BOOST_MSM_EUML_TRANSITION_TABLE((
        Playing == Stopped + play / start_play(),
        Playing == Paused + end_pause / resume_playback,
        //  +------------------------------------------------------------------------------+
        Empty == Open + open_close / (close_drawer, activate_empty_(target_)),
        //  +------------------------------------------------------------------------------+
        Open == Empty + open_close / open_drawer,
        Open == Paused + open_close / stop_and_open,
        Open == Stopped + open_close / open_drawer,
        Open == Playing + open_close / stop_and_open,
        //  +------------------------------------------------------------------------------+
        Paused == Playing + pause / pause_playback,
        //  +------------------------------------------------------------------------------+
        Stopped == Playing + stop / stop_playback,
        Stopped == Paused + stop / stop_playback,
        Stopped == Empty + cd_detected[good_disk_format &&
        (event_(cd_type) == Int_<DISK_CD>())]
        / (store_cd_info, process_(play)),
        Stopped == Stopped + stop
        //  +------------------------------------------------------------------------------+
        ), transition_table)

        // create a state machine "on the fly"
        BOOST_MSM_EUML_DECLARE_STATE_MACHINE((transition_table, //STT
            init_ << Empty, // Init State
            no_action, // Entry
            no_action, // Exit
            attributes_ << no_attributes_, // Attributes
            configure_ << no_configure_, // configuration
            Log_No_Transition // no_transition handler
            ),
            player_) //fsm name

        // or simply, if no no_transition handler needed:
        //BOOST_MSM_EUML_DECLARE_STATE_MACHINE(( transition_table, //STT
        //                                    Empty // Init State
        //                                 ),player_)

        // choice of back-end
        typedef msm::back::state_machine<player_> player;

    //
    // Testing utilities.
    //
    static char const* const state_names[] = { "Stopped", "Paused", "Open", "Empty", "Playing" };
    void pstate(player const& p)
    {
        std::cout << " -> " << state_names[p.current_state()[0]] << std::endl;
    }

    void test()
    {
        player p;
        // needed to start the highest-level SM. This will call on_entry and mark the start of the SM
        p.start();
        // note that we write open_close and not open_close(), like usual. Both are possible with eUML, but
        // you now have less to type.
        // go to Open, call on_exit on Empty, then action, then on_entry on Open
        p.process_event(open_close); pstate(p);
        p.process_event(open_close); pstate(p);
        // will be rejected, wrong disk type
        p.process_event(
            cd_detected("louie, louie", DISK_DVD)); pstate(p);
        p.process_event(
            cd_detected("louie, louie", DISK_CD)); pstate(p);
        // no need to call play as the previous event does it in its action method
        //p.process_event(play);

        // at this point, Play is active
        p.process_event(pause); pstate(p);
        // go back to Playing
        p.process_event(end_pause);  pstate(p);
        p.process_event(pause); pstate(p);
        p.process_event(stop);  pstate(p);
        // event leading to the same state
        // no action method called as none is defined in the transition table
        p.process_event(stop);  pstate(p);
        // test call to no_transition
        p.process_event(pause); pstate(p);
    }
}

int main()
{
    test();
    return 0;
}