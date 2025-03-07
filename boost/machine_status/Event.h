// 防止头文件被重复包含，这是一种常见的预处理技巧
// 当第一次包含该头文件时，_EVENT_H_ 未被定义，所以下面的代码会被执行
// 并定义 _EVENT_H_，之后再次包含该头文件时，由于 _EVENT_H_ 已被定义，下面的代码将被跳过
#ifndef _EVENT_H_
#define _EVENT_H_

// 定义一个命名空间 HsmState，用于封装相关的类和枚举，避免命名冲突
namespace HsmState
{
    // 定义一个枚举类型 EventDeal，用于表示事件处理的状态
    enum  EventDeal
    {
        // 表示事件结束处理，当处理结果为 tail 时，事件处理流程结束
        tail = 0,   
        // 表示事件需要继续传递，当处理结果为 keep_on 时，事件会继续传递给其他处理者
        keep_on 
    };
 
    // 定义一个类 EventData，用于存储事件相关的数据
    // 事件数据包含事件类型和一个指向具体数据的指针
    class EventData
    {
    public:
        // 构造函数，接收一个整数类型的事件类型作为参数
        // 初始化成员变量 m_nEventType 为传入的事件类型，m_pData 初始化为 nullptr
        EventData(int event_type)
        {
            m_nEventType = event_type;
            m_pData = nullptr;
        }
 
        // 模板函数，用于设置事件数据
        // 接收一个指向任意类型 T 的指针 t，并将其赋值给成员变量 m_pData
        template <class T>
        void SetData(T* t)
        {
            m_pData = t;
        }
 
        // 模板函数，用于获取事件数据
        // 返回一个指向任意类型 T 的指针，将成员变量 m_pData 强制转换为 T* 类型
        template <class T>
        T* GetData()
        {
            return (T*)m_pData;
        }
 
        // 成员变量，用于存储事件类型
        int m_nEventType;

    private:
        // 成员变量，用于存储指向具体数据的指针
        // 由于使用了 void* 类型，所以可以存储任意类型的指针
        void* m_pData;
    };
}

// 结束 #ifndef _EVENT_H_ 的条件编译块
#endif // _EVENT_H_