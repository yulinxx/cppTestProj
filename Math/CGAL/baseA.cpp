// CGAL 入门基础_点云侠的博客-CSDN博客
// https://blog.csdn.net/qq_36686437/article/details/125552700

#include <iostream>
#include <CGAL/Simple_cartesian.h> //笛卡尔坐标相关头文件

typedef CGAL::Simple_cartesian<double> Kernel; // 内核使用双精度浮点数作为该点的笛卡尔坐标
typedef Kernel::Point_2 Point_2;               // 二维点
typedef Kernel::Segment_2 Segment_2;           // 二维线段

int main()
{
    // 定义两个位于笛卡尔坐标系下的二维点坐标
    Point_2 p(1, 1), q(10, 10);
    std::cout << "p = " << p << std::endl;
    std::cout << "q = " << q.x() << " " << q.y() << std::endl;

    // 计算两点之间的平方距离
    std::cout << "两点之间的平方距离：" << CGAL::squared_distance(p, q) << std::endl;

    // 计算m到线段pq的平方距离
    Segment_2 s(p, q); // p和q两点构成的线段
    Point_2 m(5, 9);   // 点坐标m

    std::cout << "m = " << m << std::endl;
    std::cout << "点m到线段pq的平方距离：" << CGAL::squared_distance(s, m) << std::endl;

    // 判断三点之间的位置关系
    std::cout << "p 到 q 再到 m 三点的关系为(与先后顺序有关)： ";
    switch (CGAL::orientation(p, q, m))
    {
    case CGAL::COLLINEAR:
        std::cout << "三点共线\n";
        break;
    case CGAL::LEFT_TURN:
        std::cout << "三点构成左转\n";
        break;
    case CGAL::RIGHT_TURN:
        std::cout << "三点构成右转\n";
        break;
    }

    std::cout << "p和q的中点为： " << CGAL::midpoint(p, q) << std::endl;

    return 0;
}