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


///////////////////////////////////
struct testStruct
{
    int n = 0;
    float d = 0.; 
};

void testFunc(testStruct* pS)
{
    std::cout << pS->n << "\t" << pS->d << std::endl;   // 0	0

    // 指针地址             指针地址            指针地址的地址      指针地址
    // 0000023A5C671190	0000023A5C671190	000000CAB76FFBC0	0000023A5C671190
    std::cout << pS << "\t" << (pS) << "\t" << &pS << "\t"
              << static_cast<void *>(pS) << std::endl;
}


///////////////////////////////////
// char  string 转换
void charStrSwitch()
{
    unsigned char myString[] = "This is my string\n12345";  // 24
    const unsigned char*  pOri = myString;
    std::cout << pOri << std::endl;

    std::string strData = (char *)const_cast<unsigned char *>(pOri);

    const unsigned char * resData = (const unsigned char *)strData.data();

    std::cout << resData << std::endl;
}

///////////////////////////////////
// 位的与或非
void bitTest()
{
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
}









///////////////////////////////////
int main()
{
    // This is my string
    // 12345
    // This is my string
    // 12345
    charStrSwitch();    

    testStruct* p = new testStruct();
    testFunc(p);

    bitTest();

    int n = -17 % 8;    // -1

    // 向量叉乘 判断两条共点的线的方向
    double dx1 = 10.0;
    double dy1 = 0.0;

    double dx2 = 5.0;
    double dy2 = 0.0;

    double dx3 = 5.0;
    double dy3 = 5.0;

    double dxv1 = dx2 - dx1;    // -5.0000000000000000
    double dyv1 = dy2 - dy1;    // 0.0000000000000000

    double dxv2 = dx3 - dx2;    // 0.0000000000000000
    double dyv2 = dy3 - dy2;    // 5.0000000000000000

    double dRes = dxv1 * dyv2 - dyv1 * dxv2;    // -25.000000000000000

    Objx<int, 4> xa;
    Objx<char, 2> xb;

    ObjA<int> objA;
    ObjB<int, 5> objB;

    // testTemp();
    

    return 0;
}


/*

This is my string
12345
This is my string
12345
0       0
00000183D8DC32D0        00000183D8DC32D0        0000004C2CF3F6F0        00000183D8DC32D0
aaaaaaaaaaaobjx aaaaaaa  
objx bbbbbbbb  

*/