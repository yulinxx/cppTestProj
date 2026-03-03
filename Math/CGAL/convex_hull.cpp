/**
 * @file convex_hull.cpp
 * @brief CGAL 凸包计算示例
 *
 * 什么是 CGAL 凸包？
 * ====================
 * 凸包（Convex Hull）是包含给定点集的最小凸多边形（或凸多面体）。
 * 简单来说，凸包就像用一根橡皮筋绑住所有点后形成的边界。
 *
 * 凸包的数学定义：
 * - 对于点集 S，凸包 CH(S) 是包含 S 的最小凸集
 * - 凸集的定义：集合中任意两点的连线完全包含在该集合中
 * - 凸包的顶点是点集的极点（extreme points）
 *
 * 凸包的重要性质：
 * 1. 最小性：凸包是包含所有点的最小凸多边形
 * 2. 唯一性：在一般位置下（无三点共线），凸包是唯一的
 * 3. 极点定理：凸包的顶点是点集的极点
 * 4. 支撑超平面：凸包的每条边都是某个支撑超平面的一部分
 *
 * 凸包的几何意义：
 * - 想象用一块木板围住所有点，木板的内边界就是凸包
 * - 凸包是点集的"外壳"，内部点被完全包含
 * - 凸包上的点称为"极点"，不在凸包上的点称为"内部点"
 *
 * CGAL 提供的凸包算法：
 * 1. Graham 扫描法：O(n log n)，先排序后扫描
 * 2. QuickHull 算法：平均 O(n log n)，最坏 O(n^2)
 * 3. 增量算法：O(n^2)，适合动态插入
 * 4. 单调链算法（Andrew's algorithm）：O(n log n)
 *
 * 凸包的应用场景：
 * 1. 碰撞检测：使用凸包近似复杂物体
 * 2. 形状分析：计算形状的直径、宽度
 * 3. 地理信息系统：点集的最小包围多边形
 * 4. 计算机图形学：视锥剔除、碰撞检测
 * 5. 模式识别：形状简化、特征提取
 * 6. 机器人学：可达空间分析
 * 7. 优化问题：线性规划的可行域
 * 8. 统计学：散点图的边界
 *
 * 凸包的扩展：
 * 1. 2D凸包：点集的最小凸多边形
 * 2. 3D凸包：点集的最小凸多面体
 * 3. 高维凸包：d维空间中的凸包
 * 4. 加权凸包：考虑点权重的凸包
 * 5. 近似凸包：快速近似算法
 *
 * 本示例演示：
 * 1. 基础凸包计算
 * 2. 凸包点的遍历
 * 3. 凸包面积计算
 * 4. 特殊情况处理（共线点、单点、两点）
 * 5. 随机点集的凸包
 * 6. 凸包的应用：最远点对（直径）计算
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/convex_hull_2.h>

// 定义内核和点类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 基础凸包计算
 * 2. 凸包点的遍历
 * 3. 凸包面积计算
 * 4. 特殊情况处理（共线点）
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 凸包计算示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 基础凸包计算 ====================
    std::cout << "\n=== 示例1: 基础凸包计算 ===" << std::endl;
    
    // 创建点集
    std::vector<Point_2> points;
    points.push_back(Point_2(0, 0));
    points.push_back(Point_2(4, 0));
    points.push_back(Point_2(4, 4));
    points.push_back(Point_2(0, 4));
    points.push_back(Point_2(2, 2));  // 内部点
    points.push_back(Point_2(1, 3));  // 内部点
    
    std::cout << "输入点集:" << std::endl;
    for (const auto& p : points) {
        std::cout << "  (" << p.x() << ", " << p.y() << ")" << std::endl;
    }
    
    // 准备结果容器
    std::vector<Point_2> hull;
    
    // 计算凸包
    // convex_hull_2 函数接受输入迭代器和输出迭代器
    CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter(hull));
    
    std::cout << "\n凸包顶点（按逆时针顺序）:" << std::endl;
    for (const auto& p : hull) {
        std::cout << "  (" << p.x() << ", " << p.y() << ")" << std::endl;
    }
    std::cout << "  注意: 内部点 (2,2) 和 (1,3) 不在凸包上" << std::endl;

    // ==================== 示例2: 凸包面积计算 ====================
    std::cout << "\n=== 示例2: 凸包面积计算 ===" << std::endl;
    
    if (hull.size() >= 3) {
        // 使用叉积计算多边形面积
        double area = 0.0;
        for (size_t i = 0; i < hull.size(); i++) {
            size_t j = (i + 1) % hull.size();
            area += hull[i].x() * hull[j].y();
            area -= hull[j].x() * hull[i].y();
        }
        area = std::abs(area) / 2.0;
        
        std::cout << "凸包面积: " << area << std::endl;
        std::cout << "  计算公式: 面积 = |Σ(x_i * y_{i+1} - x_{i+1} * y_i)| / 2" << std::endl;
    }

    // ==================== 示例3: 共线点的凸包 ====================
    std::cout << "\n=== 示例3: 共线点的凸包 ===" << std::endl;
    
    std::vector<Point_2> collinear_points;
    collinear_points.push_back(Point_2(0, 0));
    collinear_points.push_back(Point_2(1, 1));
    collinear_points.push_back(Point_2(2, 2));
    collinear_points.push_back(Point_2(3, 3));
    
    std::cout << "共线点集: (0,0), (1,1), (2,2), (3,3)" << std::endl;
    
    std::vector<Point_2> collinear_hull;
    CGAL::convex_hull_2(collinear_points.begin(), collinear_points.end(), 
                        std::back_inserter(collinear_hull));
    
    std::cout << "凸包顶点: ";
    for (const auto& p : collinear_hull) {
        std::cout << "(" << p.x() << ", " << p.y() << ") ";
    }
    std::cout << std::endl;
    std::cout << "  注意: 共线点的凸包是一条线段" << std::endl;

    // ==================== 示例4: 单点和两点的凸包 ====================
    std::cout << "\n=== 示例4: 特殊情况的凸包 ===" << std::endl;
    
    // 单点
    std::vector<Point_2> single_point;
    single_point.push_back(Point_2(5, 5));
    std::vector<Point_2> single_hull;
    CGAL::convex_hull_2(single_point.begin(), single_point.end(), 
                        std::back_inserter(single_hull));
    std::cout << "单点的凸包: " << single_hull.size() << " 个顶点" << std::endl;
    
    // 两点
    std::vector<Point_2> two_points;
    two_points.push_back(Point_2(0, 0));
    two_points.push_back(Point_2(10, 10));
    std::vector<Point_2> two_hull;
    CGAL::convex_hull_2(two_points.begin(), two_points.end(), 
                        std::back_inserter(two_hull));
    std::cout << "两点的凸包: " << two_hull.size() << " 个顶点" << std::endl;

    // ==================== 示例5: 随机点集的凸包 ====================
    std::cout << "\n=== 示例5: 随机点集的凸包 ===" << std::endl;
    
    // 生成一些随机点
    std::vector<Point_2> random_points;
    for (int i = 0; i < 10; i++) {
        double x = static_cast<double>(rand() % 100);
        double y = static_cast<double>(rand() % 100);
        random_points.push_back(Point_2(x, y));
    }
    
    std::cout << "生成了 10 个随机点" << std::endl;
    
    std::vector<Point_2> random_hull;
    CGAL::convex_hull_2(random_points.begin(), random_points.end(), 
                        std::back_inserter(random_hull));
    
    std::cout << "凸包顶点数: " << random_hull.size() << std::endl;
    std::cout << "凸包顶点:" << std::endl;
    for (const auto& p : random_hull) {
        std::cout << "  (" << p.x() << ", " << p.y() << ")" << std::endl;
    }

    // ==================== 示例6: 凸包的应用 - 最远点对 ====================
    std::cout << "\n=== 示例6: 凸包的应用 - 最远点对（直径） ===" << std::endl;
    
    if (hull.size() >= 2) {
        double max_dist_sq = 0;
        Point_2 p1, p2;
        
        // 遍历凸包上的所有点对
        for (size_t i = 0; i < hull.size(); i++) {
            for (size_t j = i + 1; j < hull.size(); j++) {
                double dist_sq = CGAL::squared_distance(hull[i], hull[j]);
                if (dist_sq > max_dist_sq) {
                    max_dist_sq = dist_sq;
                    p1 = hull[i];
                    p2 = hull[j];
                }
            }
        }
        
        std::cout << "最远点对: (" << p1.x() << ", " << p1.y() << ") 和 ("
                  << p2.x() << ", " << p2.y() << ")" << std::endl;
        std::cout << "距离: " << std::sqrt(max_dist_sq) << std::endl;
        std::cout << "  注意: 最远点对必然在凸包上" << std::endl;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "   凸包示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
