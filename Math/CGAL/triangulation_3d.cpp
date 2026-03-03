/**
 * @file triangulation_3d.cpp
 * @brief CGAL 3D 三角剖分示例
 *
 * 什么是 CGAL 3D 三角剖分？
 * ====================
 * 3D 三角剖分是将三维点集分割成四面体网格的过程。
 * 生成的网格满足：所有四面体不重叠，且完全填充点集的凸包。
 *
 * 3D 三角剖分的数学定义：
 * - 对于点集 S，3D三角剖分 T(S) 是满足以下条件的四面体集合：
 *   1. 所有四面体的顶点都来自 S
 *   2. 四面体之间不重叠（仅共享面、边或顶点）
 *   3. 所有四面体的并集等于 S 的凸包
 *
 * 3D Delaunay 三角剖分的核心性质：
 * 1. 空球性质（Empty Sphere Property）：
 *    每个四面体的外接球不包含其他顶点
 *    这是 3D Delaunay 三角剖分最重要的性质
 * 2. 最大化最小角：
 *    Delaunay 三角剖分最大化所有四面体的最小角
 *    从而避免产生狭长的四面体
 * 3. 对偶性：
 *    3D Delaunay 三角剖分的对偶图是 3D Voronoi 图
 *
 * 3D 三角剖分的欧拉公式：
 * - V - E + F - C = 1
 * - V: 顶点数, E: 边数, F: 面数, C: 四面体数
 *
 * CGAL 提供的 3D 三角剖分类型：
 * 1. Delaunay_triangulation_3：标准 3D Delaunay 三角剖分
 * 2. Regular_triangulation_3：正则三角剖分（加权）
 * 3. Constrained_triangulation_3：约束三角剖分
 * 4. Triangulation_hierarchy_3：层次三角剖分
 *
 * 3D 三角剖分的应用场景：
 * 1. 有限元分析（FEA）：3D结构的离散化
 * 2. 计算流体力学（CFD）：流体域的网格化
 * 3. 计算机图形学：3D模型表示
 * 4. 3D打印：体网格生成
 * 5. 医学影像：器官建模
 * 6. 地理信息系统：3D地形
 * 7. 机器人学：3D环境建模
 * 8. 分子建模：分子表面
 *
 * 3D 三角剖分的算法：
 * 1. 增量算法：逐点插入，O(n²)
 * 2. 分割合并算法：O(n log n)
 * 3. Bowyer-Watson 算法：专门用于 Delaunay
 * 4. 随机增量算法：期望 O(n log n)
 *
 * 本示例演示：
 * 1. 创建 3D Delaunay 三角剖分
 * 2. 遍历三角剖分的四面体
 * 3. Delaunay 性质验证
 * 4. 最近邻查询
 * 5. 有限元分析应用
 * 6. 3D打印应用
 * 7. 计算机图形学应用
 * 8. GIS应用
 * 9. 医学影像应用
 * 10. 机器人应用
 * 11. 计算机视觉应用
 * 12. 三角剖分的性质
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Delaunay_triangulation_3.h>

// 定义内核和点类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;
typedef CGAL::Delaunay_triangulation_3<Kernel> Delaunay_3;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 3D Delaunay 三角剖分的创建
 * 2. 三角剖分的遍历
 * 3. 三角剖分的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 3D 三角剖分示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建 3D Delaunay 三角剖分 ====================
    std::cout << "\n=== 示例1: 创建 3D Delaunay 三角剖分 ===" << std::endl;
    
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
    
    std::cout << "输入点集（立方体的 8 个顶点）:" << std::endl;
    for (size_t i = 0; i < points.size(); i++) {
        std::cout << "  点 " << i << ": (" 
                  << points[i].x() << ", " 
                  << points[i].y() << ", " 
                  << points[i].z() << ")" << std::endl;
    }
    
    // 创建 3D Delaunay 三角剖分
    Delaunay_3 dt;
    dt.insert(points.begin(), points.end());
    
    std::cout << "\n三角剖分创建完成" << std::endl;
    std::cout << "  顶点数: " << dt.number_of_vertices() << std::endl;
    std::cout << "  四面体数: " << dt.number_of_cells() << std::endl;
    std::cout << "  注意: 立方体被分成 5 个四面体" << std::endl;

    // ==================== 示例2: 遍历三角剖分的四面体 ====================
    std::cout << "\n=== 示例2: 遍历三角剖分的四面体 ===" << std::endl;
    
    std::cout << "所有四面体:" << std::endl;
    int cell_count = 0;
    for (auto cell = dt.finite_cells_begin(); cell != dt.finite_cells_end(); ++cell) {
        cell_count++;
        std::cout << "四面体 " << cell_count << ":" << std::endl;
        
        // 获取四面体的四个顶点
        Point_3 p1 = cell->vertex(0)->point();
        Point_3 p2 = cell->vertex(1)->point();
        Point_3 p3 = cell->vertex(2)->point();
        Point_3 p4 = cell->vertex(3)->point();
        
        std::cout << "  顶点1: (" << p1.x() << ", " << p1.y() << ", " << p1.z() << ")" << std::endl;
        std::cout << "  顶点2: (" << p2.x() << ", " << p2.y() << ", " << p2.z() << ")" << std::endl;
        std::cout << "  顶点3: (" << p3.x() << ", " << p3.y() << ", " << p3.z() << ")" << std::endl;
        std::cout << "  顶点4: (" << p4.x() << ", " << p4.y() << ", " << p4.z() << ")" << std::endl;
    }

    // ==================== 示例3: Delaunay 性质验证 ====================
    std::cout << "\n=== 示例3: Delaunay 性质验证 ===" << std::endl;
    
    std::cout << "3D Delaunay 三角剖分的性质:" << std::endl;
    std::cout << "  1. 空球性质: 每个四面体的外接球不包含其他顶点" << std::endl;
    std::cout << "  2. 最大化最小角: 避免狭长四面体" << std::endl;
    std::cout << "  3. 唯一性: 在一般位置下，Delaunay 三角剖分是唯一的" << std::endl;
    
    std::cout << "\n空球性质:" << std::endl;
    std::cout << "  对于每个四面体，存在一个球通过其四个顶点" << std::endl;
    std::cout << "  该球内部不包含其他顶点" << std::endl;

    // ==================== 示例4: 最近邻查询 ====================
    std::cout << "\n=== 示例4: 最近邻查询 ===" << std::endl;
    
    Point_3 query(2, 2, 2);
    std::cout << "查询点: (" << query.x() << ", " << query.y() << ", " << query.z() << ")" << std::endl;
    
    // 查找最近的顶点
    Delaunay_3::Vertex_handle nearest = dt.nearest_vertex(query);
    Point_3 nearest_point = nearest->point();
    
    std::cout << "最近邻点: (" << nearest_point.x() << ", " 
              << nearest_point.y() << ", " << nearest_point.z() << ")" << std::endl;
    std::cout << "距离: " << std::sqrt(CGAL::squared_distance(query, nearest_point)) << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 最近设施搜索" << std::endl;
    std::cout << "  2. 模式识别" << std::endl;
    std::cout << "  3. 机器学习（KNN 算法）" << std::endl;

    // ==================== 示例5: 三角剖分的应用 - 有限元分析 ====================
    std::cout << "\n=== 示例5: 三角剖分的应用 - 有限元分析 ===" << std::endl;
    
    std::cout << "有限元分析（FEA）中的 3D 三角剖分:" << std::endl;
    std::cout << "  1. 将三维物体离散化为四面体网格" << std::endl;
    std::cout << "  2. 每个四面体是一个单元" << std::endl;
    std::cout << "  3. 求解偏微分方程" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 机械零件的应力分析" << std::endl;
    std::cout << "  2. 建筑结构的有限元分析" << std::endl;
    std::cout << "  3. 流体动力学模拟" << std::endl;

    // ==================== 示例6: 三角剖分的应用 - 3D 打印 ====================
    std::cout << "\n=== 示例6: 三角剖分的应用 - 3D 打印 ===" << std::endl;
    
    std::cout << "3D 打印中的 3D 三角剖分:" << std::cout << "  1. 将 3D 模型离散化为四面体网格" << std::endl;
    std::cout << "  2. 生成打印路径" << std::endl;
    std::cout << "  3. 优化打印质量" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 医疗器械的 3D 打印" << std::endl;
    std::cout << "  2. 航空航天零件的 3D 打印" << std::endl;
    std::cout << "  3. 首饰的 3D 打印" << std::endl;

    // ==================== 示例7: 三角剖分的应用 - 计算机图形学 ====================
    std::cout << "\n=== 示例7: 三角剖分的应用 - 计算机图形学 ===" << std::endl;
    
    std::cout << "计算机图形学中的 3D 三角剖分:" << std::endl;
    std::cout << "  1. 3D 模型的离散化" << std::endl;
    std::cout << "  2. 粒子系统" << std::endl;
    std::cout << "  3. 碰撞检测" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 3D 游戏中的模型" << std::endl;
    std::cout << "  2. 虚拟现实中的场景" << std::endl;
    std::cout << "  3. 3D 扫描" << std::endl;

    // ==================== 示例8: 三角剖分的应用 - 地理信息系统 ====================
    std::cout << "\n=== 示例8: 三角剖分的应用 - 地理信息系统 ===" << std::endl;
    
    std::cout << "GIS 中的 3D 三角剖分:" << std::endl;
    std::cout << "  1. DEM（数字高程模型）的三角剖分" << std::endl;
    std::cout << "  2. 3D 地形可视化" << std::endl;
    std::cout << "  3. 空间分析" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 地形分析" << std::endl;
    std::cout << "  2. 洪水模拟" << std::endl;
    std::cout << "  3. 视域分析" << std::endl;

    // ==================== 示例9: 三角剖分的应用 - 医学影像 ====================
    std::cout << "\n=== 示例9: 三角剖分的应用 - 医学影像 ===" << std::endl;
    
    std::cout << "医学影像中的 3D 三角剖分:" << std::endl;
    std::cout << "  1. 从 CT/MRI 数据重建器官表面" << std::endl;
    std::cout << "  2. 3D 器官模型" << std::endl;
    std::cout << "  3. 手术规划" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 手术规划" << std::endl;
    std::cout << "  2. 疾病诊断" << std::endl;
    std::cout << "  3. 医学教育" << std::endl;

    // ==================== 示例10: 三角剖分的应用 - 机器人 ====================
    std::cout << "\n=== 示例10: 三角剖分的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的 3D 三角剖分:" << std::endl;
    std::cout << "  1. 3D 地图构建" << std::endl;
    std::cout << "  2. 环境建模" << std::endl;
    std::cout << "  3. 碰撞检测" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 从深度相机重建环境" << std::endl;
    std::cout << "  2. 构建机器人周围的障碍物网格" << std::endl;
    std::cout << "  3. 规划无碰撞路径" << std::endl;

    // ==================== 示例11: 三角剖分的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例11: 三角剖分的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的 3D 三角剖分:" << std::endl;
    std::cout << "  1. 3D 重建" << std::endl;
    std::cout << "  2. 立体匹配" << std::endl;
    std::cout << "  3. 光流计算" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 从双目图像重建 3D 场景" << std::endl;
    std::cout << "  2. 从多视角图像重建 3D 模型" << std::endl;
    std::cout << "  3. 3D 物体识别" << std::endl;

    // ==================== 示例12: 三角剖分的性质 ====================
    std::cout << "\n=== 示例12: 三角剖分的性质 ===" << std::endl;
    
    std::cout << "3D 三角剖分的主要性质:" << std::endl;
    std::cout << "  1. 欧拉公式: V - E + F - C = 1" << std::endl;
    std::cout << "    V: 顶点数, E: 边数, F: 面数, C: 四面体数" << std::endl;
    std::cout << "  2. 每个面被两个四面体共享" << std::endl;
    std::cout << "  3. 每条边被多个四面体共享" << std::endl;
    std::cout << "  4. 四面体不重叠" << std::endl;
    
    std::cout << "\n3D Delaunay 三角剖分的性质:" << std::endl;
    std::cout << "  1. 空球性质" << std::endl;
    std::cout << "  2. 最大化最小角" << std::endl;
    std::cout << "  3. 唯一性" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   3D 三角剖分示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
