#include "Context.h"
#include "State.h"
 
namespace HsmState
{
    /**
     * @brief Context 类的构造函数
     * 
     * 初始化 Context 对象，目前为空实现
     */
    Context::Context()
    {
        // 此处可以添加初始化代码
    }
    
    /**
     * @brief Context 类的析构函数
     * 
     * 释放 Context 对象管理的所有状态对象，并清空状态映射表
     */
    Context::~Context()
    {
        // 遍历状态映射表
        for (auto iter : m_mapStrStates)
        {
            // 检查状态对象是否存在
            if (iter.second.m_state)
            {
                // 释放状态对象
                delete iter.second.m_state;
                // 将指针置为 nullptr，避免悬空指针
                iter.second.m_state = nullptr;
            }
        }
        // 清空状态映射表
        m_mapStrStates.clear();
    }
    
    /**
     * @brief 启动状态机
     * 
     * 根据给定的状态名称启动状态机，并将该状态设置为当前状态
     * 
     * @param name 要启动的状态名称
     * @return bool 如果找到并启动状态，返回 true；否则返回 false
     */
    bool Context::Start(std::string name)
    {
        // 在状态映射表中查找指定名称的状态
        std::unordered_map<std::string, NodeState>::iterator iter_map = m_mapStrStates.find(name);
        // 检查是否找到该状态
        if (iter_map != m_mapStrStates.end())
        {
            // 设置当前状态
            m_curNodeState = iter_map->second;
            // 设置当前状态名称
            m_strCurName = iter_map->first;
            // 启动当前状态
            iter_map->second.m_state->start();
            return true;
        }
        return false;
    }
    
    /**
     * @brief 创建一个新的状态
     * 
     * 创建一个新的状态对象，并将其添加到状态映射表中
     * 
     * @param state 状态对象指针，在 Context 销毁时，内部会释放该对象
     * @param name 状态名称，若为空则使用类型名称
     * @param father_name 父状态的名称
     * @return State* 返回创建的状态对象指针
     */
    State* Context::CreateState(State* state, std::string name, std::string father_name)
    {
        // 创建一个新的节点状态对象
        NodeState node_state;
        // 设置节点状态的状态对象
        node_state.m_state = state;
        // 设置状态对象的上下文为当前 Context 对象
        node_state.m_state->SetContext(this);
        // 设置节点状态的父状态名称
        node_state.m_father_name = father_name;
        // 将节点状态添加到状态映射表中
        m_mapStrStates[name] = node_state;
        // 返回创建的状态对象指针
        return state;
    }
    
    /**
     * @brief 更新当前状态
     * 
     * 调用当前状态的更新函数
     */
    void Context::Update()
    {
        // 调用当前状态的更新函数
        m_curNodeState.m_state->update();
    }
    
    /**
     * @brief 同步发送事件
     * 
     * 发送一个事件，提供给根状态和当前状态处理，如果当前状态是子状态，则还会给父状态处理
     * 
     * @param event_data 要发送的事件数据
     */
    void Context::SendEvent(EventData event_data)
    {
        // 递归发送事件
        RecursiveSend(m_curNodeState, event_data);
    }
    
    /**
     * @brief 异步发送事件
     * 
     * 异步发送一个事件，目前待实现
     * 
     * @param event_data 要发送的事件数据
     */
    void Context::SendAsyncEvent(EventData event_data)
    {
        // todo 待实现
    }
    
    /**
     * @brief 获取当前状态的名称
     * 
     * 返回当前状态的名称
     * 
     * @return std::string 当前状态的名称
     */
    std::string Context::GetCurStateName()
    {
        // 返回当前状态的名称
        return m_strCurName;
    }
    
    /**
     * @brief 递归发送事件
     * 
     * 递归地将事件发送给当前状态及其父状态，直到事件被处理或没有父状态为止
     * 
     * @param node_state 当前节点状态
     * @param event_data 要发送的事件数据
     */
    void Context::RecursiveSend(NodeState& node_state, EventData& event_data)
    {
        // 调用当前状态的事件处理函数
        EventDeal event_deal = node_state.m_state->RunEventFunc(event_data);
        // 检查事件处理结果是否为继续处理，并且当前状态有父状态
        if (event_deal == keep_on
            && !node_state.m_father_name.empty())
        {
            // 在状态映射表中查找父状态
            std::unordered_map<std::string, NodeState>::iterator iter_map = m_mapStrStates.find(node_state.m_father_name);
            // 检查是否找到父状态
            if (iter_map != m_mapStrStates.end())
            {
                // 递归发送事件给父状态
                RecursiveSend(iter_map->second, event_data);
            }
        }
    }
    
    /**
     * @brief 状态转移
     * 
     * 将当前状态转移到指定名称的状态
     * 
     * @param name 要转移到的状态名称
     */
    void Context::TransForState(std::string name)
    {
        // 复制状态名称
        std::string str_name = std::string(name);
        // 在状态映射表中查找指定名称的状态
        std::unordered_map<std::string, NodeState>::iterator iter_map = m_mapStrStates.find(str_name);
        // 检查是否找到该状态
        if (iter_map != m_mapStrStates.end())
        {
            // 停止当前状态
            m_curNodeState.m_state->stop();
            // 设置新的当前状态
            m_curNodeState = iter_map->second;
            // 设置新的当前状态名称
            m_strCurName = iter_map->first;
            // 启动新的当前状态
            m_curNodeState.m_state->start();
        }
    }
}