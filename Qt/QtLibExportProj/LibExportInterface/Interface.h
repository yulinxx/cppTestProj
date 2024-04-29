#ifndef INTERFACE_H
#define INTERFACE_H

#include "LibExports.h"

class EXPORT_API Interface 
{
public:
    Interface() {}
    virtual ~Interface() {}

public:
    virtual void foo();
    virtual void bar();    // 添加其他纯虚函数或者虚函数作为接口
    
};

#endif // INTERFACE_H