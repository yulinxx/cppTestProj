#ifndef IMPLEMENTATION_H
#define IMPLEMENTATION_H

#include "Interface.h"

class EXPORT_API Implementation : public Interface 
{
public:
    virtual void foo() override;
    virtual void bar() override;

public:
    double add(double a, double b);
    double sub(double a, double b);
    double mul(double a, double b);
    double div(double a, double b);
};

#endif  // IMPLEMENTATION_H