
#include "Context.h"
#include "State.h"
 
namespace HsmState
{
	// 设置时间响应回调函数
	void State::set_event_func(std::function<EventDeal(EventData&)> func)
	{
		m_event_func = func;
	}
 
	EventDeal State::RunEventFunc(EventData& event_data)
	{
		if (m_event_func == nullptr)
			return keep_on;
		return m_event_func(event_data);
	}
 
	void State::SetContext(Context* context)
	{
		m_context = context;
	}
 
	void State::TransState(std::string name)
	{
		m_context->TransForState(name);
	}
}