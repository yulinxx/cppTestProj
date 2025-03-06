// https://blog.csdn.net/qnavy123/article/details/90670054

#include <iostream>

class ObjTest
{
public:
    ObjTest(int a) : m_nMemberA(a), m_nCount_Edit(0)
    {
    }
    ~ObjTest()
    {
    }

    // c++类中 引入 const关键字 在成员函数尾部，使函数成为 常成员函数。
    // 在常成员函数内部，不允许修改调用它的对象的内部状态（即不许修改对象的成员变量的值）

    // 如果我想在常成员函数内部，修改对象的某个成员变量时，怎么办？这时，就需要 引入 mutable，
    // 将该对象的成员变量定义为 mutable 变量，但是，我们会把定义为mutable的这个变量，看做不属于对象的状态。
    // 这样，即使在常成员函数内部，mutable变量也可以被修改，与此同时，常成员函数内部，
    // 对于对象的其他成员变量（即对象的内部状态）还是不能修改的！
    void constDisplayFunc() const
    {
        m_nCount_Edit++;
        // m_nMemberA++; // ++f进行修改出错 : 表达式必须是可修改的左值C/C++(137)

        std::cout << " call func display()" << std::endl;
    }

    int constGetCountFunc() const
    {
        return m_nCount_Edit;
    }

private:
    int m_nMemberA;            // 在const 函数中,不能被修改
    mutable int m_nCount_Edit; // 在const 函数中,能被修改
};

//////////////////////////////////////

int main(int argc, char* argv[])
{
    ObjTest obj(1);
    obj.constDisplayFunc();
    obj.constDisplayFunc();

    std::cout << "call func " << obj.constGetCountFunc() << " times." << std::endl;

    return 0;
}