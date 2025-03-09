// 引入标准输入输出流库,用于进行输入输出操作
#include <iostream>
// 引入CGAL库中的精确谓词和精确构造内核,用于精确的几何计算
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
// 引入字符串流库,用于从字符串中读取数据
#include <sstream>

// 定义一个精确谓词和精确构造的内核类型
typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
// 定义二维点类型,基于上述内核
typedef Kernel::Point_2 Point_2;

/**
 * @brief 主函数,程序的入口点.
 *
 * 该函数创建了三个二维点,并检查它们是否共线.
 * 它使用了不同的方法来初始化点,并使用CGAL库的collinear函数进行共线检查.
 *
 * @return int 返回0表示程序正常结束.
 */
int main()
{
    // 创建三个二维点对象,分别表示三个点的坐标
    // p点坐标为(0, 0.3)
    // r点坐标为(2, 0.9)
    Point_2 p(0, 0.3), q, r(2, 0.9);

    {
        // 将q点坐标设置为(1, 0.6)
        q = Point_2(1, 0.6);
        // 使用CGAL的collinear函数检查p, q, r三点是否共线
        // 如果共线,输出"collinear",否则输出"not collinear"
        std::cout << (CGAL::collinear(p, q, r) ? "collinear\n" : "not collinear\n");
    }

    {
        // 创建一个字符串输入流,包含三个点的坐标信息
        std::istringstream input("0 0.3   1 0.6   2 0.9");
        // 从输入流中读取数据并赋值给p, q, r三个点
        input >> p >> q >> r;
        // 再次使用CGAL的collinear函数检查p, q, r三点是否共线
        // 如果共线,输出"collinear",否则输出"not collinear"
        std::cout << (CGAL::collinear(p, q, r) ? "collinear\n" : "not collinear\n");
    }

    {
        // 计算p和r两点的中点,并将结果赋值给q
        q = CGAL::midpoint(p, r);
        // 最后一次使用CGAL的collinear函数检查p, q, r三点是否共线
        // 如果共线,输出"collinear",否则输出"not collinear"
        std::cout << (CGAL::collinear(p, q, r) ? "collinear\n" : "not collinear\n");
    }

    // 程序正常结束,返回0
    return 0;
}