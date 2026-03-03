/**
 * @file arrangement_dcel.cpp
 * @brief CGAL 排列 DCEL 示例
 * 
 * DCEL（Doubly Connected Edge List）是表示排列的数据结构
 * 主要功能：
 * 1. DCEL 的创建
 * 2. DCEL 的遍历
 * 3. DCEL 的应用
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
 * 1. DCEL 的概念
 * 2. DCEL 的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 排列 DCEL 示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建排列 ====================
    std::cout << "\n=== 示例1: 创建排列 ===" << std::endl;
    
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

    // ==================== 示例2: DCEL 的概念 ====================
    std::cout << "\n=== 示例2: DCEL 的概念 ===" << std::endl;
    
    std::cout << "DCEL（Doubly Connected Edge List）:" << std::endl;
    std::cout << "  定义: 表示排列的数据结构" << std::endl;
    std::cout << "  组成:" << std::endl;
    std::cout << "    1. 顶点（Vertex）" << std::endl;
    std::cout << "    2. 半边（Halfedge）" << std::endl;
    std::cout << "    3. 面（Face）" << std::endl;
    std::cout << "  特点:" << std::endl;
    std::cout << "    1. 每条边由两条半边表示" << std::endl;
    std::cout << "    2. 半边有方向" << std::endl;
    std::cout << "    3. 半边形成环" << std::endl;
    
    std::cout << "\nDCEL 的优势:" << std::endl;
    std::cout << "  1. 高效的遍历" << std::endl;
    std::cout << "  2. 灵活的查询" << std::endl;
    std::cout << "  3. 支持复杂拓扑" << std::endl;

    // ==================== 示例3: DCEL 的遍历 ====================
    std::cout << "\n=== 示例3: DCEL 的遍历 ===" << std::endl;
    
    std::cout << "DCEL 的遍历:" << std::endl;
    std::cout << "  1. 遍历顶点" << std::endl;
    std::cout << "  2. 遍历半边" << std::endl;
    std::cout << "  3. 遍历面" << std::endl;
    
    std::cout << "\n遍历顶点:" << std::endl;
    int vertex_count = 0;
    for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
        vertex_count++;
        std::cout << "  顶点 " << vertex_count << ": ("
                  << vit->point().x() << ", " << vit->point().y() << ")" << std::endl;
    }
    
    std::cout << "\n遍历半边:" << std::endl;
    int halfedge_count = 0;
    for (auto eit = arr.halfedges_begin(); eit != arr.halfedges_end(); ++eit) {
        halfedge_count++;
        std::cout << "  半边 " << halfedge_count << ": ("
                  << eit->source()->point().x() << ", " << eit->source()->point().y() << ") -> ("
                  << eit->target()->point().x() << ", " << eit->target()->point().y() << ")" << std::endl;
    }
    
    std::cout << "\n遍历面:" << std::endl;
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

    // ==================== 示例4: DCEL 的应用 - 布尔运算 ====================
    std::cout << "\n=== 示例4: DCEL 的应用 - 布尔运算 ===" << std::endl;
    
    std::cout << "布尔运算中的 DCEL:" << std::endl;
    std::cout << "  1. 将两个多边形的边插入同一个排列" << std::endl;
    std::cout << "  2. 标记每个面属于哪个多边形" << std::endl;
    std::cout << "  3. 根据布尔运算类型选择面" << std::endl;
    std::cout << "  4. 重构结果多边形" << std::endl;
    
    std::cout << "支持的布尔运算:" << std::endl;
    std::cout << "  1. 并集（Union）" << std::endl;
    std::cout << "  2. 交集（Intersection）" << std::endl;
    std::cout << "  3. 差集（Difference）" << std::endl;
    std::cout << "  4. 对称差（Symmetric Difference）" << std::endl;

    // ==================== 示例5: DCEL 的应用 - 点定位 ====================
    std::cout << "\n=== 示例5: DCEL 的应用 - 点定位 ===" << std::endl;
    
    std::cout << "点定位中的 DCEL:" << std::endl;
    std::cout << "  1. 使用点定位数据结构" << std::endl;
    std::cout << "  2. 快速查找点所在的面" << std::endl;
    std::cout << "  3. 支持动态更新" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 查找点所在的面" << std::endl;
    std::cout << "  2. 查找点所在的边" << std::endl;
    std::cout << "  3. 查找点所在的顶点" << std::endl;

    // ==================== 示例6: DCEL 的应用 - 路径规划 ====================
    std::cout << "\n=== 示例6: DCEL 的应用 - 路径规划 ===" << std::endl;
    
    std::cout << "路径规划中的 DCEL:" << std::endl;
    std::cout << "  1. 构建障碍物的排列" << std::endl;
    std::cout << "  2. 自由空间由排列的面表示" << std::endl;
    std::cout << "  3. 在排列上搜索路径" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 精确的碰撞检测" << std::endl;
    std::cout << "  2. 完备性（如果存在路径，一定能找到）" << std::endl;
    std::cout << "  3. 提供最优路径（如果使用最短路径算法）" << std::endl;

    // ==================== 示例7: DCEL 的应用 - GIS ====================
    std::cout << "\n=== 示例7: DCEL 的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS 中的 DCEL:" << std::endl;
    std::cout << "  1. 地图叠加分析（Overlay）" << std::endl;
    std::cout << "  2. 多边形相交计算" << std::endl;
    std::cout << "  3. 路径网络分析" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到道路和河流的交点" << std::endl;
    std::cout << "  2. 计算土地利用和行政区的重叠" << std::endl;
    std::cout << "  3. 计算缓冲区和区域的交集" << std::endl;

    // ==================== 示例8: DCEL 的应用 - 计算机图形学 ====================
    std::cout << "\n=== 示例8: DCEL 的应用 - 计算机图形学 ===" << std::endl;
    
    std::cout << "计算机图形学中的 DCEL:" << std::endl;
    std::cout << "  1. 视觉 hull 计算" << std::endl;
    std::cout << "  2. 光流分析" << std::endl;
    std::cout << "  3. 图像分割" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 多视角重建中的可见性分析" << std::endl;
    std::cout << "  2. 运动对象的轨迹分析" << std::endl;

    // ==================== 示例9: DCEL 的应用 - 机器人 ====================
    std::cout << "\n=== 示例9: DCEL 的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的 DCEL:" << std::endl;
    std::cout << "  1. 构型空间（C-Space）的排列" << std::endl;
    std::cout << "  2. 可达性分析" << std::endl;
    std::cout << "  3. 碰撞检测" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 计算机器人手臂的可达区域" << std::endl;
    std::cout << "  2. 计算障碍物的碰撞区域" << std::endl;
    std::cout << "  3. 规划无碰撞路径" << std::endl;

    // ==================== 示例10: DCEL 的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例10: DCEL 的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的 DCEL:" << std::endl;
    std::cout << "  1. 视觉 hull 计算" << std::endl;
    std::cout << "  2. 光流分析" << std::endl;
    std::cout << "  3. 图像分割" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 多视角重建中的可见性分析" << std::endl;
    std::cout << "  2. 运动对象的轨迹分析" << std::endl;

    // ==================== 示例11: DCEL 的挑战 ====================
    std::cout << "\n=== 示例11: DCEL 的挑战 ===" << std::endl;
    
    std::cout << "DCEL 的挑战:" << std::endl;
    std::cout << "  1. 复杂的拓扑结构" << std::endl;
    std::cout << "  2. 数值精度问题" << std::endl;
    std::cout << "  3. 大规模数据处理" << std::endl;
    std::cout << "  4. 动态更新" << std::endl;
    
    std::cout << "CGAL 的解决方案:" << std::endl;
    std::cout << "  1. 使用精确算术" << std::endl;
    std::cout << "  2. 系统化处理退化情况" << std::endl;
    std::cout << "  3. 高效的数据结构" << std::endl;

    // ==================== 示例12: DCEL 的性质 ====================
    std::cout << "\n=== 示例12: DCEL 的性质 ===" << std::endl;
    
    std::cout << "DCEL 的主要性质:" << std::endl;
    std::cout << "  1. 欧拉公式: V - E + F = 1 + C" << std::endl;
    std::cout << "    V: 顶点数, E: 边数, F: 面数, C: 连通分量数" << std::endl;
    std::cout << "  2. 每条边有两个半边" << std::endl;
    std::cout << "  3. 每个半边有一个源顶点和一个目标顶点" << std::endl;
    std::cout << "  4. 每个半边有一个下一条半边和上一条半边" << std::endl;
    
    std::cout << "\nDCEL 的优势:" << std::endl;
    std::cout << "  1. 高效的遍历" << std::endl;
    std::cout << "  2. 灵活的查询" << std::endl;
    std::cout << "  3. 支持复杂拓扑" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   DCEL 示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
