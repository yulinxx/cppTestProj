// 包含标准输入输出流库,用于使用 std::cout 进行输出
#include <iostream>
// 包含 CGAL 库中的 Simple_cartesian 类,用于定义简单的笛卡尔坐标系
#include <CGAL/Simple_cartesian.h>
// 定义一个使用 double 类型的简单笛卡尔坐标系的内核
typedef CGAL::Simple_cartesian<double> Kernel;
// 定义二维点类型,使用上面定义的内核
typedef Kernel::Point_2 Point_2;

/**
 * @brief 主函数,程序的入口点
 * @return 程序的退出状态码,0 表示正常退出
 */
int main()
{
    {
        // 创建三个二维点对象,分别表示三个点的坐标
        Point_2 p(0, 0.3), q(1, 0.6), r(2, 0.9);
        // 调用 CGAL 库的 collinear 函数判断三个点是否共线
        // 根据判断结果输出相应信息
        std::cout << (CGAL::collinear(p, q, r) ? "collinear\n" : "not collinear\n");
    }
    {
        // 创建三个二维点对象,分别表示三个点的坐标
        Point_2 p(0, 1.0 / 3.0), q(1, 2.0 / 3.0), r(2, 1);
        // 调用 CGAL 库的 collinear 函数判断三个点是否共线
        // 根据判断结果输出相应信息
        std::cout << (CGAL::collinear(p, q, r) ? "collinear\n" : "not collinear\n");
    }
    {
        // 创建三个二维点对象,分别表示三个点的坐标
        Point_2 p(0, 0), q(1, 1), r(2, 2);
        // 调用 CGAL 库的 collinear 函数判断三个点是否共线
        // 根据判断结果输出相应信息
        std::cout << (CGAL::collinear(p, q, r) ? "collinear\n" : "not collinear\n");
    }
    // 返回 0 表示程序正常结束
    return 0;
}