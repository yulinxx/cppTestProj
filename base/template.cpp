/*
 * ......................................&&.........................
 * ....................................&&&..........................
 * .................................&&&&............................
 * ...............................&&&&..............................
 * .............................&&&&&&..............................
 * ...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * ..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * ................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * .......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * ...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * ..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * ...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * ..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * ..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * ........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * .......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * ........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * ..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * ................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * ..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * ..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * ...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * .........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * .......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * ......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * .....&&&&&&&&&&&&&&&&............................&&..............
 * ....&&&&&&&&&&&&&&&.................&&...........................
 * ...&&&&&&&&&&&&&&&.....................&&&&......................
 * ...&&&&&&&&&&.&&&........................&&&&&...................
 * ..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * ..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * ..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * ..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * ..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * ...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * ....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * .......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * .......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * ........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * .......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * .......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * ........&&...................&&&&&&.........................&&&..
 * .........&.....................&&&&........................&&....
 * ...............................&&&.......................&&......
 * ................................&&......................&&.......
 * .................................&&..............................
 * ..................................&..............................
 */

#include <stdio.h>

#define TESTX(x) \
    std::cout << ##x##x #x #x << std::endl;

#include <iostream>

int nFlag = 0x00;

template <typename T>
class ObjA
{
};

template <typename T, int>
class ObjB
{
};

template <typename NumType, int NType>
struct Objx
{
    Objx()
    {
        std::cout << "objx aaaaaaa  \n";
    }
};

template <typename NumType>
struct Objx<NumType, 2>
{
    Objx()
    {
        std::cout << "objx bbbbbbbb  \n";
    }
};

class ObjPtr
{
public:
    int n = 99;
    int m = 88;
};

const double eps = 1e-8;
typedef struct _InOutParam_
{
    // 基本参数 需设定
    double m_dLen;   // 长度
    double m_dAngle; // 角度

    _InOutParam_()
    {
        m_dLen = 5;
        m_dAngle = 10;
    }

    bool operator==(const _InOutParam_ &p) const
    {
        return fabs(this->m_dLen - p.m_dLen) < eps &&
               fabs(this->m_dAngle - p.m_dAngle) < eps;
    }

    bool operator!=(const _InOutParam_ &p) const
    {
        if (this == &p)
            return false;
        else
            return true;
    }

} InOutParam;

class Base
{
public:
    int a = 9;

protected:
    int b = 19;
};

class SubObj : Base
{
public:
    int c = 99;
};




struct testStruct
{
    int n = 0;
    float d = 0.; 
};

void testFunc(testStruct* pS)
{
    std::cout << pS->n << "\t" << pS->d << std::endl;

    std::cout << pS << "\t" << (pS) << "\t" << &pS << "\t"
              << static_cast<void *>(pS) << std::endl;
}


void charStrSwitch()
{
    unsigned char myString[] = "This is my string\n12345";  // 24
    const unsigned char*  pOri = myString;
    std::cout << pOri << std::endl;

    std::string strData = (char *)const_cast<unsigned char *>(pOri);

    const unsigned char * resData = (const unsigned char *)strData.data();

    std::cout << resData << std::endl;
}

int main()
{
    charStrSwitch();

    // testStruct* p = new testStruct(3, 9.9);
    testStruct* p = new testStruct();
    testFunc(p);

    // SubObj obj;
    // auto tempA = obj.a;
    // auto temp = obj.b;

    int n = -17 % 8;

    int nSize = 8;
    int nMaxPtX = 6;
    double dXPrev = (nMaxPtX - 2) < 0 ? nSize + (nMaxPtX - 2) : (nMaxPtX - 2);
    double dYPrev = (nMaxPtX - 1) < 0 ? nSize + (nMaxPtX - 1) : (nMaxPtX - 1);
    double dXNext = (nMaxPtX + 2) % nSize;
    double dYNext = (nMaxPtX + 3) % nSize;

    double dx1 = 10.0;
    double dy1 = 0.0;

    double dx2 = 5.0;
    double dy2 = 0.0;

    double dx3 = 5.0;
    double dy3 = 5.0;

    double dxv1 = dx2 - dx1;
    double dyv1 = dy2 - dy1;

    double dxv2 = dx3 - dx2;
    double dyv2 = dy3 - dy2;

    double dRes = dxv1 * dyv2 - dyv1 * dxv2;

    InOutParam a;
    a.m_dAngle = 10;

    InOutParam b;
    b.m_dAngle = 10.0001;

    if (a == b)
    {
        std::cout << " =========== ";
    }

    if (a != b)
    {
        std::cout << "aaaaaaaaaaa";
    }

    std::shared_ptr<ObjPtr> pObj = nullptr;

    if (!pObj)
    {
        // pObj = std::shared_ptr<ObjPtr>(new ObjPtr());
        pObj = std::make_shared<ObjPtr>();
    }

    if (!pObj)
    {
        // pObj = std::shared_ptr<ObjPtr>(new ObjPtr());
        pObj = std::make_shared<ObjPtr>();
    }

    int nTest = pObj->m;

    Objx<int, 4> xa;
    Objx<char, 2> xb;
    return 0;

    ObjA<int> objA;
    ObjB<int, 5> objB;

    TESTX("abcdef")

    int nFlag = 0x00;
    while (!(nFlag & 0x11))
    {
        {
            nFlag |= 0x10;

            int nTemp = nFlag & 0x11;
            bool bTest = (nFlag == 0x11);

            nFlag |= 0x01;

            bool bTest2 = (nFlag == 0x11);
            int nx = nTemp;
        }

        {
            nFlag = 0x00;
            nFlag |= 0x01;

            int nTemp2 = nFlag & 0x01;
            bool bTest = (nFlag == 0x11);

            nFlag |= 0x10;
            bool bTest2 = (nFlag == 0x11);
            int nx = nTemp2;
        }
    }

    {
        int a = nFlag & 0x01;
        nFlag |= 0x01;
        int b = nFlag & 0x01;

        int c = nFlag & 0x10;
        nFlag |= 0x10;
        int d = (nFlag & 0x10);
        int m = d;
    }
    ////////////////////////
    nFlag = 0x00;

    {

        int c = nFlag & 0x10;
        nFlag |= 0x10;
        bool d = nFlag & 0x10;

        bool a = nFlag & 0x01;
        nFlag |= 0x01;
        bool b = !(nFlag & 0x01);

        bool m = b;
    }

    return 0;
}