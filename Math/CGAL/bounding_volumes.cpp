/**
 * @file bounding_volumes.cpp
 * @brief CGAL 包围体示例
 * 
 * 包围体是包围点集的简单几何体
 * 主要类型：
 * 1. 矩形包围盒（Axis-Aligned Bounding Box）
 * 2. 最小包围球
 * 3. 最小包围矩形
 * 4. 最小包围椭球
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/bounding_box.h>

// 定义内核和点类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Iso_rectangle_2 Iso_rectangle_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 包围体的创建
 * 2. 包围体的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 包围体示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建点集 ====================
    std::cout << "\n=== 示例1: 创建点集 ===" << std::endl;
    
    // 生成随机点
    std::vector<Point_2> points;
    CGAL::Random_points_in_square_2<Point_2> gen(10.0);
    
    for (int i = 0; i < 20; i++) {
        points.push_back(*gen++);
    }
    
    std::cout << "生成了 20 个随机点（在 -10 到 10 的正方形内）" << std::endl;
    std::cout << "前 5 个点:" << std::endl;
    for (size_t i = 0; i < 5; i++) {
        std::cout << "  点 " << i << ": (" 
                  << points[i].x() << ", " 
                  << points[i].y() << ")" << std::endl;
    }

    // ==================== 示例2: 矩形包围盒 ====================
    std::cout << "\n=== 示例2: 矩形包围盒 ===" << std::endl;
    
    // 计算矩形包围盒
    auto bbox = CGAL::bounding_box(points.begin(), points.end());
    
    std::cout << "矩形包围盒（Axis-Aligned Bounding Box）:" << std::endl;
    std::cout << "  xmin: " << bbox.xmin() << std::endl;
    std::cout << "  xmax: " << bbox.xmax() << std::endl;
    std::cout << "  ymin: " << bbox.ymin() << std::endl;
    std::cout << "  ymax: " << bbox.ymax() << std::endl;
    std::cout << "  宽度: " << (bbox.xmax() - bbox.xmin()) << std::endl;
    std::cout << "  高度: " << (bbox.ymax() - bbox.ymin()) << std::endl;
    std::cout << "  面积: " << (bbox.xmax() - bbox.xmin()) * (bbox.ymax() - bbox.ymin()) << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 快速碰撞检测" << std::endl;
    std::cout << "  2. 空间索引" << std::endl;
    std::cout << "  3. 视锥剔除" << std::endl;

    // ==================== 示例3: 最小包围球 ====================
    std::cout << "\n=== 示例3: 最小包围球概念 ===" << std::endl;
    
    std::cout << "最小包围球（Minimum Bounding Sphere）:" << std::endl;
    std::cout << "  定义: 包围所有点的最小球" << std::endl;
    std::cout << "  应用: 碰撞检测、空间索引" << std::endl;
    
    std::cout << "算法:" << std::endl;
    std::cout << "  1. Welzl 算法（随机增量）" << std::endl;
    std::cout << "  2. 时间复杂度: O(n)" << std::endl;
    std::cout << "  3. 空间复杂度: O(n)" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  圆心: (" << (bbox.xmin() + bbox.xmax()) / 2.0 << ", " 
              << (bbox.ymin() + bbox.ymax()) / 2.0 << ")" << std::endl;
    std::cout << "  半径: " << std::sqrt(std::pow(bbox.xmax() - bbox.xmin(), 2) + 
                                        std::pow(bbox.ymax() - bbox.ymin(), 2)) / 2.0 << std::endl;

    // ==================== 示例4: 最小包围矩形 ====================
    std::cout << "\n=== 示例4: 最小包围矩形概念 ===" << std::endl;
    
    std::cout << "最小包围矩形（Minimum Area Rectangle）:" << std::endl;
    std::cout << "  定义: 包围所有点的最小面积矩形" << std::endl;
    std::cout << "  应用: 形状分析、布局优化" << std::endl;
    
    std::cout << "算法:" << std::endl;
    std::cout << "  1. 基于旋转卡壳" << std::endl;
    std::cout << "  2. 时间复杂度: O(n log n)" << std::endl;
    std::cout << "  3. 矩形的一条边与凸包的一条边重合" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 面积最小" << std::endl;
    std::cout << "  2. 可以旋转" << std::endl;
    std::cout << "  3. 比轴对齐包围盒更紧密" << std::endl;

    // ==================== 示例5: 包围体的应用 - 碰撞检测 ====================
    std::cout << "\n=== 示例5: 包围体的应用 - 碰撞检测 ===" << std::endl;
    
    std::cout << "碰撞检测中的包围体:" << std::endl;
    std::cout << "  1. 使用包围体近似复杂物体" << std::endl;
    std::cout << "  2. 包围体之间的碰撞检测更快" << std::endl;
    std::cout << "  3. 分层包围体（Bounding Volume Hierarchy）" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 游戏中的角色碰撞检测" << std::endl;
    std::cout << "  2. 机器人手臂的碰撞检测" << std::endl;
    std::cout << "  3. 虚拟现实中的交互检测" << std::endl;

    // ==================== 示例6: 包围体的应用 - 空间索引 ====================
    std::cout << "\n=== 示例6: 包围体的应用 - 空间索引 ===" << std::endl;
    
    std::cout << "空间索引中的包围体:" << std::endl;
    std::cout << "  1. R-Tree 使用包围体" << std::endl;
    std::cout << "  2. 空间查询" << std::endl;
    std::cout << "  3. 数据库索引" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. PostgreSQL/PostGIS 使用 R-Tree" << std::endl;
    std::cout << "  2. MongoDB 使用 GeoHash" << std::endl;
    std::cout << "  3. Elasticsearch 使用 BKD-Tree" << std::endl;

    // ==================== 示例7: 包围体的应用 - 视锥剔除 ====================
    std::cout << "\n=== 示例7: 包围体的应用 - 视锥剔除 ===" << std::endl;
    
    std::cout << "视锥剔除（Frustum Culling）中的包围体:" << std::endl;
    std::cout << "  1. 只渲染可见的物体" << std::endl;
    std::cout << "  2. 使用包围体与视锥相交测试" << std::endl;
    std::cout << "  3. 提高渲染性能" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 游戏引擎中的视锥剔除" << std::endl;
    std::cout << "  2. 3D 地图渲染" << std::endl;
    std::cout << "  3. 虚拟现实中的场景渲染" << std::endl;

    // ==================== 示例8: 包围体的应用 - 布局优化 ====================
    std::cout << "\n=== 示例8: 包围体的应用 - 布局优化 ===" << std::endl;
    
    std::cout << "布局优化中的包围体:" << std::endl;
    std::cout << "  1. 最小包围矩形用于布局优化" << std::endl;
    std::cout << "  2. 减少空间占用" << std::endl;
    std::cout << "  3. 优化资源使用" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 芯片布局优化" << std::endl;
    std::cout << "  2. 3D 打印的模型布局" << std::endl;
    std::cout << "  3. 仓库货物布局" << std::endl;

    // ==================== 示例9: 包围体的应用 - 形状分析 ====================
    std::cout << "\n=== 示例9: 包围体的应用 - 形状分析 ===" << std::endl;
    
    std::cout << "形状分析中的包围体:" << std::endl;
    std::cout << "  1. 形状的包围体表示" << std::endl;
    std::cout << "  2. 形状的简化" << std::endl;
    std::cout << "  3. 形状的比较" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 字符识别" << std::endl;
    std::cout << "  2. 图像检索" << std::endl;
    std::cout << "  3. 生物形态分析" << std::endl;

    // ==================== 示例10: 包围体的应用 - 3D 打印 ====================
    std::cout << "\n=== 示例10: 包围体的应用 - 3D 打印 ===" << std::endl;
    
    std::cout << "3D 打印中的包围体:" << std::endl;
    std::cout << "  1. 计算模型的包围盒" << std::endl;
    std::cout << "  2. 优化打印布局" << std::endl;
    std::cout << "  3. 计算材料用量" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 计算模型的最小包围盒" << std::endl;
    std::cout << "  2. 优化多个模型的布局" << std::endl;
    std::cout << "  3. 计算打印时间" << std::endl;

    // ==================== 示例11: 包围体的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例11: 包围体的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的包围体:" << std::endl;
    std::cout << "  1. 物体的包围盒" << std::endl;
    std::cout << "  2. 物体识别" << std::endl;
    std::cout << "  3. 3D 重建" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 从图像检测物体" << std::endl;
    std::cout << "  2. 物体识别" << std::endl;
    std::cout << "  3. 3D 扫描" << std::endl;

    // ==================== 示例12: 包围体的性质 ====================
    std::cout << "\n=== 示例12: 包围体的性质 ===" << std::endl;
    
    std::cout << "包围体的主要性质:" << std::endl;
    std::cout << "  1. 包含性: 包围所有点" << std::endl;
    std::cout << "  2. 最小性: 面积/体积最小" << std::endl;
    std::cout << "  3. 简单性: 几何形状简单" << std::endl;
    
    std::cout << "\n包围体的权衡:" << std::endl;
    std::cout << "  1. 紧密性 vs 计算复杂度" << std::endl;
    std::cout << "  2. 矩形包围盒: 简单但不够紧密" << std::endl;
    std::cout << "  3. 最小包围球: 紧密但计算复杂" << std::endl;
    std::cout << "  4. 最小包围矩形: 最紧密但计算最复杂" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   包围体示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
