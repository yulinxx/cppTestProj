/**
 * @file points_and_segment.cpp
 * @brief 此程序演示了如何使用CGAL库处理二维平面上的点和线段.
 * @author [你的名字]
 * @date [当前日期]
 */

 // 包含标准输入输出流库,用于后续的打印操作
#include <iostream>

// 包含CGAL的简单笛卡尔内核库,用于定义点和线段等几何对象
#include <CGAL/Simple_cartesian.h>

// 包含CGAL的精确谓词内核,用于精确的几何计算
#include <CGAL/Exact_predicates_kernel.h>

// 包含CGAL的凸包算法
#include <CGAL/convex_hull_2.h>

// 包含标准库的数学函数
#include <cmath>

// 定义一个使用双精度浮点数的简单笛卡尔内核
typedef CGAL::Simple_cartesian<double> Kernel;

// 定义二维点类型,使用前面定义的内核
typedef Kernel::Point_2 Point_2;

// 定义二维线段类型,使用前面定义的内核
typedef Kernel::Segment_2 Segment_2;

// 定义二维圆类型
typedef Kernel::Circle_2 Circle_2;

// 定义二维直线类型
typedef Kernel::Line_2 Line_2;

// 定义二维射线类型
typedef Kernel::Ray_2 Ray_2;

// 定义二维矩形类型
typedef Kernel::Iso_rectangle_2 Iso_rectangle_2;

// 定义二维向量类型
typedef Kernel::Vector_2 Vector_2;

/**
 * @brief 主函数,程序的入口点.
 * 
 * 本文件包含20个详细的CGAL示例程序，涵盖：
 * 1. 基础点和线段操作
 * 2. 距离计算
 * 3. 位置关系判断
 * 4. 线段相交测试
 * 5. 圆相关操作
 * 6. 直线和射线
 * 7. 三角形计算
 * 8. 矩形操作
 * 9. 垂直和平行判断
 * 10. 旋转方向判断
 * 11. 凸包计算
 * 12. 向量运算
 * 13. 中垂线性质
 * 14. 点到直线的距离
 * 15. 平行四边形法则
 * 16. 圆的相交测试
 * 17. 垂足计算
 * 18. 点在多边形内测试
 * 19. Voronoi图概念
 * 20. 精确计算
 * 
 * @return 程序的退出状态码,0表示正常退出.
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 点和线段处理示例程序（初学者版）" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 基础点和线段操作 ====================
    // 这是原始示例，保留作为基础参考
    std::cout << "\n=== 示例1: 基础点和线段操作 ===" << std::endl;
    Point_2 p(1, 1), q(10, 10);

    std::cout << "p = " << p << std::endl;
    std::cout << "q = " << q.x() << " " << q.y() << std::endl;
    std::cout << "sqdist(p,q) = " << CGAL::squared_distance(p, q) << std::endl;
    
    Segment_2 s(p, q);
    Point_2 m(5, 9);
    std::cout << "m = " << m << std::endl;
    std::cout << "sqdist(Segment_2(p,q), m) = " << CGAL::squared_distance(s, m) << std::endl;
    
    std::cout << "p, q, and m ";
    switch (CGAL::orientation(p, q, m))
    {
    case CGAL::COLLINEAR:
        std::cout << "are collinear (三点共线)\n";
        break;
    case CGAL::LEFT_TURN:
        std::cout << "make a left turn (三点构成左转/逆时针)\n";
        break;
    case CGAL::RIGHT_TURN:
        std::cout << "make a right turn (三点构成右转/顺时针)\n";
        break;
    }
    std::cout << "midpoint(p,q) = " << CGAL::midpoint(p, q) << std::endl;

    // ==================== 示例2: 点到线段的距离计算 ====================
    std::cout << "\n=== 示例2: 点到线段的距离计算 ===" << std::endl;
    Segment_2 seg1(Point_2(0, 0), Point_2(10, 0)); // x轴上的线段
    Point_2 pt1(5, 3), pt2(15, 3), pt3(-5, 3);
    
    std::cout << "线段: (0,0) -> (10,0)" << std::endl;
    std::cout << "点(5,3) 到线段的距离: " 
              << std::sqrt(CGAL::squared_distance(seg1, pt1)) << std::endl;
    std::cout << "  解释: 垂足(5,0)在线段上，距离 = sqrt(3^2) = 3" << std::endl;
    
    std::cout << "点(15,3) 到线段的距离: " 
              << std::sqrt(CGAL::squared_distance(seg1, pt2)) << std::endl;
    std::cout << "  解释: 垂足(15,0)在线段外，距离 = 到端点(10,0)的距离 = sqrt(5^2+3^2)" << std::endl;
    
    std::cout << "点(-5,3) 到线段的距离: " 
              << std::sqrt(CGAL::squared_distance(seg1, pt3)) << std::endl;
    std::cout << "  解释: 垂足(-5,0)在线段外，距离 = 到端点(0,0)的距离 = sqrt(5^2+3^2)" << std::endl;

    // ==================== 示例3: 线段相交测试 ====================
    std::cout << "\n=== 示例3: 线段相交测试 ===" << std::endl;
    Segment_2 s1(Point_2(0, 0), Point_2(10, 10));
    Segment_2 s2(Point_2(0, 10), Point_2(10, 0));
    Segment_2 s3(Point_2(20, 20), Point_2(30, 30));
    
    std::cout << "线段 s1: (0,0) -> (10,10)" << std::endl;
    std::cout << "线段 s2: (0,10) -> (10,0)" << std::endl;
    std::cout << "线段 s3: (20,20) -> (30,30)" << std::endl;
    
    std::cout << "\ns1 和 s2 是否相交: " 
              << (CGAL::do_intersect(s1, s2) ? "是" : "否") << std::endl;
    std::cout << "  解释: 两条线段在 (5,5) 处相交" << std::endl;
    
    std::cout << "s1 和 s3 是否相交: " 
              << (CGAL::do_intersect(s1, s3) ? "是" : "否") << std::endl;
    std::cout << "  解释: 两条线段在同一条直线上但不重叠，所以不相交" << std::endl;

    // ==================== 示例4: 三点位置关系判断 ====================
    std::cout << "\n=== 示例4: 三点位置关系判断 ===" << std::endl;
    Point_2 p1(0, 0), p2(5, 5), p3(3, 4), p4(3, 3);
    
    std::cout << "判断三点 p1(0,0), p2(5,5), p3(3,4) 的关系: ";
    switch (CGAL::orientation(p1, p2, p3)) {
    case CGAL::COLLINEAR:
        std::cout << "三点共线" << std::endl;
        break;
    case CGAL::LEFT_TURN:
        std::cout << "三点构成左转（逆时针）" << std::endl;
        break;
    case CGAL::RIGHT_TURN:
        std::cout << "三点构成右转（顺时针）" << std::endl;
        break;
    }
    
    std::cout << "判断三点 p1(0,0), p2(5,5), p4(3,3) 的关系: ";
    switch (CGAL::orientation(p1, p2, p4)) {
    case CGAL::COLLINEAR:
        std::cout << "三点共线" << std::endl;
        std::cout << "  解释: 点(3,3)在直线y=x上" << std::endl;
        break;
    case CGAL::LEFT_TURN:
        std::cout << "三点构成左转（逆时针）" << std::endl;
        break;
    case CGAL::RIGHT_TURN:
        std::cout << "三点构成右转（顺时针）" << std::endl;
        break;
    }

    // ==================== 示例5: 圆和点的关系 ====================
    std::cout << "\n=== 示例5: 圆和点的关系 ===" << std::endl;
    Circle_2 circle(Point_2(0, 0), 100); // 圆心在原点，半径平方为100（半径=10）
    Point_2 p_inside(3, 4), p_on(10, 0), p_outside(15, 0);
    
    std::cout << "圆: 圆心(0,0), 半径=10" << std::endl;
    
    std::cout << "点(3,4) 与圆的关系: ";
    switch (circle.bounded_side(p_inside)) {
    case CGAL::ON_BOUNDED_SIDE: std::cout << "在圆内" << std::endl; break;
    case CGAL::ON_BOUNDARY: std::cout << "在圆上" << std::endl; break;
    case CGAL::ON_UNBOUNDED_SIDE: std::cout << "在圆外" << std::endl; break;
    }
    std::cout << "  解释: sqrt(3^2+4^2) = 5 < 10，所以在圆内" << std::endl;
    
    std::cout << "点(10,0) 与圆的关系: ";
    switch (circle.bounded_side(p_on)) {
    case CGAL::ON_BOUNDED_SIDE: std::cout << "在圆内" << std::endl; break;
    case CGAL::ON_BOUNDARY: std::cout << "在圆上" << std::endl; break;
    case CGAL::ON_UNBOUNDED_SIDE: std::cout << "在圆外" << std::endl; break;
    }
    std::cout << "  解释: sqrt(10^2+0^2) = 10，所以在圆上" << std::endl;

    // ==================== 示例6: 直线和射线 ====================
    std::cout << "\n=== 示例6: 直线和射线 ===" << std::endl;
    Point_2 l1(0, 0), l2(4, 4);
    Line_2 line(l1, l2); // 通过两点确定一条直线 y = x
    Ray_2 ray(l1, l2);   // 从 l1 出发，经过 l2 的射线
    
    std::cout << "直线: y = x" << std::endl;
    std::cout << "  通过点: (" << l1.x() << ", " << l1.y() << ") 和 (" << l2.x() << ", " << l2.y() << ")" << std::endl;
    
    Point_2 test_pt(2, 2);
    std::cout << "点(2,2) 在直线上: " << (line.has_on(test_pt) ? "是" : "否") << std::endl;
    
    std::cout << "\n射线: 从(" << l1.x() << ", " << l1.y() << ") 经过 (" << l2.x() << ", " << l2.y() << ")" << std::endl;
    std::cout << "点(2,2) 在射线上: " << (ray.has_on(test_pt) ? "是" : "否") << std::endl;
    std::cout << "点(-1,-1) 在射线上: " << (ray.has_on(Point_2(-1, -1)) ? "是" : "否") << std::endl;
    std::cout << "  解释: 射线有起点，只向一个方向延伸" << std::endl;

    // ==================== 示例7: 三角形计算 ====================
    std::cout << "\n=== 示例7: 三角形计算 ===" << std::endl;
    Point_2 tri_a(0, 0), tri_b(6, 0), tri_c(3, 5);
    
    // 计算三角形面积（使用叉积）
    Vector_2 v1 = tri_b - tri_a;
    Vector_2 v2 = tri_c - tri_a;
    double area = std::abs(v1.x() * v2.y() - v1.y() * v2.x()) / 2.0;
    
    std::cout << "三角形顶点: A(" << tri_a.x() << ", " << tri_a.y() << "), "
              << "B(" << tri_b.x() << ", " << tri_b.y() << "), "
              << "C(" << tri_c.x() << ", " << tri_c.y() << ")" << std::endl;
    std::cout << "三角形面积: " << area << std::endl;
    std::cout << "计算公式: 面积 = |(B-A) × (C-A)| / 2" << std::endl;
    
    // 计算重心
    Point_2 centroid((tri_a.x() + tri_b.x() + tri_c.x()) / 3.0,
                     (tri_a.y() + tri_b.y() + tri_c.y()) / 3.0);
    std::cout << "三角形重心: (" << centroid.x() << ", " << centroid.y() << ")" << std::endl;
    std::cout << "重心是三条中线的交点" << std::endl;

    // ==================== 示例8: 矩形操作 ====================
    std::cout << "\n=== 示例8: 矩形操作 ===" << std::endl;
    Iso_rectangle_2 rect(Point_2(0, 0), Point_2(10, 5));
    
    std::cout << "矩形: 左下角(" << rect.xmin() << ", " << rect.ymin() << "), "
              << "右上角(" << rect.xmax() << ", " << rect.ymax() << ")" << std::endl;
    std::cout << "矩形宽度: " << (rect.xmax() - rect.xmin()) << std::endl;
    std::cout << "矩形高度: " << (rect.ymax() - rect.ymin()) << std::endl;
    std::cout << "矩形面积: " << (rect.xmax() - rect.xmin()) * (rect.ymax() - rect.ymin()) << std::endl;
    
    Point_2 pt_in(5, 2), pt_out(15, 3);
    std::cout << "\n点(5,2) 在矩形内: " << (rect.has_on_bounded_side(pt_in) ? "是" : "否") << std::endl;
    std::cout << "点(15,3) 在矩形内: " << (rect.has_on_bounded_side(pt_out) ? "是" : "否") << std::endl;

    // ==================== 示例9: 垂直和平行判断 ====================
    std::cout << "\n=== 示例9: 垂直和平行判断 ===" << std::endl;
    Segment_2 seg_h(Point_2(0, 0), Point_2(4, 0)); // 水平线段
    Segment_2 seg_v(Point_2(2, 0), Point_2(2, 4)); // 垂直线段
    Segment_2 seg_d(Point_2(0, 0), Point_2(4, 4)); // 对角线段
    
    std::cout << "水平线段: (0,0) -> (4,0)" << std::endl;
    std::cout << "垂直线段: (2,0) -> (2,4)" << std::endl;
    std::cout << "对角线段: (0,0) -> (4,4)" << std::endl;
    
    std::cout << "\n水平线段和垂直线段垂直: " 
              << (CGAL::is_perpendicular(seg_h, seg_v) ? "是" : "否") << std::endl;
    std::cout << "水平线段和对角线段垂直: " 
              << (CGAL::is_perpendicular(seg_h, seg_d) ? "是" : "否") << std::endl;
    
    std::cout << "\n水平线段和自身平行: " 
              << (CGAL::are_parallel(seg_h, seg_h) ? "是" : "否") << std::endl;
    std::cout << "水平线段和垂直线段平行: " 
              << (CGAL::are_parallel(seg_h, seg_v) ? "是" : "否") << std::endl;

    // ==================== 示例10: 旋转方向判断 ====================
    std::cout << "\n=== 示例10: 旋转方向判断 ===" << std::endl;
    Point_2 origin(0, 0);
    Point_2 positive_x(1, 0), positive_y(0, 1);
    
    std::cout << "从 x轴正方向 到 y轴正方向 是: ";
    switch (CGAL::orientation(positive_x, origin, positive_y)) {
    case CGAL::LEFT_TURN: 
        std::cout << "左转（逆时针）" << std::endl; 
        std::cout << "  解释: 在标准坐标系中，逆时针旋转为正方向" << std::endl;
        break;
    case CGAL::RIGHT_TURN: 
        std::cout << "右转（顺时针）" << std::endl; 
        break;
    case CGAL::COLLINEAR: 
        std::cout << "共线" << std::endl; 
        break;
    }
    
    std::cout << "从 y轴正方向 到 x轴正方向 是: ";
    switch (CGAL::orientation(positive_y, origin, positive_x)) {
    case CGAL::LEFT_TURN: 
        std::cout << "左转（逆时针）" << std::endl; 
        break;
    case CGAL::RIGHT_TURN: 
        std::cout << "右转（顺时针）" << std::endl; 
        std::cout << "  解释: 顺时针旋转为负方向" << std::endl;
        break;
    case CGAL::COLLINEAR: 
        std::cout << "共线" << std::endl; 
        break;
    }

    // ==================== 示例11: 凸包计算 ====================
    std::cout << "\n=== 示例11: 凸包计算 ===" << std::endl;
    Point_2 points[] = {Point_2(0,0), Point_2(4,0), Point_2(4,4), 
                        Point_2(0,4), Point_2(2,2), Point_2(1,3)};
    Point_2* points_end = points + 6;
    Point_2* result = new Point_2[6];
    Point_2* end = CGAL::convex_hull_2(points, points_end, result);
    
    std::cout << "输入点: (0,0), (4,0), (4,4), (0,4), (2,2), (1,3)" << std::endl;
    std::cout << "凸包顶点（按逆时针顺序）:" << std::endl;
    for (Point_2* p = result; p != end; ++p) {
        std::cout << "  (" << p->x() << ", " << p->y() << ")" << std::endl;
    }
    std::cout << "凸包是包含所有点的最小凸多边形" << std::endl;
    delete[] result;

    // ==================== 示例12: 向量运算 ====================
    std::cout << "\n=== 示例12: 向量运算 ===" << std::endl;
    Vector_2 v1(3, 4), v2(1, 2);
    
    std::cout << "向量 v1 = (" << v1.x() << ", " << v1.y() << ")" << std::endl;
    std::cout << "向量 v2 = (" << v2.x() << ", " << v2.y() << ")" << std::endl;
    
    // 向量加法
    Vector_2 sum_vec = v1 + v2;
    std::cout << "\n向量加法 v1 + v2: (" << sum_vec.x() << ", " << sum_vec.y() << ")" << std::endl;
    
    // 向量减法
    Vector_2 diff_vec = v1 - v2;
    std::cout << "向量减法 v1 - v2: (" << diff_vec.x() << ", " << diff_vec.y() << ")" << std::endl;
    
    // 向量数乘
    Vector_2 scale_vec = v1 * 2.0;
    std::cout << "向量数乘 v1 * 2: (" << scale_vec.x() << ", " << scale_vec.y() << ")" << std::endl;
    
    // 点积
    double dot = v1.x() * v2.x() + v1.y() * v2.y();
    std::cout << "\n点积 v1 · v2: " << dot << std::endl;
    std::cout << "  解释: v1 · v2 = |v1| * |v2| * cos(θ)" << std::endl;
    
    // 叉积（二维中结果是标量）
    double cross = v1.x() * v2.y() - v1.y() * v2.x();
    std::cout << "叉积 v1 × v2: " << cross << std::endl;
    std::cout << "  解释: 叉积绝对值 = 平行四边形面积" << std::endl;

    // ==================== 示例13: 中垂线性质 ====================
    std::cout << "\n=== 示例13: 中垂线性质 ===" << std::endl;
    Point_2 pa(0, 0), pb(6, 8);
    Segment_2 seg_ab(pa, pb);
    Point_2 mid_ab = CGAL::midpoint(pa, pb);
    
    std::cout << "线段 AB: A(" << pa.x() << ", " << pa.y() << ") 到 B(" << pb.x() << ", " << pb.y() << ")" << std::endl;
    std::cout << "中点 M: (" << mid_ab.x() << ", " << mid_ab.y() << ")" << std::endl;
    
    // 中垂线上的任意点到 A 和 B 的距离相等
    Point_2 perp_on_line(3, 8); // 这个点在中垂线上
    double dist_to_a = CGAL::squared_distance(perp_on_line, pa);
    double dist_to_b = CGAL::squared_distance(perp_on_line, pb);
    
    std::cout << "\n中垂线上的点 P(3,8):" << std::endl;
    std::cout << "  PA 距离平方: " << dist_to_a << std::endl;
    std::cout << "  PB 距离平方: " << dist_to_b << std::endl;
    std::cout << "  PA == PB: " << (dist_to_a == dist_to_b ? "是" : "否") << std::endl;
    std::cout << "  解释: 中垂线上的任意点到线段两端点的距离相等" << std::endl;

    // ==================== 示例14: 点到直线的距离 ====================
    std::cout << "\n=== 示例14: 点到直线的距离 ===" << std::endl;
    Line_2 x_axis(Point_2(0, 0), Point_2(10, 0)); // x轴
    Point_2 test_point(5, 3);
    
    std::cout << "点(5,3) 到 x轴的距离: " 
              << std::sqrt(CGAL::squared_distance(x_axis, test_point)) << std::endl;
    std::cout << "  解释: 垂足是(5,0)，距离就是y坐标的绝对值" << std::endl;

    // ==================== 示例15: 平行四边形法则 ====================
    std::cout << "\n=== 示例15: 平行四边形法则 ===" << std::endl;
    Point_2 par_a(0, 0), par_b(4, 1), par_c(1, 3);
    Point_2 par_d = par_b + par_c - par_a; // 第四个顶点
    
    std::cout << "平行四边形顶点:" << std::endl;
    std::cout << "  A: (" << par_a.x() << ", " << par_a.y() << ")" << std::endl;
    std::cout << "  B: (" << par_b.x() << ", " << par_b.y() << ")" << std::endl;
    std::cout << "  C: (" << par_c.x() << ", " << par_c.y() << ")" << std::endl;
    std::cout << "  D: (" << par_d.x() << ", " << par_d.y() << ")" << std::endl;
    std::cout << "  解释: D = B + C - A（向量加法）" << std::endl;
    
    // 对角线
    Segment_2 diag1(par_a, par_d);
    Segment_2 diag2(par_b, par_c);
    std::cout << "\n对角线1中点: (" << CGAL::midpoint(diag1).x() << ", " << CGAL::midpoint(diag1).y() << ")" << std::endl;
    std::cout << "对角线2中点: (" << CGAL::midpoint(diag2).x() << ", " << CGAL::midpoint(diag2).y() << ")" << std::endl;
    std::cout << "两条对角线在中点相交" << std::endl;

    // ==================== 示例16: 圆的相交测试 ====================
    std::cout << "\n=== 示例16: 圆的相交测试 ===" << std::endl;
    Circle_2 c1(Point_2(0, 0), 25);   // 圆心(0,0)，半径5
    Circle_2 c2(Point_2(8, 0), 25);   // 圆心(8,0)，半径5，圆心距8 < 10，相交
    Circle_2 c3(Point_2(20, 0), 25);  // 圆心(20,0)，半径5，圆心距20 > 10，不相交
    Circle_2 c4(Point_2(10, 0), 25);  // 圆心(10,0)，半径5，圆心距10 = 10，相切
    
    std::cout << "圆c1: 圆心(0,0)，半径5" << std::endl;
    std::cout << "圆c2: 圆心(8,0)，半径5" << std::endl;
    std::cout << "圆c3: 圆心(20,0)，半径5" << std::endl;
    std::cout << "圆c4: 圆心(10,0)，半径5" << std::endl;
    
    std::cout << "\nc1 和 c2 是否相交: " << (CGAL::do_intersect(c1, c2) ? "是" : "否") << std::endl;
    std::cout << "  解释: 圆心距8 < 半径和10，所以相交于两点" << std::endl;
    
    std::cout << "c1 和 c3 是否相交: " << (CGAL::do_intersect(c1, c3) ? "是" : "否") << std::endl;
    std::cout << "  解释: 圆心距20 > 半径和10，所以不相交" << std::endl;
    
    std::cout << "c1 和 c4 是否相交: " << (CGAL::do_intersect(c1, c4) ? "是" : "否") << std::endl;
    std::cout << "  解释: 圆心距10 = 半径和10，所以外切于一点" << std::endl;

    // ==================== 示例17: 垂足计算 ====================
    std::cout << "\n=== 示例17: 垂足计算 ===" << std::endl;
    Segment_2 seg(Point_2(0, 0), Point_2(10, 0)); // x轴上的线段
    Point_2 pt(5, 3);
    
    std::cout << "线段: (0,0) -> (10,0)" << std::endl;
    std::cout << "点: (" << pt.x() << ", " << pt.y() << ")" << std::endl;
    
    // 计算垂足
    Line_2 line_seg(seg);
    Point_2 foot = line_seg.projection(pt);
    
    std::cout << "垂足: (" << foot.x() << ", " << foot.y() << ")" << std::endl;
    std::cout << "  解释: 从点向直线作垂线，垂足的坐标" << std::endl;
    std::cout << "点到线段的距离: " << std::sqrt(CGAL::squared_distance(seg, pt)) << std::endl;

    // ==================== 示例18: 点在多边形内测试 ====================
    std::cout << "\n=== 示例18: 点在多边形内测试概念 ===" << std::endl;
    Point_2 poly_v1(0, 0), poly_v2(4, 0), poly_v3(4, 4), poly_v4(0, 4);
    Point_2 inside(2, 2), outside(5, 5);
    
    std::cout << "矩形多边形顶点: (0,0), (4,0), (4,4), (0,4)" << std::endl;
    std::cout << "点(2,2) 在矩形内: " << (rect.has_on_bounded_side(inside) ? "是" : "否") << std::endl;
    std::cout << "点(5,5) 在矩形内: " << (rect.has_on_bounded_side(outside) ? "是" : "否") << std::endl;
    std::cout << "  解释: CGAL提供更复杂的多边形包含测试" << std::endl;

    // ==================== 示例19: Voronoi图概念 ====================
    std::cout << "\n=== 示例19: Voronoi图概念 ===" << std::endl;
    Point_2 sites[] = {Point_2(0,0), Point_2(4,0), Point_2(2,3)};
    
    std::cout << "生成点（Site）: (0,0), (4,0), (2,3)" << std::endl;
    std::cout << "Voronoi图将平面划分为区域" << std::endl;
    std::cout << "每个区域包含到一个生成点最近的点" << std::endl;
    std::cout << "Voronoi边是到两个生成点距离相等的点的集合" << std::endl;
    std::cout << "Voronoi顶点是到三个或更多生成点距离相等的点" << std::endl;

    // ==================== 示例20: 精确计算 ====================
    std::cout << "\n=== 示例20: 精确计算（使用精确内核）=== " << std::endl;
    typedef CGAL::Exact_predicates_kernel Exact_kernel;
    typedef Exact_kernel::Point_2 Exact_point_2;
    
    Exact_point_2 ep1(0, 0.3), ep2(1, 0.6), ep3(2, 0.9);
    std::cout << "使用精确内核判断三点是否共线:" << std::endl;
    std::cout << "点(0,0.3), (1,0.6), (2,0.9): " 
              << (CGAL::collinear(ep1, ep2, ep3) ? "共线" : "不共线") << std::endl;
    std::cout << "精确内核可以避免浮点数精度问题" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   所有示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
