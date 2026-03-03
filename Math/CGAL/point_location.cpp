/**
 * @file point_location.cpp
 * @brief CGAL 点定位示例
 * 
 * 点定位是确定点在几何结构中的位置的技术
 * 主要功能：
 * 1. 点在多边形内的判断
 * 2. 点在三角剖分中的定位
 * 3. 点在排列中的定位
 * 4. 点在网格中的定位
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Polygon_2.h>

// 定义内核和类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef CGAL::Delaunay_triangulation_2<Kernel> Delaunay;
typedef CGAL::Polygon_2<Kernel> Polygon_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 基础点定位
 * 2. 点定位的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 点定位示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 点在多边形内的判断 ====================
    std::cout << "\n=== 示例1: 点在多边形内的判断 ===" << std::endl;
    
    // 创建矩形多边形
    Polygon_2 poly;
    poly.push_back(Point_2(0, 0));
    poly.push_back(Point_2(4, 0));
    poly.push_back(Point_2(4, 4));
    poly.push_back(Point_2(0, 4));
    
    Point_2 p1(2, 2);    // 多边形内部
    Point_2 p2(5, 5);    // 多边形外部
    Point_2 p3(2, 0);    // 多边形边界上
    
    std::cout << "多边形: (0,0), (4,0), (4,4), (0,4)" << std::endl;
    std::cout << "点 p1(2,2): " << (poly.has_on_bounded_side(p1) ? "在内部" : "不在内部") << std::endl;
    std::cout << "点 p2(5,5): " << (poly.has_on_bounded_side(p2) ? "在内部" : "不在内部") << std::endl;
    std::cout << "点 p3(2,0): " << (poly.has_on_boundary(p3) ? "在边界上" : "不在边界上") << std::endl;
    
    std::cout << "\n应用:" << std::endl;
    std::cout << "  1. GIS 空间分析" << std::endl;
    std::cout << "  2. 碰撞检测" << std::endl;
    std::cout << "  3. 游戏中的区域判断" << std::endl;

    // ==================== 示例2: 点在三角剖分中的定位 ====================
    std::cout << "\n=== 示例2: 点在三角剖分中的定位 ===" << std::endl;
    
    // 创建点集
    std::vector<Point_2> points;
    points.push_back(Point_2(0, 0));
    points.push_back(Point_2(4, 0));
    points.push_back(Point_2(4, 4));
    points.push_back(Point_2(0, 4));
    points.push_back(Point_2(2, 2));
    
    // 创建 Delaunay 三角剖分
    Delaunay dt;
    dt.insert(points.begin(), points.end());
    
    Point_2 query(1, 1);
    std::cout << "Delaunay 三角剖分创建完成" << std::endl;
    std::cout << "查询点: (" << query.x() << ", " << query.y() << ")" << std::endl;
    
    // 查找包含点的面
    Delaunay::Face_handle face = dt.locate(query);
    
    if (face != dt.infinite_face()) {
        std::cout << "点在三角形内" << std::endl;
        std::cout << "  三角形顶点:" << std::endl;
        for (int i = 0; i < 3; i++) {
            std::cout << "    (" << face->vertex(i)->point().x() << ", " 
                      << face->vertex(i)->point().y() << ")" << std::endl;
        }
    } else {
        std::cout << "点不在任何三角形内" << std::endl;
    }
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 最近邻搜索" << std::endl;
    std::cout << "  2. 网格查询" << std::endl;
    std::cout << "  3. 有限元分析" << std::endl;

    // ==================== 示例3: 点在排列中的定位 ====================
    std::cout << "\n=== 示例3: 点在排列中的定位概念 ===" << std::endl;
    
    std::cout << "点在排列中的定位:" << std::endl;
    std::cout << "  1. 确定点在哪个面、边或顶点" << std::endl;
    std::cout << "  2. 使用点定位数据结构" << std::endl;
    std::cout << "  3. 应用: 布尔运算、路径规划" << std::endl;
    
    std::cout << "CGAL 提供的点定位类:" << std::endl;
    std::cout << "  1. Arr_point_location_face_voronoi_2" << std::endl;
    std::cout << "  2. Arr_walk_along_line_point_location" << std::endl;
    std::cout << "  3. Arr_bounded_point_location" << std::endl;

    // ==================== 示例4: 点在网格中的定位 ====================
    std::cout << "\n=== 示例4: 点在网格中的定位概念 ===" << std::endl;
    
    std::cout << "点在网格中的定位:" << std::endl;
    std::cout << "  1. 确定点在哪个单元（四面体或六面体）" << std::endl;
    std::cout << "  2. 使用网格定位数据结构" << std::endl;
    std::cout << "  3. 应用: 有限元分析、CFD" << std::endl;
    
    std::cout << "算法:" << std::endl;
    std::cout << "  1. 暴力搜索（遍历所有单元）" << std::endl;
    std::cout << "  2. 梯形搜索（Delaunay 三角剖分）" << std::endl;
    std::cout << "  3. 网格遍历（从最近单元开始）" << std::endl;

    // ==================== 示例5: 点定位的应用 - GIS ====================
    std::cout << "\n=== 示例5: 点定位的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS 中的点定位:" << std::endl;
    std::cout << "  1. 确定点在哪个区域" << std::endl;
    std::cout << "  2. 空间查询" << std::endl;
    std::cout << "  3. 地图服务" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定坐标点在哪个行政区" << std::endl;
    std::cout << "  2. 确定坐标点在哪个土地利用类型" << std::endl;
    std::cout << "  3. 确定坐标点在哪个缓冲区" << std::endl;

    // ==================== 示例6: 点定位的应用 - 游戏 ====================
    std::cout << "\n=== 示例6: 点定位的应用 - 游戏 ===" << std::endl;
    
    std::cout << "游戏中的点定位:" << std::endl;
    std::cout << "  1. 确定角色在哪个区域" << std::endl;
    std::cout << "  2. 确定鼠标点击在哪个物体" << std::endl;
    std::cout << "  3. 确定子弹击中哪个物体" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定角色在哪个地图区域" << std::endl;
    std::cout << "  2. 确定鼠标点击在哪个 UI 元素" << std::endl;
    std::cout << "  3. 确定子弹击中哪个敌人" << std::endl;

    // ==================== 示例7: 点定位的应用 - 机器人 ====================
    std::cout << "\n=== 示例7: 点定位的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的点定位:" << std::endl;
    std::cout << "  1. 确定机器人在哪个区域" << std::endl;
    std::cout << "  2. 确定目标点在哪个区域" << std::endl;
    std::cout << "  3. 确定路径上的点在哪个区域" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定机器人在哪个导航区域" << std::endl;
    std::cout << "  2. 确定目标点在哪个自由空间" << std::endl;
    std::cout << "  3. 确定路径上的点在哪个安全区域" << std::endl;

    // ==================== 示例8: 点定位的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例8: 点定位的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算���视觉中的点定位:" << std::endl;
    std::cout << "  1. 确定点在哪个物体" << std::endl;
    std::cout << "  2. 确定点在哪个区域" << std::endl;
    std::cout << "  3. 确定点在哪个特征" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定点在哪个分割区域" << std::endl;
    std::cout << "  2. 确定点在哪个物体" << std::endl;
    std::cout << "  3. 确定点在哪个特征" << std::endl;

    // ==================== 示例9: 点定位的应用 - 有限元分析 ====================
    std::cout << "\n=== 示例9: 点定位的应用 - 有限元分析 ===" << std::endl;
    
    std::cout << "有限元分析中的点定位:" << std::endl;
    std::cout << "  1. 确定点在哪个单元" << std::endl;
    std::cout << "  2. 插值计算" << std::endl;
    std::cout << "  3. 结果可视化" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定应力点在哪个单元" << std::endl;
    std::cout << "  2. 确定位移点在哪个单元" << std::endl;
    std::cout << "  3. 确定温度点在哪个单元" << std::endl;

    // ==================== 示例10: 点定位的应用 - CFD ====================
    std::cout << "\n=== 示例10: 点定位的应用 - CFD ===" << std::endl;
    
    std::cout << "CFD 中的点定位:" << std::endl;
    std::cout << "  1. 确定点在哪个单元" << std::endl;
    std::cout << "  2. 插值计算" << std::endl;
    std::cout << "  3. 后处理" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定速度点在哪个单元" << std::endl;
    std::cout << "  2. 确定压力点在哪个单元" << std::endl;
    std::cout << "  3. 确定温度点在哪个单元" << std::endl;

    // ==================== 示例11: 点定位的算法 ====================
    std::cout << "\n=== 示例11: 点定位的算法 ===" << std::endl;
    
    std::cout << "点定位的算法:" << std::endl;
    std::cout << "  1. 暴力搜索（遍历所有单元）" << std::endl;
    std::cout << "  2. 梯形搜索（Delaunay 三角剖分）" << std::endl;
    std::cout << "  3. 网格遍历（从最近单元开始）" << std::endl;
    std::cout << "  4. KD-Tree 搜索" << std::endl;
    
    std::cout << "CGAL 的实现:" << std::endl;
    std::cout << "  1. 使用精确算术" << std::endl;
    std::cout << "  2. 高效的数据结构" << std::endl;
    std::cout << "  3. 预处理加速查询" << std::endl;

    // ==================== 示例12: 点定位的挑战 ====================
    std::cout << "\n=== 示例12: 点定位的挑战 ===" << std::endl;
    
    std::cout << "点定位的挑战:" << std::endl;
    std::cout << "  1. 大规模数据处理" << std::endl;
    std::cout << "  2. 精度问题" << std::endl;
    std::cout << "  3. 动态更新" << std::endl;
    std::cout << "  4. 复杂几何" << std::endl;
    
    std::cout << "CGAL 的解决方案:" << std::endl;
    std::cout << "  1. 高效的数据结构" << std::endl;
    std::cout << "  2. 精确的算法" << std::endl;
    std::cout << "  3. 预处理加速查询" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   点定位示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
