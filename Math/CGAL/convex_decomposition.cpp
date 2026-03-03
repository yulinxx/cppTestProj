/**
 * @file convex_decomposition.cpp
 * @brief CGAL 凸分解示例
 * 
 * 凸分解是将多边形分解为凸多边形的技术
 * 应用场景：
 * 1. 多边形布尔运算
 * 2. 碰撞检测
 * 3. 路径规划
 * 4. 网格生成
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polygon_2.h>

// 定义内核和多边形类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef CGAL::Polygon_2<Kernel> Polygon_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 凸分解的概念
 * 2. 凸分解的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 凸分解示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建凹多边形 ====================
    std::cout << "\n=== 示例1: 创建凹多边形 ===" << std::endl;
    
    // 创建一个凹五边形
    Polygon_2 concave_poly;
    concave_poly.push_back(Point_2(0, 0));
    concave_poly.push_back(Point_2(4, 0));
    concave_poly.push_back(Point_2(4, 2));
    concave_poly.push_back(Point_2(2, 2));
    concave_poly.push_back(Point_2(2, 4));
    concave_poly.push_back(Point_2(0, 4));
    
    std::cout << "凹五边形顶点:" << std::endl;
    std::cout << "  (0,0), (4,0), (4,2), (2,2), (2,4), (0,4)" << std::endl;
    std::cout << "  形状: 像一个 'L' 形" << std::endl;
    std::cout << "  凹点: (2,2)" << std::endl;

    // ==================== 示例2: 凸分解的概念 ====================
    std::cout << "\n=== 示例2: 凸分解的概念 ===" << std::endl;
    
    std::cout << "凸分解（Convex Decomposition）:" << std::endl;
    std::cout << "  定义: 将多边形分解为凸多边形" << std::endl;
    std::cout << "  目的: 简化计算，凸多边形的性质更好" << std::endl;
    std::cout << "  应用: 布尔运算、碰撞检测、路径规划" << std::endl;
    
    std::cout << "\n凹五边形的凸分解:" << std::endl;
    std::cout << "  可以分解为 2 个凸多边形:" << std::endl;
    std::cout << "  凸多边形 1: (0,0), (4,0), (4,2), (2,2)" << std::endl;
    std::cout << "  凸多边形 2: (0,0), (2,2), (2,4), (0,4)" << std::endl;

    // ==================== 示例3: 凸分解的应用 - 多边形布尔运算 ====================
    std::cout << "\n=== 示例3: 凸分解的应用 - 多边形布尔运算 ===" << std::endl;
    
    std::cout << "多边形布尔运算中的凸分解:" << std::endl;
    std::cout << "  1. 将凹多边形分解为凸多边形" << std::endl;
    std::cout << "  2. 对每对凸多边形进行布尔运算" << std::endl;
    std::cout << "  3. 合并结果" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 简化算法" << std::endl;
    std::cout << "  2. 提高效率" << std::endl;
    std::cout << "  3. 减少错误" << std::endl;

    // ==================== 示例4: 凸分解的应用 - 碰撞检测 ====================
    std::cout << "\n=== 示例4: 凸分解的应用 - 碰撞检测 ===" << std::endl;
    
    std::cout << "碰撞检测中的凸分解:" << std::endl;
    std::cout << "  1. 将复杂物体分解为凸部分" << std::endl;
    std::cout << "  2. 凸部分之间的碰撞检测更快" << std::endl;
    std::cout << "  3. 分层凸包（Hierarchical Convex Hulls）" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 游戏中的角色碰撞检测" << std::endl;
    std::cout << "  2. 机器人手臂的碰撞检测" << std::endl;
    std::cout << "  3. 虚拟现实中的交互检测" << std::endl;

    // ==================== 示例5: 凸分解的应用 - 路径规划 ====================
    std::cout << "\n=== 示例5: 凸分解的应用 - 路径规划 ===" << std::endl;
    
    std::cout << "路径规划中的凸分解:" << std::endl;
    std::cout << "  1. 将自由空间分解为凸区域" << std::endl;
    std::cout << "  2. 在凸区域之间搜索路径" << std::endl;
    std::cout << "  3. 保证路径的最优性" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 路径平滑" << std::endl;
    std::cout << "  2. 计算效率高" << std::endl;
    std::cout << "  3. 保证最优性" << std::endl;

    // ==================== 示例6: 凸分解的应用 - 网格生成 ====================
    std::cout << "\n=== 示例6: 凸分解的应用 - 网格生成 ===" << std::endl;
    
    std::cout << "网格生成中的凸分解:" << std::endl;
    std::cout << "  1. 将多边形分解为凸片" << std::endl;
    std::cout << "  2. 每个凸片独立三角化" << std::endl;
    std::cout << "  3. 保证网格质量" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 有限元分析的网格生成" << std::endl;
    std::cout << "  2. 3D 打印的切片" << std::endl;
    std::cout << "  3. 计算机图形学的网格生成" << std::endl;

    // ==================== 示例7: 凸分解的应用 - 计算机图形学 ====================
    std::cout << "\n=== 示例7: 凸分解的应用 - 计算机图形学 ===" << std::endl;
    
    std::cout << "计算机图形学中的凸分解:" << std::endl;
    std::cout << "  1. 凸分解用于碰撞检测" << std::endl;
    std::cout << "  2. 凸分解用于光线追踪" << std::endl;
    std::cout << "  3. 凸分解用于阴影计算" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 游戏中的碰撞检测" << std::endl;
    std::cout << "  2. 电影中的光线追踪" << std::endl;
    std::cout << "  3. 虚拟现实中的交互" << std::endl;

    // ==================== 示例8: 凸分解的应用 - 机器人 ====================
    std::cout << "\n=== 示例8: 凸分解的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的凸分解:" << std::endl;
    std::cout << "  1. 构型空间（C-Space）的凸分解" << std::endl;
    std::cout << "  2. 可达性分析" << std::endl;
    std::cout << "  3. 碰撞检测" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 计算机器人手臂的可达区域" << std::endl;
    std::cout << "  2. 计算障碍物的碰撞区域" << std::endl;
    std::cout << "  3. 规划无碰撞路径" << std::endl;

    // ==================== 示例9: 凸分解的应用 - GIS ====================
    std::cout << "\n=== 示例9: 凸分解的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS 中的凸分解:" << std::endl;
    std::cout << "  1. 复杂区域的凸分解" << std::endl;
    std::cout << "  2. 空间分析" << std::endl;
    std::cout << "  3. 地图简化" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 复杂行政区的凸分解" << std::endl;
    std::cout << "  2. 复杂地形的凸分解" << std::endl;
    std::cout << "  3. 复杂区域的空间分析" << std::endl;

    // ==================== 示例10: 凸分解的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例10: 凸分解的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的凸分解:" << std::endl;
    std::cout << "  1. 形状的凸分解" << std::endl;
    std::cout << "  2. 物体识别" << std::endl;
    std::cout << "  3. 3D 重建" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 形状的凸分解表示" << std::endl;
    std::cout << "  2. 物体识别" << std::endl;
    std::cout << "  3. 3D 扫描" << std::endl;

    // ==================== 示例11: 凸分解的算法 ====================
    std::cout << "\n=== 示例11: 凸分解的算法 ===" << std::endl;
    
    std::cout << "凸分解的算法:" << std::endl;
    std::cout << "  1. 基于对角线的算法" << std::endl;
    std::cout << "  2. 基于三角剖分的算法" << std::endl;
    std::cout << "  3. 基于凸包的算法" << std::endl;
    
    std::cout << "CGAL 的实现:" << std::endl;
    std::cout << "  1. 使用精确算术" << std::endl;
    std::cout << "  2. 处理退化情况" << std::endl;
    std::cout << "  3. 高效的数据结构" << std::endl;

    // ==================== 示例12: 凸分解的性质 ====================
    std::cout << "\n=== 示例12: 凸分解的性质 ===" << std::endl;
    
    std::cout << "凸分解的主要性质:" << std::endl;
    std::cout << "  1. 完整性: 分解后的凸多边形覆盖原多边形" << std::endl;
    std::cout << "  2. 不重叠: 凸多边形之间不重叠" << std::endl;
    std::cout << "  3. 凸性: 每个分解后的多边形是凸的" << std::endl;
    
    std::cout << "\n凸分解的挑战:" << std::endl;
    std::cout << "  1. 凸多边形的数量" << std::endl;
    std::cout << "  2. 分解的质量" << std::endl;
    std::cout << "  3. 计算效率" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   凸分解示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
