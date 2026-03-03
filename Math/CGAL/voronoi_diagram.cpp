/**
 * @file voronoi_diagram.cpp
 * @brief CGAL Voronoi 图示例
 * 
 * Voronoi 图是将平面划分为区域的结构，每个区域包含到一个生成点最近的点
 * 应用场景：
 * 1. 最近邻搜索
 * 2. 位置服务（最近设施）
 * 3. 晶体结构建模
 * 4. 机器人路径规划
 * 5. 图像处理（分水岭算法）
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Delaunay_triangulation_2.h>

// 定义内核和类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef CGAL::Delaunay_triangulation_2<Kernel> Delaunay;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. Voronoi 图与 Delaunay 三角剖分的关系
 * 2. Voronoi 单元的计算
 * 3. Voronoi 图的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL Voronoi 图示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: Voronoi 图与 Delaunay 三角剖分的关系 ====================
    std::cout << "\n=== 示例1: Voronoi 图与 Delaunay 三角剖分的关系 ===" << std::endl;
    
    // 创建点集
    std::vector<Point_2> points;
    points.push_back(Point_2(0, 0));
    points.push_back(Point_2(4, 0));
    points.push_back(Point_2(4, 4));
    points.push_back(Point_2(0, 4));
    points.push_back(Point_2(2, 2));
    
    std::cout << "生成点（Site）:" << std::endl;
    for (const auto& p : points) {
        std::cout << "  (" << p.x() << ", " << p.y() << ")" << std::endl;
    }
    
    // 创建 Delaunay 三角剖分
    Delaunay dt;
    dt.insert(points.begin(), points.end());
    
    std::cout << "\nDelaunay 三角剖分:" << std::endl;
    std::cout << "  顶点数: " << dt.number_of_vertices() << std::endl;
    std::cout << "  三角形数: " << dt.number_of_faces() << std::endl;
    
    std::cout << "\nVoronoi 图:" << std::endl;
    std::cout << "  Voronoi 顶点数: " << dt.number_of_vertices() - 2 + dt.number_of_faces() << std::endl;
    std::cout << "  Voronoi 边数: 与 Delaunay 边数相同" << std::endl;
    
    std::cout << "\n对偶关系:" << std::endl;
    std::cout << "  Delaunay 三角形 -> Voronoi 顶点" << std::endl;
    std::cout << "  Delaunay 边 -> Voronoi 边" << std::endl;
    std::cout << "  Delaunay 顶点 -> Voronoi 单元" << std::endl;

    // ==================== 示例2: Voronoi 单元的计算 ====================
    std::cout << "\n=== 示例2: Voronoi 单元的计算 ===" << std::endl;
    
    std::cout << "Voronoi 单元（Voronoi Cell）:" << std::endl;
    std::cout << "  每个生成点对应一个 Voronoi 单元" << std::endl;
    std::cout << "  单元内的任意点到该生成点的距离最近" << std::endl;
    
    // 计算第一个点的 Voronoi 单元
    auto vertex = dt.vertices_begin();
    Point_2 site = vertex->point();
    
    std::cout << "生成点: (" << site.x() << ", " << site.y() << ")" << std::endl;
    std::cout << "  该点的 Voronoi 单元包含所有到它距离最近的点" << std::endl;
    
    // 找到相邻的 Voronoi 顶点
    std::cout << "  相邻的 Voronoi 顶点（三角形外心）:" << std::endl;
    for (auto face = dt.finite_faces_begin(); face != dt.finite_faces_end(); ++face) {
        // 检查这个三角形是否包含当前顶点
        if (face->vertex(0) == vertex || face->vertex(1) == vertex || face->vertex(2) == vertex) {
            Point_2 circumcenter = dt.dual(face);
            std::cout << "    (" << circumcenter.x() << ", " << circumcenter.y() << ")" << std::endl;
        }
    }

    // ==================== 示例3: Voronoi 图的应用 - 最近邻搜索 ====================
    std::cout << "\n=== 示例3: Voronoi 图的应用 - 最近邻搜索 ===" << std::endl;
    
    Point_2 query(1.5, 1.5);
    std::cout << "查询点: (" << query.x() << ", " << query.y() << ")" << std::endl;
    
    // 查找最近的生成点
    auto nearest = dt.nearest_vertex(query);
    Point_2 nearest_site = nearest->point();
    
    std::cout << "最近生成点: (" << nearest_site.x() << ", " << nearest_site.y() << ")" << std::endl;
    std::cout << "距离: " << std::sqrt(CGAL::squared_distance(query, nearest_site)) << std::endl;
    
    std::cout << "  应用: 找到最近的设施、最近的点等" << std::endl;

    // ==================== 示例4: Voronoi 图的应用 - 位置服务 ====================
    std::cout << "\n=== 示例4: Voronoi 图的应用 - 位置服务 ===" << std::endl;
    
    std::cout << "位置服务（Location-Based Services）:" << std::endl;
    std::cout << "  1. 找到最近的加油站" << std::endl;
    std::cout << "  2. 找到最近的医院" << std::endl;
    std::cout << "  3. 找到最近的餐厅" << std::endl;
    std::cout << "  4. 找到最近的充电站" << std::endl;
    
    std::cout << "实现方式:" << std::endl;
    std::cout << "  1. 构建 Voronoi 图" << std::endl;
    std::cout << "  2. 在 Voronoi 单元内快速定位" << std::endl;
    std::cout << "  3. 查询时只需找到所属单元" << std::endl;

    // ==================== 示例5: Voronoi 图的应用 - 机器人路径规划 ====================
    std::cout << "\n=== 示例5: Voronoi 图的应用 - 机器人路径规划 ===" << std::endl;
    
    std::cout << "Voronoi 图用于路径规划:" << std::endl;
    std::cout << "  1. 障碍物的 Voronoi 图表示" << std::endl;
    std::cout << "  2. Voronoi 边是到障碍物距离相等的点的集合" << std::endl;
    std::cout << "  3. 沿 Voronoi 边移动可以保持与障碍物的最大距离" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 安全性高（远离障碍物）" << std::endl;
    std::cout << "  2. 路径平滑" << std::endl;
    std::cout << "  3. 可以快速重规划" << std::endl;

    // ==================== 示例6: Voronoi 图的应用 - 晶体结构建模 ====================
    std::cout << "\n=== 示例6: Voronoi 图的应用 - 晶体结构建模 ===" << std::endl;
    
    std::cout << "晶体结构的 Voronoi 分析:" << std::endl;
    std::cout << "  1. 原子位置作为生成点" << std::endl;
    std::cout << "  2. Voronoi 单元表示原子的配位多面体" << std::endl;
    std::cout << "  3. 分析晶体结构的对称性和稳定性" << std::endl;
    
    std::cout << "计算指标:" << std::endl;
    std::cout << "  1. 配位数（Voronoi 面数）" << std::endl;
    std::cout << "  2. 单元体积" << std::endl;
    std::cout << "  3. 单元形状（各向异性）" << std::endl;

    // ==================== 示例7: Voronoi 图的应用 - 图像处理 ====================
    std::cout << "\n=== 示例7: Voronoi 图的应用 - 图像处理 ===" << std::endl;
    
    std::cout << "Voronoi 图在图像处理中的应用:" << std::endl;
    std::cout << "  1. 分水岭算法（Image Segmentation）" << std::endl;
    std::cout << "  2. 矢量化（Polygonal Approximation）" << std::endl;
    std::cout << "  3. 点云采样（Poisson Disk Sampling）" << std::endl;
    std::cout << "  4. 纹理生成" << std::endl;
    
    std::cout << "Poisson Disk 采样:" << std::endl;
    std::cout << "  1. 生成���匀分布的点" << std::endl;
    std::cout << "  2. 点之间保持最小距离" << std::endl;
    std::cout << "  3. 用于高质量采样" << std::endl;

    // ==================== 示例8: 加权 Voronoi 图 ====================
    std::cout << "\n=== 示例8: 加权 Voronoi 图 ===" << std::endl;
    
    std::cout << "加权 Voronoi 图（Weighted Voronoi Diagram）:" << std::endl;
    std::cout << "  1. 每个生成点有一个权重" << std::endl;
    std::cout << "  2. 距离度量改为加权距离" << std::endl;
    std::cout << "  3. 生成点的影响力由权重决定" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 设施选址（影响力由权重决定）" << std::endl;
    std::cout << "  2. 电磁场建模" << std::endl;
    std::cout << "  3. 重力场建模" << std::endl;

    // ==================== 示例9: Apollonius 图 ====================
    std::cout << "\n=== 示例9: Apollonius 图 ===" << std::endl;
    
    std::cout << "Apollonius 图（圆的 Voronoi 图）:" << std::endl;
    std::cout << "  1. 生成元是圆（点加半径）" << std::endl;
    std::cout << "  2. 单元由到圆的加权距离相等的点组成" << std::endl;
    std::cout << "  3. 边是双曲线的一支" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 无线网络覆盖" << std::endl;
    std::cout << "  2. 信号强度建模" << std::endl;
    std::cout << "  3. 生物学中的细胞生长模拟" << std::endl;

    // ==================== 示例10: Voronoi 图的性质 ====================
    std::cout << "\n=== 示例10: Voronoi 图的性质 ===" << std::endl;
    
    std::cout << "Voronoi 图的主要性质:" << std::endl;
    std::cout << "  1. 划分性质: 平面被划分为不重叠的区域" << std::endl;
    std::cout << "  2. 最近性: 每个区域内的点到对应生成点最近" << std::endl;
    std::cout << "  3. 对偶性: 与 Delaunay 三角剖分对偶" << std::endl;
    std::cout << "  4. 凸性: 每个 Voronoi 单元是凸多边形" << std::endl;
    std::cout << "  5. 边的性质: 边是两个生成点的垂直平分线" << std::endl;
    
    std::cout << "\nVoronoi 图的构造算法:" << std::endl;
    std::cout << "  1. Fortune 算法（ sweepline）" << std::endl;
    std::cout << "  2. 增量算法" << std::endl;
    std::cout << "  3. 分治算法" << std::endl;
    std::cout << "  4. 通过 Delaunay 三角剖分对偶构造" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   Voronoi 图示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
