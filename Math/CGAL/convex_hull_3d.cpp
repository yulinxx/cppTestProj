/**
 * @file convex_hull_3d.cpp
 * @brief CGAL 3D 凸包示例
 * 
 * 3D 凸包是包含给定点集的最小凸多面体
 * 应用场景：
 * 1. 碰撞检测
 * 2. 形状分析
 * 3. 3D 打印
 * 4. 计算机图形学
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/convex_hull_3.h>

// 定义内核和点类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 3D 凸包的创建
 * 2. 凸包的遍历
 * 3. 凸包的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 3D 凸包示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建 3D 凸包 ====================
    std::cout << "\n=== 示例1: 创建 3D 凸包 ===" << std::endl;
    
    // 创建点集（立方体的 8 个顶点）
    std::vector<Point_3> points;
    points.push_back(Point_3(0, 0, 0));
    points.push_back(Point_3(4, 0, 0));
    points.push_back(Point_3(4, 4, 0));
    points.push_back(Point_3(0, 4, 0));
    points.push_back(Point_3(0, 0, 4));
    points.push_back(Point_3(4, 0, 4));
    points.push_back(Point_3(4, 4, 4));
    points.push_back(Point_3(0, 4, 4));
    
    // 添加一些内部点
    points.push_back(Point_3(2, 2, 2));
    points.push_back(Point_3(1, 1, 1));
    points.push_back(Point_3(3, 3, 3));
    
    std::cout << "输入点集（立方体的顶点 + 内部点）:" << std::endl;
    for (size_t i = 0; i < points.size(); i++) {
        std::cout << "  点 " << i << ": (" 
                  << points[i].x() << ", " 
                  << points[i].y() << ", " 
                  << points[i].z() << ")" << std::endl;
    }
    
    // 计算凸包
    std::vector<Point_3> hull;
    CGAL::convex_hull_3(points.begin(), points.end(), std::back_inserter(hull));
    
    std::cout << "\n凸包顶点数: " << hull.size() << std::endl;
    std::cout << "  注意: 内部点不在凸包上" << std::endl;

    // ==================== 示例2: 凸包的遍历 ====================
    std::cout << "\n=== 示例2: 凸包的遍历 ===" << std::endl;
    
    std::cout << "凸包顶点:" << std::endl;
    for (size_t i = 0; i < hull.size(); i++) {
        std::cout << "  顶点 " << i << ": (" 
                  << hull[i].x() << ", " 
                  << hull[i].y() << ", " 
                  << hull[i].z() << ")" << std::endl;
    }
    
    std::cout << "\n凸包的面（三角形）:" << std::endl;
    std::cout << "  立方体的凸包有 6 个面" << std::endl;
    std::cout << "  每个面是 4 个顶点组成的四边形" << std::endl;
    std::cout << "  CGAL 会将四边形分割成 2 个三角形" << std::endl;

    // ==================== 示例3: 凸包的应用 - 碰撞检测 ====================
    std::cout << "\n=== 示例3: 凸包的应用 - 碰撞检测 ===" << std::endl;
    
    std::cout << "碰撞检测中的凸包:" << std::endl;
    std::cout << "  1. 使用凸包近似复杂物体" << std::endl;
    std::cout << "  2. 凸包之间的碰撞检测更快" << std::endl;
    std::cout << "  3. 分层凸包（Hierarchical Convex Hulls）" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 游戏中的角色碰撞检测" << std::endl;
    std::cout << "  2. 机器人手臂的碰撞检测" << std::endl;
    std::cout << "  3. 虚拟现实中的交互检测" << std::endl;

    // ==================== 示例4: 凸包的应用 - 形状分析 ====================
    std::cout << "\n=== 示例4: 凸包的应用 - 形状分析 ===" << std::endl;
    
    std::cout << "形状分析中的凸包:" << std::endl;
    std::cout << "  1. 形状的凸包表示" << std::endl;
    std::cout << "  2. 形状的简化" << std::endl;
    std::cout << "  3. 形状的比较" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 字符识别" << std::endl;
    std::cout << "  2. 图像检索" << std::endl;
    std::cout << "  3. 生物形态分析" << std::endl;

    // ==================== 示例5: 凸包的应用 - 3D 打印 ====================
    std::cout << "\n=== 示例5: 凸包的应用 - 3D 打印 ===" << std::endl;
    
    std::cout << "3D 打印中的凸包:" << std::endl;
    std::cout << "  1. 计算模型的包围盒" << std::endl;
    std::cout << "  2. 优化打印布局" << std::endl;
    std::cout << "  3. 计算材料用量" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 计算模型的最小包围盒" << std::endl;
    std::cout << "  2. 优化多个模型的布局" << std::endl;
    std::cout << "  3. 计算打印时间" << std::endl;

    // ==================== 示例6: 凸包的应用 - 计算机图形学 ====================
    std::cout << "\n=== 示例6: 凸包的应用 - 计算机图形学 ===" << std::endl;
    
    std::cout << "计算机图形学中的凸包:" << std::endl;
    std::cout << "  1. 视锥剔除（Frustum Culling）" << std::endl;
    std::cout << "  2. 碰撞检测" << std::endl;
    std::cout << "  3. 光线追踪（BVH）" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 只渲染可见的物体" << std::endl;
    std::cout << "  2. 快速碰撞检测" << std::endl;
    std::cout << "  3. 光线与物体的相交测试" << std::endl;

    // ==================== 示例7: 凸包的应用 - 机器人 ====================
    std::cout << "\n=== 示例7: 凸包的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的凸包:" << std::endl;
    std::cout << "  1. 构型空间（C-Space）计算" << std::endl;
    std::cout << "  2. 可达性分析" << std::endl;
    std::cout << "  3. 碰撞检测" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 计算机器人手臂的可达区域" << std::endl;
    std::cout << "  2. 计算障碍物的碰撞区域" << std::endl;
    std::cout << "  3. 规划无碰撞路径" << std::endl;

    // ==================== 示例8: 凸包的应用 - 地理信息系统 ====================
    std::cout << "\n=== 示例8: 凸包的应用 - 地理信息系统 ===" << std::endl;
    
    std::cout << "GIS 中的凸包:" << std::endl;
    std::cout << "  1. 点集的凸包（包围多边形）" << std::endl;
    std::cout << "  2. 最小包围盒" << std::endl;
    std::cout << "  3. 空间索引" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 空间查询" << std::endl;
    std::cout << "  2. 数据压缩" << std::endl;
    std::cout << "  3. 可视化" << std::endl;

    // ==================== 示例9: 凸包的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例9: 凸包的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的凸包:" << std::endl;
    std::cout << "  1. 形状的凸包表示" << std::endl;
    std::cout << "  2. 物体识别" << std::endl;
    std::cout << "  3. 3D 重建" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 从点云重建物体表面" << std::endl;
    std::cout << "  2. 物体识别" << std::endl;
    std::cout << "  3. 3D 扫描" << std::endl;

    // ==================== 示例10: 凸包的应用 - 生物学 ====================
    std::cout << "\n=== 示例10: 凸包的应用 - 生物学 ===" << std::endl;
    
    std::cout << "生物学中的凸包:" << std::endl;
    std::cout << "  1. 分子表面建模" << std::endl;
    std::cout << "  2. 蛋白质结构分析" << std::endl;
    std::cout << "  3. 细胞形态分析" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 计算分子的溶剂可及表面" << std::endl;
    std::cout << "  2. 分析蛋白质的结构" << std::endl;
    std::cout << "  3. 细胞形态比较" << std::endl;

    // ==================== 示例11: 凸包的算法 ====================
    std::cout << "\n=== 示例11: 凸包的算法 ===" << std::endl;
    
    std::cout << "凸包的算法:" << std::endl;
    std::cout << "  1. QuickHull 算法" << std::endl;
    std::cout << "  2. 增量算法" << std::endl;
    std::cout << "  3. 分治算法" << std::endl;
    std::cout << "  4. Graham 扫描（2D）" << std::endl;
    
    std::cout << "CGAL 的实现:" << std::endl;
    std::cout << "  1. 使用精确算术" << std::endl;
    std::cout << "  2. 处理退化情况" << std::endl;
    std::cout << "  3. 高效的数据结构" << std::endl;

    // ==================== 示例12: 凸包的性质 ====================
    std::cout << "\n=== 示例12: 凸包的性质 ===" << std::endl;
    
    std::cout << "凸包的主要性质:" << std::endl;
    std::cout << "  1. 最小性: 包含所有点的最小凸多面体" << std::endl;
    std::cout << "  2. 凸性: 任意两点的连线在凸包内" << std::endl;
    std::cout << "  3. 顶点: 凸包的顶点是原点集的子集" << std::endl;
    std::cout << "  4. 面: 凸包的面是凸多边形" << std::endl;
    
    std::cout << "\n凸包的计算复杂度:" << std::endl;
    std::cout << "  2D: O(n log n)" << std::endl;
    std::cout << "  3D: O(n log n)（平均）" << std::endl;
    std::cout << "  dD: O(n^floor(d/2))（最坏）" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   3D 凸包示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
