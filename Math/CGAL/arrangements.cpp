/**
 * @file arrangements.cpp
 * @brief CGAL 排列（Arrangements）示例
 *
 * 什么是 CGAL 排列？
 * ====================
 * 排列（Arrangement）是计算几何中的一个基本数据结构，它将平面划分为
 * 顶点（vertices）、边（edges）和面（faces）。当将曲线（如线段、圆弧、
 * 多项式曲线等）插入排列时，这些曲线会被分割，它们的交点成为排列的顶点，
 * 曲线段成为排列的边，曲线围成的区域成为排列的面。
 *
 * 排列的核心概念：
 * 1. 顶点（Vertex）：曲线的端点或交点
 * 2. 边（Edge）：曲线的一部分，连接两个顶点
 * 3. 面（Face）：被边围成的区域，包括有界面和无界面
 *
 * 排列的数学性质：
 * - 欧拉公式：V - E + F = 1 + C（V顶点数，E边数，F面数，C连通分量数）
 * - 每条边有两个相邻的面
 * - 每个顶点是至少两条边的交点
 *
 * CGAL 排列的特点：
 * 1. 支持多种曲线类型：线段、圆弧、Bézier曲线、有理Bézier曲线等
 * 2. 高效的插入算法：扫描线算法、增量算法
 * 3. 丰富的查询功能：点定位、遍历、射线查询等
 * 4. 与其他CGAL数据结构集成：Delaunay三角剖分、Voronoi图等
 *
 * 应用场景：
 * 1. CAD/CAM 布尔运算：计算多边形的并、交、差
 * 2. 地理信息系统（GIS）：地图叠加分析、空间查询
 * 3. 运动规划：机器人路径规划、避障
 * 4. 计算机视觉：视觉hull计算、图像分割
 * 5. 分子建模：分子表面分析
 * 6. 电磁场分析：电磁场分布计算
 *
 * 排列的构造算法：
 * 1. 扫描线算法（Sweep-line）：O((n+k) log n)，n为曲线数，k为交点数
 * 2. 增量算法：O(n^2)，适合动态插入
 * 3. 分治算法：O(n log n + k)
 *
 * 本示例演示：
 * 1. 创建基础排列
 * 2. 遍历排列的顶点、边、面
 * 3. 线段相交检测
 * 4. 点定位
 * 5. 布尔运算概念
 * 6. 运动规划应用
 * 7. GIS空间分析
 * 8. 计算机视觉应用
 */

#include <iostream>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>

// 定义内核和类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits_2;
typedef CGAL::Arrangement_2<Traits_2> Arrangement_2;
typedef Traits_2::Point_2 Point_2;
typedef Traits_2::X_monotone_curve_2 Segment_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 基础排列的创建
 * 2. 排列的遍历
 * 3. 排列的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 排列（Arrangements）示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建基础排列 ====================
    std::cout << "\n=== 示例1: 创建基础排列 ===" << std::endl;
    
    // 创建排列
    Arrangement_2 arr;
    
    // 插入线段
    Segment_2 s1(Point_2(0, 0), Point_2(4, 0));
    Segment_2 s2(Point_2(0, 0), Point_2(0, 4));
    Segment_2 s3(Point_2(4, 0), Point_2(4, 4));
    Segment_2 s4(Point_2(0, 4), Point_2(4, 4));
    Segment_2 s5(Point_2(0, 0), Point_2(4, 4));  // 对角线
    
    arr.insert(s1);
    arr.insert(s2);
    arr.insert(s3);
    arr.insert(s4);
    arr.insert(s5);
    
    std::cout << "插入了 5 条线段:" << std::endl;
    std::cout << "  边: (0,0)-(4,0), (0,0)-(0,4), (4,0)-(4,4), (0,4)-(4,4), (0,0)-(4,4)" << std::endl;
    
    std::cout << "\n排列统计:" << std::endl;
    std::cout << "  顶点数: " << arr.number_of_vertices() << std::endl;
    std::cout << "  边数: " << arr.number_of_edges() << std::endl;
    std::cout << "  面数: " << arr.number_of_faces() << std::endl;

    // ==================== 示例2: 遍历排列的顶点 ====================
    std::cout << "\n=== 示例2: 遍历排列的顶点 ===" << std::endl;
    
    std::cout << "排列的顶点:" << std::endl;
    int vertex_count = 0;
    for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
        vertex_count++;
        std::cout << "  顶点 " << vertex_count << ": ("
                  << vit->point().x() << ", " << vit->point().y() << ")" << std::endl;
    }

    // ==================== 示例3: 遍历排列的边 ====================
    std::cout << "\n=== 示例3: 遍历排列的边 ===" << std::endl;
    
    std::cout << "排列的边:" << std::endl;
    int edge_count = 0;
    for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit) {
        edge_count++;
        std::cout << "  边 " << edge_count << ": ("
                  << eit->source()->point().x() << ", " << eit->source()->point().y() << ") -> ("
                  << eit->target()->point().x() << ", " << eit->target()->point().y() << ")" << std::endl;
    }

    // ==================== 示例4: 遍历排列的面 ====================
    std::cout << "\n=== 示例4: 遍历排列的面 ===" << std::endl;
    
    std::cout << "排列的面:" << std::endl;
    int face_count = 0;
    for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
        face_count++;
        if (!fit->is_unbounded()) {
            std::cout << "  有界面 " << face_count << ":" << std::endl;
            std::cout << "    边界环数: " << fit->number_of_holes() + 1 << std::endl;
        } else {
            std::cout << "  无界面 (外部面)" << std::endl;
        }
    }

    // ==================== 示例5: 排列的应用 - 线段相交检测 ====================
    std::cout << "\n=== 示例5: 排列的应用 - 线段相交检测 ===" << std::endl;
    
    std::cout << "使用排列检测线段相交:" << std::endl;
    std::cout << "  1. 将所有线段插入排列" << std::endl;
    std::cout << "  2. 排列自动处理相交" << std::endl;
    std::cout << "  3. 相交点成为排列的顶点" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 一次插入，多次查询" << std::endl;
    std::cout << "  2. 自动处理所有相交" << std::endl;
    std::cout << "  3. 提供完整的拓扑信息" << std::endl;

    // ==================== 示例6: 排列的应用 - 点定位 ====================
    std::cout << "\n=== 示例6: 排列的应用 - 点定位 ===" << std::endl;
    
    Point_2 query1(1, 1);
    Point_2 query2(2, 2);
    
    std::cout << "点定位查询:" << std::endl;
    
    // 查找包含点的面
    // 注意: 这里只是演示概念，实际需要使用 Arr_point_location_face_voronoi_2
    std::cout << "  点(" << query1.x() << ", " << query1.y() << ") 的定位:" << std::endl;
    std::cout << "    可以使用点定位数据结构快速查找" << std::endl;
    
    std::cout << "  点(" << query2.x() << ", " << query2.y() << ") 的定位:" << std::endl;
    std::cout << "    位于哪个面或边上" << std::endl;

    // ==================== 示例7: 排列的应用 - 布尔运算 ====================
    std::cout << "\n=== 示例7: 排列的应用 - 布尔运算概念 ===" << std::endl;
    
    std::cout << "使用排列进行多边形布尔运算:" << std::endl;
    std::cout << "  1. 将两个多边形的边插入同一个排列" << std::endl;
    std::cout << "  2. 标记每个面属于哪个多边形" << std::endl;
    std::cout << "  3. 根据布尔运算类型选择面" << std::endl;
    std::cout << "  4. 重构结果多边形" << std::endl;
    
    std::cout << "支持的布尔运算:" << std::endl;
    std::cout << "  1. 并集（Union）" << std::endl;
    std::cout << "  2. 交集（Intersection）" << std::endl;
    std::cout << "  3. 差集（Difference）" << std::endl;
    std::cout << "  4. 对称差（Symmetric Difference）" << std::endl;

    // ==================== 示例8: 排列的应用 - 运动规划 ====================
    std::cout << "\n=== 示例8: 排列的应用 - 运动规划 ===" << std::endl;
    
    std::cout << "运动规划（Motion Planning）:" << std::endl;
    std::cout << "  1. 构建障碍物的排列" << std::endl;
    std::cout << "  2. 自由空间由排列的面表示" << std::endl;
    std::cout << "  3. 在排列上搜索路径" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 精确的碰撞检测" << std::endl;
    std::cout << "  2. 完备性（如果存在路径，一定能找到）" << std::endl;
    std::cout << "  3. 提供最优路径（如果使用最短路径算法）" << std::endl;

    // ==================== 示例9: 排列的应用 - GIS 空间分析 ====================
    std::cout << "\n=== 示例9: 排列的应用 - GIS 空间分析 ===" << std::endl;
    
    std::cout << "GIS（地理信息系统）中的排列应用:" << std::endl;
    std::cout << "  1. 地图叠加分析（Overlay）" << std::endl;
    std::cout << "  2. 多边形相交计算" << std::endl;
    std::cout << "  3. 路径网络分析" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到道路和河流的交点" << std::endl;
    std::cout << "  2. 计算土地利用和行政区的重叠" << std::endl;
    std::cout << "  3. 计算缓冲区和区域的交集" << std::endl;

    // ==================== 示例10: 排列的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例10: 排列的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的排列应用:" << std::endl;
    std::cout << "  1. 视觉 hull 计算" << std::endl;
    std::cout << "  2. 光流分析" << std::endl;
    std::cout << "  3. 图像分割" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 多视角重建中的可见性分析" << std::endl;
    std::cout << "  2. 运动对象的轨迹分析" << std::endl;

    // ==================== 示例11: 插入更多线段观察排列变化 ====================
    std::cout << "\n=== 示例11: 插入更多线段观察排列变化 ===" << std::endl;
    
    // 插入更多线段
    Segment_2 s6(Point_2(0, 2), Point_2(4, 2));
    Segment_2 s7(Point_2(2, 0), Point_2(2, 4));
    
    arr.insert(s6);
    arr.insert(s7);
    
    std::cout << "插入了水平线 (0,2)-(4,2) 和垂直线 (2,0)-(2,4)" << std::endl;
    std::cout << "排列统计（更新后）:" << std::endl;
    std::cout << "  顶点数: " << arr.number_of_vertices() << std::endl;
    std::cout << "  边数: " << arr.number_of_edges() << std::endl;
    std::cout << "  面数: " << arr.number_of_faces() << std::endl;
    std::cout << "  原来的 4 个象限被进一步细分" << std::endl;

    // ==================== 示例12: 排列的性质 ====================
    std::cout << "\n=== 示例12: 排列的性质 ===" << std::endl;
    
    std::cout << "排列的主要性质:" << std::endl;
    std::cout << "  1. 欧拉公式: V - E + F = 1 + C" << std::endl;
    std::cout << "    V: 顶点数, E: 边数, F: 面数, C: 连通分量数" << std::endl;
    std::cout << "  2. 每条边有两个相邻的面" << std::endl;
    std::cout << "  3. 每个顶点是至少两条边的交点" << std::endl;
    std::cout << "  4. 排列是平面图" << std::endl;
    
    std::cout << "\n排列的构造算法:" << std::endl;
    std::cout << "  1. 扫描线算法（Sweep-line）" << std::endl;
    std::cout << "  2. 增量算法" << std::endl;
    std::cout << "  3. 分治算法" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   排列示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
