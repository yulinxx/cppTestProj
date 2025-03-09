/**
 * @file points_and_segment.cpp
 * @brief 此程序演示了如何使用CGAL库处理二维平面上的点和线段.
 * @author [你的名字]
 * @date [当前日期]
 */

 // 包含标准输入输出流库,用于后续的打印操作
#include <iostream>

// 包含CGAL的简单笛卡尔内核库,用于定义点和线段等几何对象
#include <CGAL/Simple_cartesian.h>

// 定义一个使用双精度浮点数的简单笛卡尔内核
typedef CGAL::Simple_cartesian<double> Kernel;

// 定义二维点类型,使用前面定义的内核
typedef Kernel::Point_2 Point_2;

// 定义二维线段类型,使用前面定义的内核
typedef Kernel::Segment_2 Segment_2;

/**
 * @brief 主函数,程序的入口点.
 * @return 程序的退出状态码,0表示正常退出.
 */
int main()
{
    // 创建两个二维点对象p和q,分别表示坐标为(1, 1)和(10, 10)的点
    Point_2 p(1, 1), q(10, 10);

    // 打印点p的信息
    std::cout << "p = " << p << std::endl;

    // 打印点q的x和y坐标
    std::cout << "q = " << q.x() << " " << q.y() << std::endl;

    // 计算并打印点p和点q之间的平方距离
    std::cout << "sqdist(p,q) = "
        << CGAL::squared_distance(p, q) << std::endl;

    // 创建一个二维线段对象s,由点p和点q组成
    Segment_2 s(p, q);

    // 创建一个二维点对象m,表示坐标为(5, 9)的点
    Point_2 m(5, 9);

    // 打印点m的信息
    std::cout << "m = " << m << std::endl;
    std::cout << "sqdist(Segment_2(p,q), m) = "
        << CGAL::squared_distance(s, m) << std::endl;
    std::cout << "p, q, and m ";
    switch (CGAL::orientation(p, q, m))
    {
    case CGAL::COLLINEAR:
        std::cout << "are collinear\n";
        break;
    case CGAL::LEFT_TURN:
        std::cout << "make a left turn\n";
        break;
    case CGAL::RIGHT_TURN:
        std::cout << "make a right turn\n";
        break;
    }
    std::cout << " midpoint(p,q) = " << CGAL::midpoint(p, q) << std::endl;
    return 0;
}