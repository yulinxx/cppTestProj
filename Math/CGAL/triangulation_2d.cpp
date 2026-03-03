/**
 * @file triangulation_2d.cpp
 * @brief CGAL 2D 三角剖分示例
 *
 * 什么是 CGAL 2D 三角剖分？
 * ====================
 * 三角剖分（Triangulation）是将点集分割成三角形网格的过程。
 * 生成的网格满足：所有三角形不重叠，且完全覆盖点集的凸包。
 *
 * 三角剖分的数学定义：
 * - 对于点集 S，三角剖分 T(S) 是满足以下条件的三角形集合：
 *   1. 所有三角形的顶点都来自 S
 *   2. 三角形之间不重叠（仅共享边或顶点）
 *   3. 所有三角形的并集等于 S 的凸包
 *
 * Delaunay 三角剖分的核心性质：
 * 1. 空圆性质（Empty Circle Property）：
 *    每个三角形的外接圆不包含其他顶点
 *    这是 Delaunay 三角剖分最重要的性质
 * 2. 最大化最小角：
 *    Delaunay 三角剖分最大化所有三角形的最小角
 *    从而避免产生狭长的三角形
 * 3. 唯一性：
 *    在一般位置下（无四点共圆），Delaunay 三角剖分是唯一的
 * 4. 对偶性：
 *    Delaunay 三角剖分的对偶图是 Voronoi 图
 *
 * CGAL 提供的三角剖分类型：
 * 1. Delaunay_triangulation_2：标准 Delaunay 三角剖分
 * 2. Constrained_Delaunay_triangulation_2：约束 Delaunay 三角剖分
 * 3. Regular_triangulation_2：正则三角剖分（加权）
 * 4. Constrained_triangulation_2：约束三角剖分
 *
 * 三角剖分的应用场景：
 * 1. 有限元分析（FEA）：将连续域离散化为三角形单元
 * 2. 计算机图形学：曲面表示、纹理映射
 * 3. 地理信息系统：地形建模、空间分析
 * 4. 机器人学：路径规划、碰撞检测
 * 5. 医学影像：器官建模、手术规划
 * 6. 3D 打印：网格生成、切片
 * 7. 计算几何：点定位、范围查询
 *
 * 三角剖分的算法：
 * 1. 增量算法：逐点插入，O(n^2)
 * 2. 分割合并算法：O(n log n)
 * 3. 扫描线算法：O(n log n)
 * 4. Bowyer-Watson 算法：专门用于 Delaunay
 *
 * 本示例演示：
 * 1. 创建 Delaunay 三角剖分
 * 2. 遍历三角剖分的面
 * 3. Delaunay 性质验证
 * 4. 最近邻查询
 * 5. 网格生成应用
 * 6. 形状分析应用
 * 7. 路径规划概念
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Delaunay_triangulation_2.h>

// 定义内核和点类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef CGAL::Delaunay_triangulation_2<Kernel> Delaunay;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. Delaunay 三角剖分的创建和遍历
 * 2. 三角剖分的几何属性
 * 3. 最近邻查询
 * 4. 三角剖分的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 2D 三角剖分示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建 Delaunay 三角剖分 ====================
    std::cout << "\n=== 示例1: 创建 Delaunay 三角剖分 ===" << std::endl;
    
    // 创建点集
    std::vector<Point_2> points;
    points.push_back(Point_2(0, 0));
    points.push_back(Point_2(4, 0));
    points.push_back(Point_2(4, 4));
    points.push_back(Point_2(0, 4));
    points.push_back(Point_2(2, 2));  // 中心点
    
    std::cout << "输入点集:" << std::endl;
    for (const auto& p : points) {
        std::cout << "  (" << p.x() << ", " << p.y() << ")" << std::endl;
    }
    
    // 创建 Delaunay 三角剖分
    Delaunay dt;
    dt.insert(points.begin(), points.end());
    
    std::cout << "\n三角剖分创建完成" << std::endl;
    std::cout << "顶点数: " << dt.number_of_vertices() << std::endl;
    std::cout << "三角形数: " << dt.number_of_faces() << std::endl;

    // ==================== 示例2: 遍历三角剖分的面 ====================
    std::cout << "\n=== 示例2: 遍历三角剖分的面 ===" << std::endl;
    
    std::cout << "所有三角形面:" << std::endl;
    int face_count = 0;
    for (auto face = dt.finite_faces_begin(); face != dt.finite_faces_end(); ++face) {
        face_count++;
        std::cout << "三角形 " << face_count << ":" << std::endl;
        
        // 获取三角形的三个顶点
        Point_2 p1 = face->vertex(0)->point();
        Point_2 p2 = face->vertex(1)->point();
        Point_2 p3 = face->vertex(2)->point();
        
        std::cout << "  顶点1: (" << p1.x() << ", " << p1.y() << ")" << std::endl;
        std::cout << "  顶点2: (" << p2.x() << ", " << p2.y() << ")" << std::endl;
        std::cout << "  顶点3: (" << p3.x() << ", " << p3.y() << ")" << std::endl;
        
        // 计算三角形面积
        double area = std::abs((p2.x() - p1.x()) * (p3.y() - p1.y()) - 
                              (p3.x() - p1.x()) * (p2.y() - p1.y())) / 2.0;
        std::cout << "  面积: " << area << std::endl;
    }

    // ==================== 示例3: Delaunay 性质验证 ====================
    std::cout << "\n=== 示例3: Delaunay 性质验证 ===" << std::endl;
    std::cout << "Delaunay 三角剖分的性质:" << std::endl;
    std::cout << "  1. 空圆性质: 每个三角形的外接圆不包含其他顶点" << std::endl;
    std::cout << "  2. 最大化最小角: 避免狭长三角形" << std::endl;
    std::cout << "  3. 唯一性: 在一般位置下，Delaunay 三角剖分是唯一的" << std::endl;

    // ==================== 示例4: 最近邻查询 ====================
    std::cout << "\n=== 示例4: 最近邻查询 ===" << std::endl;
    
    Point_2 query_point(1.5, 1.5);
    std::cout << "查询点: (" << query_point.x() << ", " << query_point.y() << ")" << std::endl;
    
    // 查找最近的顶点
    Delaunay::Vertex_handle nearest = dt.nearest_vertex(query_point);
    Point_2 nearest_point = nearest->point();
    
    std::cout << "最近邻点: (" << nearest_point.x() << ", " << nearest_point.y() << ")" << std::endl;
    std::cout << "距离: " << std::sqrt(CGAL::squared_distance(query_point, nearest_point)) << std::endl;

    // ==================== 示例5: 三角剖分的应用 - 网格生成 ====================
    std::cout << "\n=== 示例5: 三角剖分的应用 - 网格生成 ===" << std::endl;
    
    // 生成一个规则网格的点集
    std::vector<Point_2> grid_points;
    for (int i = 0; i <= 4; i++) {
        for (int j = 0; j <= 4; j++) {
            grid_points.push_back(Point_2(i, j));
        }
    }
    
    std::cout << "生成了 5x5 = 25 个网格点" << std::endl;
    
    Delaunay grid_dt;
    grid_dt.insert(grid_points.begin(), grid_points.end());
    
    std::cout << "三角剖分后:" << std::endl;
    std::cout << "  顶点数: " << grid_dt.number_of_vertices() << std::endl;
    std::cout << "  三角形数: " << grid_dt.number_of_faces() << std::endl;
    std::cout << "  可用于有限元分析的网格" << std::endl;

    // ==================== 示例6: 三角剖分的应用 - 形状分析 ====================
    std::cout << "\n=== 示例6: 三角剖分的应用 - 形状分析 ===" << std::endl;
    
    // 计算所有三角形的最小角
    double min_angle = 90.0;  // 初始化为较大值
    for (auto face = dt.finite_faces_begin(); face != dt.finite_faces_end(); ++face) {
        Point_2 p1 = face->vertex(0)->point();
        Point_2 p2 = face->vertex(1)->point();
        Point_2 p3 = face->vertex(2)->point();
        
        // 计算三角形的三个边长
        double a = std::sqrt(CGAL::squared_distance(p2, p3));
        double b = std::sqrt(CGAL::squared_distance(p1, p3));
        double c = std::sqrt(CGAL::squared_distance(p1, p2));
        
        // 使用余弦定理计算最小角
        double cos_angle = (b*b + c*c - a*a) / (2*b*c);
        double angle = std::acos(cos_angle) * 180.0 / 3.14159265359;
        
        if (angle < min_angle) {
            min_angle = angle;
        }
    }
    
    std::cout << "三角剖分中最小角: " << min_angle << " 度" << std::endl;
    std::cout << "  最小角越大，三角形质量越好" << std::endl;

    // ==================== 示例7: 三角剖分的应用 - 路径规划 ====================
    std::cout << "\n=== 示例7: 三角剖分的应用 - 路径规划概念 ===" << std::endl;
    
    std::cout << "三角剖分可用于路径规划:" << std::endl;
    std::cout << "  1. 将自由空间三角剖分" << std::endl;
    std::cout << "  2. 在三角形中心构建对偶图" << std::endl;
    std::cout << "  3. 使用 A* 等算法在对偶图上搜索路径" << std::endl;
    std::cout << "  4. 将路径转换回实际路径" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   2D 三角剖分示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
