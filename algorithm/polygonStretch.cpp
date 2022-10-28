// 多边形扩展算法_leon_zeng0的博客-CSDN博客_多边形扩大
// https://blog.csdn.net/leon_zeng0/article/details/73500174


#include <vector>
#include <math.h>
#include <stdio.h>
#include <iostream>

// Point2D 的类定义，也包括运算符重构

class Point2D
{

public:
    double x;
    double y;

    Point2D()
    {
        x = 0;
        y = 0;
    }

    Point2D(double x, double y)
    {
        this->x = x;
        this->y = y;
    }

    friend Point2D operator+(Point2D a, Point2D b)
    {
        Point2D p;
        p.x = a.x + b.x;
        p.y = a.y + b.y;
        return p;
    };

    friend Point2D operator-(Point2D a, Point2D b)
    {
        Point2D p;
        p.x = a.x - b.x;
        p.y = a.y - b.y;
        return p;
    };

    friend double operator*(Point2D a, Point2D b)
    {
        double p = a.x * b.x + a.y * b.y;
        return p;
    };

    friend Point2D operator*(Point2D a, double value)
    {
        Point2D p;
        p.x = a.x * value;
        p.y = a.y * value;
        return p;
    };

    friend double xlJi(Point2D a, Point2D b)
    {
        double p = ((a.x) * (b.y)) - ((a.y) * (b.x));
        return p;
    };
};


class InitObj
{
public:
    InitObj()
    {
        std::cout<<" Init"<<std::endl;
    }
    InitObj(int n)
    {
        std::cout<<" Init "<<n<<std::endl;
    }

};


//测试程序
int main()
{
    InitObj a(2);
    std::cout<<"a"<<std::endl;
    InitObj b[3];
    std::cout<<"b"<<std::endl;
    InitObj* p[4];
    std::cout<<"c"<<std::endl;

    std::vector<Point2D> pList;
    std::vector<Point2D> DpList;
    std::vector<Point2D> nDpList;
    std::vector<Point2D> newList;

    Point2D p2d, p2;

    // //--注释的是原文数据，我这里把数据的顺序反了一下
    // p2d.x = 0;
    // p2d.y = 0;
    // pList.push_back(p2d);

    // // p2d.x = 50;
    // // p2d.y = 0;
    // // pList.push_back(p2d);

    // p2d.x = 100;
    // p2d.y = 0;
    // pList.push_back(p2d);

    // p2d.x = 50;
    // p2d.y = 50;
    // pList.push_back(p2d);

    // p2d.x = 100;
    // p2d.y = 100;
    // pList.push_back(p2d);

    // p2d.x = 0;
    // p2d.y = 100;
    // pList.push_back(p2d);

    double dPts[] = {

 -0.5,0.4,
 -0.3,0.6,
 -0.3,0.4,
 -0.0,0.5,
 0.0,0.3,
 -0.3,0.3,
 -0.1,-0.2,
 -0.4,-0.1,
 -0.5,0.1,
 -0.8,0.1,
 -0.8,0.7,
 -0.6,0.3

    };

    int nSize = sizeof(dPts) / sizeof(double);

    for (size_t i = 0; i < nSize; i += 2)
    {
        pList.push_back(Point2D(dPts[i], dPts[i+1]));
        //pList.push_back(Point2D(dPts[i+1]));
    }
    
    //-------------------------------------------

    int i, index, count;
    count = pList.size();
    for (i = 0; i < count; i++)
    {
        index = (i + 1) % count;
        p2d = pList[index] - pList[i];
        DpList.push_back(p2d);
    }
    
    // 初始化ndpList，单位化两顶点向量差
    double r;
    for (i = 0; i < count; i++)
    {
        r = sqrt(DpList[i] * DpList[i]);
        r = 1 / r;
        p2d = DpList[i] * r;
        nDpList.push_back(p2d);
    }

    // 计算新顶点， 注意参数dist为负是向内收缩， 为正是向外扩张
    //上述说法只是对于顺时针而言
    //我把数据改为逆时针，结论刚好相反
    double lenth;
    double dist = 0.1;
    int startindex, endindex;
    for (i = 0; i < count; i++)
    {
        startindex = i == 0 ? count - 1 : i - 1;
        endindex = i;
        double sina = xlJi(nDpList[startindex], nDpList[endindex]);
        lenth = dist / sina;
        p2d = nDpList[endindex] - nDpList[startindex];
        p2 = pList[i] + p2d * lenth;
        newList.push_back(p2);
        printf(" %.1f,%.1f,\n", p2.x, p2.y);
    }

    return 0;
}