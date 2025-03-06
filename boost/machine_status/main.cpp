
#include "Context.h"
#include "State.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
 
using namespace HsmState;
bool g_run = true;
 
enum EventS
{
	belazy = 0, // 偷懒事件
};
 
 ////////////////////////////////////////
// 开始状态
class StartState : public State
{
public :
	void start()
	{
		std::cout << "StartState start" << std::endl;
	}
 
	void stop()
	{
		std::cout << "StartState stop" << std::endl;
	}
 
	void update()
	{
		time++;
		if (time == 10)
		{
			TransState("HungerState");
		}
	}
 
	int time = 0;
};
 
 ////////////////////////////////////////
// 饥饿状态
class HungerState : public State
{
public:
	void start()
	{
		std::cout << "HungerState start" << std::endl;
	}
 
	void stop()
	{
		std::cout << "HungerState stop" << std::endl;
	}
 
	void update()
	{
		time++;
		if (time == 10)
		{
			TransState("Dinner");
		}
	}
 
	int time = 0;
};
 
 ////////////////////////////////////////
 // 晚餐
class Dinner : public State
{
public:
	void start()
	{
		std::cout << "Dinner start" << std::endl;
	}
 
	void stop()
	{
		std::cout << "Dinner stop" << std::endl;
	}
 
	void update()
	{
		TransState("DoTheCookingState");
	}
};
 
 ////////////////////////////////////////
// 做饭状态
class DoTheCookingState : public State
{
public:
	void start()
	{
		std::cout << "DoTheCookingState start" << std::endl;
	}
 
	void stop()
	{
		std::cout << "DoTheCookingState stop" << std::endl;
	}
 
	void update()
	{
		time++;
		if (time == 60)
		{
			TransState("EatState");
		}
	}
 
	int time = 0;
};
 
 ////////////////////////////////////////
// 吃饭状态
class EatState : public State
{
public:
	void start()
	{
		std::cout << "EatState start" << std::endl;
	}
 
	void stop()
	{
		std::cout << "EatState stop" << std::endl;
	}
 
	void update()
	{
		time++;
		if (time == 5)
		{
			TransState("SleepState");
		}
	}
 
	int time = 0;
};
 
 
// 饭后睡觉状态
class SleepState : public State
{
public:
	void start()
	{
		std::cout << "SleepState start" << std::endl;
	}
 
	void stop()
	{
		std::cout << "SleepState stop" << std::endl;
	}
 
	void update()
	{
		time++;
		if (time == 30)
		{
			TransState("WorkState");
		}
	}
 
	int time = 0;
};
 
// 工作状态
class WorkState : public State
{
public:
	void start()
	{
		std::cout << "WorkState start" << std::endl;
 
		std::function<EventDeal(EventData&)> func = 
			std::bind(&WorkState::DealEvent, this, std::placeholders::_1);
		set_event_func(func);
	}
 
	void stop()
	{
		std::cout << "WorkState stop" << std::endl;
	}
 
	EventDeal DealEvent(EventData &event_data)
	{
		switch ((EventS)event_data.m_nEventType)
		{
		case belazy:
			TransState("LoafOnAJob");
			break;
		default:
			break;
		}
		return keep_on;
	}
 
	void update()
	{
		time++;
		if (time == 180)
		{
			g_run = false;
		}
	}
 
	int time = 0;
};
 
// 工作摸鱼状态
class LoafOnAJob : public State
{
public:
	void start()
	{
		time = 0;
		std::cout << "LoafOnAJob start" << std::endl;
	}
 
	void stop()
	{
		std::cout << "LoafOnAJob stop" << std::endl;
	}
 
	void update()
	{
		time++;
		if (time == 10)
		{
			TransState("WorkState");
		}
	}
 
	int time = 0;
};

// 对象工厂
class Factory
{
public :
	static State* CreateState(Context* context, std::string name, std::string parent_name = "")
	{
		State* state = nullptr;
		if (name == "StartState")
		{
			state = new StartState();
		}
		else if (name == "HungerState")
		{
			state = new HungerState();
		}
		else if (name == "Dinner")
		{
			state = new Dinner();
		}
		else if (name == "DoTheCookingState")
		{
			state = new DoTheCookingState();
		}
		else if (name == "EatState")
		{
			state = new EatState();
		}
		else if (name == "SleepState")
		{
			state = new SleepState();
		}
		else if (name == "WorkState")
		{
			state = new WorkState();
		}
		else if (name == "LoafOnAJob")
		{
			state = new LoafOnAJob();
		}
 
		context->CreateState(state, name, parent_name);
		return state;
	}
};
 
 
 
int main()
{
	Context* context = new Context();
 
	// 创建状态机
	Factory::CreateState(context, "StartState");
	Factory::CreateState(context, "HungerState");
	Factory::CreateState(context, "Dinner");
	Factory::CreateState(context, "DoTheCookingState", "Dinner");
	Factory::CreateState(context, "EatState", "Dinner");
	Factory::CreateState(context, "SleepState");
	Factory::CreateState(context, "WorkState");
	Factory::CreateState(context, "LoafOnAJob");
 
	// 开始状态机
	context->Start("StartState");
 
	int time = 0;
	while (g_run)
	{
		time++;
		std::this_thread::sleep_for(
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::milliseconds(10)));
 
		context->Update();
 
		// 如果为工作状态，每隔60分钟发出偷懒事件
		if (context->GetCurStateName() == "WorkState" 
			&& time % 60 == 0) 
		{
			EventData e = EventData((int)belazy);
			context->SendEvent(e);
		}
	}
 
	if (context)
	{
		delete context;
		context = nullptr;
	}
 
	std::cout << "state close" << std::endl;
	return 0;
}