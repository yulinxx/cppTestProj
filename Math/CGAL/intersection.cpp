/**
 * @file intersection.cpp
 * @brief CGAL 几何相交示例
 * 
 * 几何相交是检测几何对象是否相交的技术
 * 主要功能：
 * 1. 线段相交
 * 2. 多边形相交
 * 3. 圆相交
 * 4. 点与几何对象的关系
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/segment_2.h>
#include <CGAL/polygon_2.h>

// 定义内核和类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;
typedef Kernel::Circle_2 Circle_2;
typedef CGAL::Polygon_2<Kernel> Polygon_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 基础相交测试
 * 2. 相交的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 几何相交示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 线段相交 ====================
    std::cout << "\n=== 示例1: 线段相交 ===" << std::endl;
    
    // 创建线段
    Segment_2 s1(Point_2(0, 0), Point_2(4, 4));
    Segment_2 s2(Point_2(0, 4), Point_2(4, 0));
    Segment_2 s3(Point_2(5, 5), Point_2(8, 8));
    
    std::cout << "线段 s1: (0,0) -> (4,4)" << std::endl;
    std::cout << "线段 s2: (0,4) -> (4,0)" << std::endl;
    std::cout << "线段 s3: (5,5) -> (8,8)" << std::endl;
    
    std::cout << "\ns1 和 s2 是否相交: " 
              << (CGAL::do_intersect(s1, s2) ? "是" : "否") << std::endl;
    std::cout << "  交点: (2,2)" << std::endl;
    
    std::cout << "s1 和 s3 是否相交: " 
              << (CGAL::do_intersect(s1, s3) ? "是" : "否") << std::endl;
    std::cout << "  说明: 两条线段在同一条直线上但不重叠" << std::endl;

    // ==================== 示例2: 多边形相交 ====================
    std::cout << "\n=== 示例2: 多边形相交概念 ===" << std::endl;
    
    // 创建多边形
    Polygon_2 poly1;
    poly1.push_back(Point_2(0, 0));
    poly1.push_back(Point_2(4, 0));
    poly1.push_back(Point_2(4, 4));
    poly1.push_back(Point_2(0, 4));
    
    Polygon_2 poly2;
    poly2.push_back(Point_2(2, 2));
    poly2.push_back(Point_2(6, 2));
    poly2.push_back(Point_2(6, 6));
    poly2.push_back(Point_2(2, 6));
    
    std::cout << "多边形 1: (0,0), (4,0), (4,4), (0,4)" << std::endl;
    std::cout << "多边形 2: (2,2), (6,2), (6,6), (2,6)" << std::endl;
    
    std::cout << "\n多边形是否相交: " << std::endl;
    std::cout << "  CGAL 提供多边形相交测试" << std::endl;
    std::cout << "  重叠区域: (2,2) 到 (4,4)" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. CAD/CAM 布尔运算" << std::endl;
    std::cout << "  2. GIS 空间分析" << std::endl;
    std::cout << "  3. 碰撞检测" << std::endl;

    // ==================== 示例3: 圆相交 ====================
    std::cout << "\n=== 示例3: 圆相交 ===" << std::endl;
    
    Circle_2 c1(Point_2(0, 0), 25);   // 圆心(0,0)，半径5
    Circle_2 c2(Point_2(8, 0), 25);   // 圆心(8,0)，半径5，圆心距8 < 10，相交
    Circle_2 c3(Point_2(20, 0), 25);  // 圆心(20,0)，半径5，圆心距20 > 10，不相交
    
    std::cout << "圆 c1: 圆心(0,0)，半径5" << std::endl;
    std::cout << "圆 c2: 圆心(8,0)，半径5" << std::endl;
    std::cout << "圆 c3: 圆心(20,0)，半径5" << std::endl;
    
    std::cout << "\nc1 和 c2 是否相交: " 
              << (CGAL::do_intersect(c1, c2) ? "是" : "否") << std::endl;
    std::cout << "  说明: 圆心距8 < 半径和10，相交于两点" << std::endl;
    
    std::cout << "c1 和 c3 是否相交: " 
              << (CGAL::do_intersect(c1, c3) ? "是" : "否") << std::endl;
    std::cout << "  说明: 圆心距20 > 半径和10，不相交" << std::endl;

    // ==================== 示例4: 点与线段的关系 ====================
    std::cout << "\n=== 示例4: 点与线段的关系 ===" << std::endl;
    
    Segment_2 seg(Point_2(0, 0), Point_2(4, 4));
    Point_2 p1(2, 2);  // 在线段上
    Point_2 p2(1, 3);  // 在线段左侧
    Point_2 p3(3, 1);  // 在线段右侧
    
    std::cout << "线段: (0,0) -> (4,4)" << std::endl;
    std::cout << "点 p1(2,2): " << (seg.has_on(p1) ? "在线段上" : "不在线段上") << std::endl;
    std::cout << "点 p2(1,3): " << (seg.has_on(p2) ? "在线段上" : "不在线段上") << std::endl;
    std::cout << "点 p3(3,1): " << (seg.has_on(p3) ? "在线段上" : "不在线段上") << std::endl;
    
    std::cout << "\n点与线段的相对位置:" << std::endl;
    std::cout << "  p1(2,2): 在线段上" << std::endl;
    std::cout << "  p2(1,3): 在线段左侧（逆时针方向）" << std::endl;
    std::cout << "  p3(3,1): 在线段右侧（顺时针方向）" << std::endl;

    // ==================== 示例5: 点与多边形的关系 ====================
    std::cout << "\n=== 示例5: 点与多边形的关系 ===" << std::endl;
    
    Polygon_2 poly;
    poly.push_back(Point_2(0, 0));
    poly.push_back(Point_2(4, 0));
    poly.push_back(Point_2(4, 4));
    poly.push_back(Point_2(0, 4));
    
    Point_2 p_in(2, 2);    // 多边形内部
    Point_2 p_out(5, 5);   // 多边形外部
    Point_2 p_on(2, 0);    // 多边形边界上
    
    std::cout << "多边形: (0,0), (4,0), (4,4), (0,4)" << std::endl;
    std::cout << "点 p_in(2,2): " << (poly.has_on_bounded_side(p_in) ? "在内部" : "不在内部") << std::endl;
    std::cout << "点 p_out(5,5): " << (poly.has_on_bounded_side(p_out) ? "在内部" : "不在内部") << std::endl;
    std::cout << "点 p_on(2,0): " << (poly.has_on_boundary(p_on) ? "在边界上" : "不在边界上") << std::endl;

    // ==================== 示例6: 线段与圆的相交 ====================
    std::cout << "\n=== 示例6: 线段与圆的相交概念 ===" << std::endl;
    
    Circle_2 circle(Point_2(0, 0), 25);  // 圆心(0,0)，半径5
    Segment_2 seg1(Point_2(0, 0), Point_2(10, 0));  // 通过圆心
    Segment_2 seg2(Point_2(6, 0), Point_2(10, 0));  // 不相交
    Segment_2 seg3(Point_2(3, 0), Point_2(7, 0));   // 部分相交
    
    std::cout << "圆: 圆心(0,0)，半径5" << std::endl;
    std::cout << "线段 seg1: (0,0) -> (10,0)" << std::endl;
    std::cout << "线段 seg2: (6,0) -> (10,0)" << std::endl;
    std::cout << "线段 seg3: (3,0) -> (7,0)" << std::endl;
    
    std::cout << "\nseg1 和圆是否相交: " << std::endl;
    std::cout << "  说明: 线段通过圆心，相交于两点" << std::endl;
    
    std::cout << "seg2 和圆是否相交: " << std::endl;
    std::cout << "  说明: 线段在圆外，不相交" << std::endl;
    
    std::cout << "seg3 和圆是否相交: " << std::endl;
    std::cout << "  说明: 线段部分在圆内，部分在圆外" << std::endl;

    // ==================== 示例7: 多边形与圆的相交 ====================
    std::cout << "\n=== 示例7: 多边形与圆的相交概念 ===" << std::endl;
    
    std::cout << "多边形与圆的相交:" << std::endl;
    std::cout << "  1. 检测多边形和圆是否有重叠" << std::endl;
    std::cout << "  2. 计算相交区域" << std::endl;
    std::cout << "  3. 应用: 碰撞检测、缓冲区分析" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 检测圆形障碍物和多边形物体是否碰撞" << std::endl;
    std::cout << "  2. 计算圆形缓冲区和多边形的重叠区域" << std::endl;

    // ==================== 示例8: 线段与矩形的相交 ====================
    std::cout << "\n=== 示例8: 线段与矩形的相交概念 ===" << std::endl;
    
    std::cout << "线段与矩形的相交:" << std::endl;
    std::cout << "  1. 检测线段是否与矩形相交" << std::endl;
    std::cout << "  2. 计算交点" << std::endl;
    std::cout << "  3. 应用: 视锥剔除、碰撞检测" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 检测视线是否与矩形障碍物相交" << std::endl;
    std::cout << "  2. 检测光线是否与矩形窗口相交" << std::endl;

    // ==================== 示例9: 几何相交的应用 - 碰撞检测 ====================
    std::cout << "\n=== 示例9: 几何相交的应用 - 碰撞检测 ===" << std::endl;
    
    std::cout << "碰撞检测中的几何相交:" << std::endl;
    std::cout << "  1. 检测物体是否碰撞" << std::endl;
    std::cout << "  2. 使用包围体加速检测" << std::endl;
    std::cout << "  3. 精确碰撞检测" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 游戏中的角色碰撞检测" << std::endl;
    std::cout << "  2. 机器人手臂的碰撞检测" << std::endl;
    std::cout << "  3. 虚拟现实中的交互检测" << std::endl;

    // ==================== 示例10: 几何相交的应用 - CAD/CAM ====================
    std::cout << "\n=== 示例10: 几何相交的应用 - CAD/CAM ===" << std::endl;
    
    std::cout << "CAD/CAM 中的几何相交:" << std::endl;
    std::cout << "  1. 布尔运算" << std::endl;
    std::cout << "  2. 刀具路径规划" << std::endl;
    std::cout << "  3. 干涉检查" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 计算两个零件的重叠区域" << std::endl;
    std::cout << "  2. 检查刀具是否与零件干涉" << std::endl;
    std::cout << "  3. 计算切削区域" << std::endl;

    // ==================== 示例11: 几何相交的应用 - GIS ====================
    std::cout << "\n=== 示例11: 几何相交的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS 中的几何相交:" << std::endl;
    std::cout << "  1. 空间叠加分析" << std::endl;
    std::cout << "  2. 路径与区域的相交" << std::endl;
    std::cout << "  3. 缓冲区分析" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到道路和河流的交点" << std::endl;
    std::cout << "  2. 找到缓冲区和土地利用的重叠" << std::endl;
    std::cout << "  3. 找到视线和地形的交点" << std::endl;

    // ==================== 示例12: 几何相交的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例12: 几何相交的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的几何相交:" << std::endl;
    std::cout << "  1. 光线与物体的相交" << std::endl;
    std::cout << "  2. 视锥与物体的相交" << std::endl;
    std::cout << "  3. 光流计算" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 光线追踪中的相交测试" << std::endl;
    std::cout << "  2. 视锥剔除" << std::endl;
    std::cout << "  3. 3D 重建" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   几何相交示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
