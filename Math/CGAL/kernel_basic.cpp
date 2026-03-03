/**
 * @file kernel_basic.cpp
 * @brief CGAL 几何内核基础示例
 * 
 * CGAL 提供多种内核（Kernel），每种内核使用不同的数类型来表示几何对象
 * 本文件演示：
 * 1. Simple_cartesian - 简单笛卡尔内核（使用 double）
 * 2. Exact_predicates_inexact_constructions_kernel - 精确谓词非精确构造内核
 * 3. Exact_predicates_exact_constructions_kernel - 精确谓词精确构造内核
 */

#include <iostream>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

// ==================== 内核类型定义 ====================
// 1. Simple_cartesian: 使用 double 的简单笛卡尔内核
typedef CGAL::Simple_cartesian<double> Kernel_simple;

// 2. Exact_predicates_inexact_constructions_kernel: 精确谓词但非精确构造
//    适合大多数应用，性能较好
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel_epic;

// 3. Exact_predicates_exact_constructions_kernel: 完全精确的内核
//    适合需要高精度计算的场景，性能较慢
typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel_epec;

// 定义点类型
typedef Kernel_simple::Point_2 Point_simple;
typedef Kernel_epic::Point_2 Point_epic;
typedef Kernel_epec::Point_2 Point_epec;

/**
 * @brief 演示不同内核的使用
 * 
 * 内核选择建议：
 * - 一般应用：使用 Simple_cartesian（性能最好）
 * - 需要精确判断但可接受近似构造：使用 Exact_predicates_inexact_constructions_kernel
 * - 需要完全精确的结果：使用 Exact_predicates_exact_constructions_kernel
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 几何内核基础示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: Simple_cartesian 内核 ====================
    std::cout << "\n=== 示例1: Simple_cartesian 内核 ===" << std::endl;
    {
        Point_simple p1(0, 0);
        Point_simple p2(3, 4);
        
        std::cout << "点 p1: (" << p1.x() << ", " << p1.y() << ")" << std::endl;
        std::cout << "点 p2: (" << p2.x() << ", " << p2.y() << ")" << std::endl;
        std::cout << "距离平方: " << CGAL::squared_distance(p1, p2) << std::endl;
        std::cout << "实际距离: " << std::sqrt(CGAL::squared_distance(p1, p2)) << std::endl;
        std::cout << "  注意: 使用 double 可能存在浮点精度问题" << std::endl;
    }

    // ==================== 示例2: Exact_predicates_inexact_constructions_kernel ====================
    std::cout << "\n=== 示例2: Exact_predicates_inexact_constructions_kernel ===" << std::endl;
    {
        Point_epic p1(0, 0);
        Point_epic p2(1, 1);
        Point_epic p3(2, 2);
        
        std::cout << "判断三点是否共线: " 
                  << (CGAL::collinear(p1, p2, p3) ? "是" : "否") << std::endl;
        std::cout << "  谓词（判断）是精确的，但构造（如中点）可能有误差" << std::endl;
    }

    // ==================== 示例3: Exact_predicates_exact_constructions_kernel ====================
    std::cout << "\n=== 示例3: Exact_predicates_exact_constructions_kernel ===" << std::endl;
    {
        Point_epec p1(0, 0.3);
        Point_epec p2(1, 0.6);
        Point_epec p3(2, 0.9);
        
        std::cout << "判断三点是否共线: " 
                  << (CGAL::collinear(p1, p2, p3) ? "是" : "否") << std::endl;
        std::cout << "  所有操作都是精确的，避免浮点误差" << std::endl;
        std::cout << "  注意: 性能较慢，但结果绝对可靠" << std::endl;
    }

    // ==================== 示例4: 内核选择的实际应用 ====================
    std::cout << "\n=== 示例4: 内核选择的实际应用 ===" << std::endl;
    
    std::cout << "不同场景的内核选择:" << std::endl;
    std::cout << "  1. 简单的2D绘图/游戏: Simple_cartesian" << std::endl;
    std::cout << "  2. GIS/地图应用: Exact_predicates_inexact_constructions_kernel" << std::endl;
    std::cout << "  3. CAD/精确计算: Exact_predicates_exact_constructions_kernel" << std::endl;
    std::cout << "  4. Delaunay三角剖分: 通常使用 Exact_predicates_inexact_constructions_kernel" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   内核示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
