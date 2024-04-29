#ifndef IMPLEMENTATION_H
#define IMPLEMENTATION_H

#include "Interface.h"

class EXPORT_API Implementation : public Interface 
{
public:
    virtual void foo() override;
    virtual void bar() override;
};

#endif  // IMPLEMENTATION_H