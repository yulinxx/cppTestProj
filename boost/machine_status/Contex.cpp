#include "Context.h"
#include "State.h"
 
namespace HsmState
{
	Context::Context()
	{
 
	}
 
	Context::~Context()
	{
		for (auto iter : m_mapStrStates)
		{
			if (iter.second.m_state)
			{
				delete iter.second.m_state;
				iter.second.m_state = nullptr;
			}
		}
		m_mapStrStates.clear();
	}
 
	// 开始状态机
	bool Context::Start(std::string name)
	{
		std::unordered_map<std::string, NodeState>::iterator iter_map = m_mapStrStates.find(name);
		if (iter_map != m_mapStrStates.end())
		{
			m_curNodeState = iter_map->second;
			m_strCurName = iter_map->first;
			iter_map->second.m_state->start();
		}
		return false;
	}
 
	// 创建一个状态
	// [in] state 状态对象，在Context销毁时，内部释放state
	// [in] name  状态名称，为空名称为typedname的值
	// [in] father_name 父状态的名称
	// [out] 返回state
	State* Context::CreateState(State* state, std::string name, std::string father_name)
	{
		NodeState node_state;
		node_state.m_state = state;
		node_state.m_state->SetContext(this);
		node_state.m_father_name = father_name;
		m_mapStrStates[name] = node_state;
		return state;
	}
 
	// 更新当前状态
	void Context::Update()
	{
		m_curNodeState.m_state->update();
	}
 
	// 同步事件
	// 发送一个事件，提供给root状态和当前状态处理
	// 如果当前状态是子状态，则还会给父状态处理
	void Context::SendEvent(EventData event_data)
	{
		RecursiveSend(m_curNodeState, event_data);
	}
 
	// 异步事件
	void Context::SendAsyncEvent(EventData event_data)
	{
		// todo 待实现
	}
 
	std::string Context::GetCurStateName()
	{
		return m_strCurName;
	}
 
	// 递归send
	void Context::RecursiveSend(NodeState& node_state, EventData& event_data)
	{
		EventDeal event_deal = node_state.m_state->RunEventFunc(event_data);
		if (event_deal == keep_on
			&& !node_state.m_father_name.empty())
		{
			std::unordered_map<std::string, NodeState>::iterator iter_map = m_mapStrStates.find(node_state.m_father_name);
			if (iter_map != m_mapStrStates.end())
			{
				RecursiveSend(iter_map->second, event_data);
			}
		}
	}
 
	void Context::TransForState(std::string name)
	{
		std::string str_name = std::string(name);
		std::unordered_map<std::string, NodeState>::iterator iter_map = m_mapStrStates.find(str_name);
		if (iter_map != m_mapStrStates.end())
		{
			// 停止上一个状态
			m_curNodeState.m_state->stop();
 
			// 初始化下一个状态
			m_curNodeState = iter_map->second;
			m_strCurName = iter_map->first;
			m_curNodeState.m_state->start();
		}
	}
}