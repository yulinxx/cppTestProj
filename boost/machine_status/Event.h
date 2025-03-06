#ifndef _EVENT_H_
#define _EVENT_H_

namespace HsmState
{
	enum  EventDeal
	{
		tail = 0,   // 事件结束处理
		keep_on // 事件继续传递
	};
 
	// 事件数据
	class EventData
	{
	public:
		EventData(int event_type)
		{
			m_nEventType = event_type;
			m_pData = nullptr;
		}
 
		template <class T>
		void SetData(T* t)
		{
			m_pData = t;
		}
 
		template <class T>
		T* GetData()
		{
			return (T*)m_pData;
		}
 
		int m_nEventType;

	private:
		void* m_pData;
	};
}

#endif // _EVENT_H_