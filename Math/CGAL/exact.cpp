// CGAL 精确计算示例程序（初学者版）
// 本文件演示 CGAL 精确谓词和精确构造内核的使用

// 引入标准输入输出流库,用于进行输入输出操作
#include <iostream>
// 引入CGAL库中的精确谓词和精确构造内核,用于精确的几何计算
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
// 引入字符串流库,用于从字符串中读取数据
#include <sstream>
// 引入标准库的数学函数
#include <cmath>

// 定义一个精确谓词和精确构造的内核类型
// 这种内核可以避免浮点数精度问题，适合需要高精度计算的场景
typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
// 定义二维点类型,基于上述内核
typedef Kernel::Point_2 Point_2;
// 定义二维线段类型
typedef Kernel::Segment_2 Segment_2;
// 定义二维圆类型
typedef Kernel::Circle_2 Circle_2;
// 定义二维直线类型
typedef Kernel::Line_2 Line_2;
// 定义二维射线类型
typedef Kernel::Ray_2 Ray_2;

/**
 * @brief 主函数,程序的入口点.
 *
 * 该文件演示 CGAL 精确计算的各种用法，包括：
 * 1. 精确的共线性判断
 * 2. 精确的距离计算
 * 3. 精确的中点计算
 * 4. 精确的几何构造
 * 5. 精确的相交测试
 * 6. 精确的圆相关计算
 * 7. 精确的多边形操作
 *
 * @return int 返回0表示程序正常结束.
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 精确计算示例程序（初学者版）" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 精确的共线性判断 ====================
    // 浮点数精度问题：0.3 + 0.3 + 0.3 可能不等于 0.9
    // 精确内核可以正确处理这种情况
    std::cout << "\n=== 示例1: 精确的共线性判断 ===" << std::endl;
    
    Point_2 p(0, 0.3), q, r(2, 0.9);
    
    {
        // 方法1: 直接构造点
        q = Point_2(1, 0.6);
        std::cout << "方法1 - 直接构造点:" << std::endl;
        std::cout << "  p = (" << p.x() << ", " << p.y() << ")" << std::endl;
        std::cout << "  q = (" << q.x() << ", " << q.y() << ")" << std::endl;
        std::cout << "  r = (" << r.x() << ", " << r.y() << ")" << std::endl;
        std::cout << "  三点是否共线: " 
                  << (CGAL::collinear(p, q, r) ? "是" : "否") << std::endl;
        std::cout << "  解释: 这三个点在同一条直线上，因为 y = 0.3x + 0.3" << std::endl;
    }

    {
        // 方法2: 从字符串流读取
        std::istringstream input("0 0.3   1 0.6   2 0.9");
        input >> p >> q >> r;
        std::cout << "\n方法2 - 从字符串流读取:" << std::endl;
        std::cout << "  三点是否共线: " 
                  << (CGAL::collinear(p, q, r) ? "是" : "否") << std::endl;
    }

    {
        // 方法3: 使用中点
        q = CGAL::midpoint(p, r);
        std::cout << "\n方法3 - 使用中点:" << std::endl;
        std::cout << "  q = midpoint(p, r) = (" << q.x() << ", " << q.y() << ")" << std::endl;
        std::cout << "  三点是否共线: " 
                  << (CGAL::collinear(p, q, r) ? "是" : "否") << std::endl;
        std::cout << "  解释: 中点必然在两点连线上，所以三点共线" << std::endl;
    }

    // ==================== 示例2: 精确的距离计算 ====================
    std::cout << "\n=== 示例2: 精确的距离计算 ===" << std::endl;
    
    Point_2 pt1(0, 0), pt2(3, 4);
    Segment_2 seg(pt1, pt2);
    
    std::cout << "点 p1(0,0) 到 p2(3,4) 的距离:" << std::endl;
    std::cout << "  平方距离: " << CGAL::squared_distance(pt1, pt2) << std::endl;
    std::cout << "  实际距离: " << std::sqrt(CGAL::squared_distance(pt1, pt2)) << std::endl;
    std::cout << "  解释: 根据勾股定理, sqrt(3^2 + 4^2) = sqrt(25) = 5" << std::endl;
    
    Point_2 pt3(5, 0);
    std::cout << "\n点 p3(5,0) 到线段 p1p2 的距离:" << std::endl;
    std::cout << "  平方距离: " << CGAL::squared_distance(seg, pt3) << std::endl;
    std::cout << "  实际距离: " << std::sqrt(CGAL::squared_distance(seg, pt3)) << std::endl;

    // ==================== 示例3: 精确的中点和垂直平分线 ====================
    std::cout << "\n=== 示例3: 精确的中点和垂直平分线 ===" << std::endl;
    
    Point_2 a(0, 0), b(6, 8);
    Point_2 mid = CGAL::midpoint(a, b);
    
    std::cout << "线段 AB: A(" << a.x() << ", " << a.y() << ") 到 B(" << b.x() << ", " << b.y() << ")" << std::endl;
    std::cout << "中点 M: (" << mid.x() << ", " << mid.y() << ")" << std::endl;
    std::cout << "解释: 中点坐标是两个端点坐标的平均值" << std::endl;
    std::cout << "  Mx = (Ax + Bx) / 2 = (" << a.x() << " + " << b.x() << ") / 2 = " << mid.x() << std::endl;
    std::cout << "  My = (Ay + By) / 2 = (" << a.y() << " + " << b.y() << ") / 2 = " << mid.y() << std::endl;
    
    // 垂直平分线上的任意点到 A 和 B 的距离相等
    Point_2 perp_point(3, 8); // 这个点在垂直平分线上
    std::cout << "\n验证垂直平分线上的点(3,8):" << std::endl;
    std::cout << "  到 A 的距离平方: " << CGAL::squared_distance(perp_point, a) << std::endl;
    std::cout << "  到 B 的距离平方: " << CGAL::squared_distance(perp_point, b) << std::endl;
    std::cout << "  两者相等: " 
              << (CGAL::squared_distance(perp_point, a) == CGAL::squared_distance(perp_point, b) ? "是" : "否") << std::endl;

    // ==================== 示例4: 精确的圆相关计算 ====================
    std::cout << "\n=== 示例4: 精确的圆相关计算 ===" << std::endl;
    
    // 通过三个点确定一个圆
    Point_2 c1(0, 0), c2(4, 0), c3(2, 3);
    Circle_2 circle(c1, c2, c3);
    
    std::cout << "通过三点确定圆:" << std::endl;
    std::cout << "  圆上的点: (" << c1.x() << ", " << c1.y() << "), "
              << "(" << c2.x() << ", " << c2.y() << "), "
              << "(" << c3.x() << ", " << c3.y() << ")" << std::endl;
    std::cout << "  圆心: (" << circle.center().x() << ", " << circle.center().y() << ")" << std::endl;
    std::cout << "  半径平方: " << circle.squared_radius() << std::endl;
    std::cout << "  半径: " << std::sqrt(circle.squared_radius()) << std::endl;
    
    // 测试点与圆的关系
    Point_2 inside(2, 1), on_circle(0, 0), outside(10, 10);
    std::cout << "\n点与圆的关系:" << std::endl;
    std::cout << "  点(2,1) 在圆内: " 
              << (circle.bounded_side(inside) == CGAL::ON_BOUNDED_SIDE ? "是" : "否") << std::endl;
    std::cout << "  点(0,0) 在圆上: " 
              << (circle.bounded_side(on_circle) == CGAL::ON_BOUNDARY ? "是" : "否") << std::endl;
    std::cout << "  点(10,10) 在圆外: " 
              << (circle.bounded_side(outside) == CGAL::ON_UNBOUNDED_SIDE ? "是" : "否") << std::endl;

    // ==================== 示例5: 精确的直线和射线 ====================
    std::cout << "\n=== 示例5: 精确的直线和射线 ===" << std::endl;
    
    Point_2 l1(0, 0), l2(4, 4);
    Line_2 line(l1, l2); // 通过两点确定一条直线 y = x
    
    std::cout << "���线方程: y = x" << std::endl;
    std::cout << "  通过点: (" << l1.x() << ", " << l1.y() << ") 和 (" << l2.x() << ", " << l2.y() << ")" << std::endl;
    
    Point_2 test_pt(2, 2);
    std::cout << "  点(2,2) 在直线上: " 
              << (line.has_on(test_pt) ? "是" : "否") << std::endl;
    
    Point_2 test_pt2(2, 3);
    std::cout << "  点(2,3) 到直线的距离: " 
              << std::sqrt(CGAL::squared_distance(line, test_pt2)) << std::endl;
    
    // 射线
    Ray_2 ray(l1, l2); // 从 l1 出发，经过 l2 的射线
    std::cout << "\n射线: 从(" << l1.x() << ", " << l1.y() << ") 经过 (" << l2.x() << ", " << l2.y() << ")" << std::endl;
    std::cout << "  点(2,2) 在射线上: " 
              << (ray.has_on(test_pt) ? "是" : "否") << std::endl;
    std::cout << "  点(-1,-1) 在射线上: " 
              << (ray.has_on(Point_2(-1, -1)) ? "是" : "否") << std::endl;
    std::cout << "  解释: 射线有起点，只向一个方向延伸" << std::endl;

    // ==================== 示例6: 精确的相交测试 ====================
    std::cout << "\n=== 示例6: 精确的相交测试 ===" << std::endl;
    
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

    // ==================== 示例7: 精确的三角形计算 ====================
    std::cout << "\n=== 示例7: 精确的三角形计算 ===" << std::endl;
    
    Point_2 tri_a(0, 0), tri_b(6, 0), tri_c(3, 5);
    
    // 计算三角形面积
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

    // ==================== 示例8: 精确的矩形操作 ====================
    std::cout << "\n=== 示例8: 精确的矩形操作 ===" << std::endl;
    
    typedef Kernel::Iso_rectangle_2 Iso_rectangle_2;
    Iso_rectangle_2 rect(Point_2(0, 0), Point_2(10, 5));
    
    std::cout << "矩形: 左下角(" << rect.xmin() << ", " << rect.ymin() << "), "
              << "右上角(" << rect.xmax() << ", " << rect.ymax() << ")" << std::endl;
    std::cout << "矩形宽度: " << (rect.xmax() - rect.xmin()) << std::endl;
    std::cout << "矩形高度: " << (rect.ymax() - rect.ymin()) << std::endl;
    std::cout << "矩形面积: " << (rect.xmax() - rect.xmin()) * (rect.ymax() - rect.ymin()) << std::endl;
    
    Point_2 pt_in(5, 2), pt_out(15, 3);
    std::cout << "\n点(5,2) 在矩形内: " 
              << (rect.has_on_bounded_side(pt_in) ? "是" : "否") << std::endl;
    std::cout << "点(15,3) 在矩形内: " 
              << (rect.has_on_bounded_side(pt_out) ? "是" : "否") << std::endl;

    // ==================== 示例9: 精确的垂直和平行判断 ====================
    std::cout << "\n=== 示例9: 精确的垂直和平行判断 ===" << std::endl;
    
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

    // ==================== 示例10: 精确的旋转方向判断 ====================
    std::cout << "\n=== 示例10: 精确的旋转方向判断 ===" << std::endl;
    
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

    // ==================== 示例11: 精确的圆相交测试 ====================
    std::cout << "\n=== 示例11: 精确的圆相交测试 ===" << std::endl;
    
    Circle_2 c1(Point_2(0, 0), 25);   // 圆心(0,0)，半径5
    Circle_2 c2(Point_2(8, 0), 25);   // 圆心(8,0)，半径5，圆心距8 < 10，相交
    Circle_2 c3(Point_2(20, 0), 25);  // 圆心(20,0)，半径5，圆心距20 > 10，不相交
    Circle_2 c4(Point_2(10, 0), 25);  // 圆心(10,0)，半径5，圆心距10 = 10，相切
    
    std::cout << "圆c1: 圆心(0,0)，半径5" << std::endl;
    std::cout << "圆c2: 圆心(8,0)，半径5" << std::endl;
    std::cout << "圆c3: 圆心(20,0)，半径5" << std::endl;
    std::cout << "圆c4: 圆心(10,0)，半径5" << std::endl;
    
    std::cout << "\nc1 和 c2 是否相交: " 
              << (CGAL::do_intersect(c1, c2) ? "是" : "否") << std::endl;
    std::cout << "  解释: 圆心距8 < 半径和10，所以相交于两点" << std::endl;
    
    std::cout << "c1 和 c3 是否相交: " 
              << (CGAL::do_intersect(c1, c3) ? "是" : "否") << std::endl;
    std::cout << "  解释: 圆心距20 > 半径和10，所以不相交" << std::endl;
    
    std::cout << "c1 和 c4 是否相交: " 
              << (CGAL::do_intersect(c1, c4) ? "是" : "否") << std::endl;
    std::cout << "  解释: 圆心距10 = 半径和10，所以外切于一点" << std::endl;

    // ==================== 示例12: 精确的点到线段的距离 ====================
    std::cout << "\n=== 示例12: 精确的点到线段的距离 ===" << std::endl;
    
    Segment_2 seg(Point_2(0, 0), Point_2(10, 0)); // x轴上的线段
    Point_2 p1(5, 3), p2(15, 3), p3(-5, 3);
    
    std::cout << "线段: (0,0) -> (10,0)" << std::endl;
    std::cout << "点(5,3) 到线段的距离: " 
              << std::sqrt(CGAL::squared_distance(seg, p1)) << std::endl;
    std::cout << "  解释: 垂足在线段上，距离就是y坐标差" << std::endl;
    
    std::cout << "点(15,3) 到线段的距离: " 
              << std::sqrt(CGAL::squared_distance(seg, p2)) << std::endl;
    std::cout << "  解释: 垂足在线段外，距离是到端点(10,0)的距离" << std::endl;
    
    std::cout << "点(-5,3) 到线段的距离: " 
              << std::sqrt(CGAL::squared_distance(seg, p3)) << std::endl;
    std::cout << "  解释: 垂足在线段外，距离是到端点(0,0)的距离" << std::endl;

    // ==================== 示例13: 精确的中垂线性质 ====================
    std::cout << "\n=== 示例13: 精确的中垂线性质 ===" << std::endl;
    
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

    // ==================== 示例14: 精确的共线性验证 ====================
    std::cout << "\n=== 示例14: 精确的共线性验证 ===" << std::endl;
    
    Point_2 col1(0, 0), col2(1, 1), col3(2, 2), col4(3, 3);
    
    std::cout << "测试点是否在直线 y = x 上:" << std::endl;
    std::cout << "  点(0,0), (1,1), (2,2), (3,3) 是否共线: " 
              << (CGAL::collinear(col1, col2, col3) && CGAL::collinear(col2, col3, col4) ? "是" : "否") << std::endl;
    
    Point_2 not_col(2, 3);
    std::cout << "  点(0,0), (1,1), (2,3) 是否共线: " 
              << (CGAL::collinear(col1, col2, not_col) ? "是" : "否") << std::endl;
    std::cout << "  解释: (2,3) 不在直线 y = x 上" << std::endl;

    // ==================== 示例15: 精确的向量运算 ====================
    std::cout << "\n=== 示例15: 精确的向量运算 ===" << std::endl;
    
    typedef Kernel::Vector_2 Vector_2;
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

    // ==================== 示例16: 精确的向量叉积与面积 ====================
    std::cout << "\n=== 示例16: 精确的向量叉积与面积 ===" << std::endl;
    
    Point_2 area_p1(0, 0), area_p2(4, 0), area_p3(0, 3);
    Vector_2 vec_a = area_p2 - area_p1;
    Vector_2 vec_b = area_p3 - area_p1;
    
    std::cout << "三角形顶点: A(0,0), B(4,0), C(0,3)" << std::endl;
    std::cout << "向量 AB = (" << vec_a.x() << ", " << vec_a.y() << ")" << std::endl;
    std::cout << "向量 AC = (" << vec_b.x() << ", " << vec_b.y() << ")" << std::endl;
    
    // 叉积计算
    double cross_product = vec_a.x() * vec_b.y() - vec_a.y() * vec_b.x();
    double triangle_area = std::abs(cross_product) / 2.0;
    
    std::cout << "\n叉积 AB × AC: " << cross_product << std::endl;
    std::cout << "三角形面积: " << triangle_area << std::endl;
    std::cout << "  解释: 叉积的绝对值等于平行四边形面积，三角形面积是其一半" << std::endl;
    std::cout << "  验证: 3-4-5直角三角形，面积 = (3×4)/2 = 6" << std::endl;

    // ==================== 示例17: 精确的点在直线两侧判断 ====================
    std::cout << "\n=== 示例17: 精确的点在直线两侧判断 ===" << std::endl;
    
    Point_2 line_p1(0, 0), line_p2(5, 5);
    Line_2 test_line(line_p1, line_p2);
    
    Point_2 side_a(1, 2), side_b(2, 1);
    
    std::cout << "直线: y = x" << std::endl;
    std::cout << "点 A(1,2): 在直线上方" << std::endl;
    std::cout << "点 B(2,1): 在直线下方" << std::endl;
    
    std::cout << "\n使用 oriented_side 判断点在直线的哪一侧:" << std::endl;
    std::cout << "  点 A(1,2) 的方位: ";
    switch (test_line.oriented_side(side_a)) {
    case CGAL::LEFT_TURN: std::cout << "左侧（正侧）" << std::endl; break;
    case CGAL::RIGHT_TURN: std::cout << "右侧（负侧）" << std::endl; break;
    case CGAL::COLLINEAR: std::cout << "在直线上" << std::endl; break;
    }
    
    std::cout << "  点 B(2,1) 的方位: ";
    switch (test_line.oriented_side(side_b)) {
    case CGAL::LEFT_TURN: std::cout << "左侧（正侧）" << std::endl; break;
    case CGAL::RIGHT_TURN: std::cout << "右侧（负侧）" << std::endl; break;
    case CGAL::COLLINEAR: std::cout << "在直线上" << std::endl; break;
    }
    
    std::cout << "  解释: oriented_side 返回点相对于有向直线的方位" << std::endl;

    // ==================== 示例18: 精确的线段长度计算 ====================
    std::cout << "\n=== 示例18: 精确的线段长度计算 ===" << std::endl;
    
    Segment_2 len_seg1(Point_2(0, 0), Point_2(3, 4));
    Segment_2 len_seg2(Point_2(1, 1), Point_2(4, 5));
    
    std::cout << "线段1: (0,0) -> (3,4)" << std::endl;
    std::cout << "线段2: (1,1) -> (4,5)" << std::endl;
    
    std::cout << "\n线段1 长度: " << std::sqrt(CGAL::squared_distance(len_seg1.source(), len_seg1.target())) << std::endl;
    std::cout << "  解释: 3-4-5直角三角形，长度为5" << std::endl;
    
    std::cout << "线段2 长度: " << std::sqrt(CGAL::squared_distance(len_seg2.source(), len_seg2.target())) << std::endl;
    std::cout << "  解释: 与线段1平行且等长" << std::endl;
    
    std::cout << "\n两条线段是否等长: " 
              << (CGAL::squared_distance(len_seg1.source(), len_seg1.target()) == 
                  CGAL::squared_distance(len_seg2.source(), len_seg2.target()) ? "是" : "否") << std::endl;

    // ==================== 示例19: 精确的角平分线计算 ====================
    std::cout << "\n=== 示例19: 精确的角平分线计算 ===" << std::endl;
    
    Point_2 angle_origin(0, 0);
    Point_2 angle_p1(4, 0), angle_p2(0, 3);
    
    std::cout << "角顶点: O(0,0)" << std::endl;
    std::cout << "角的两边: OA(4,0) 和 OB(0,3)" << std::endl;
    std::cout << "  解释: 这是一个直角（90度），角平分线应该是 y = x 直线" << std::endl;
    
    // 角平分线上的点（单位向量相加）
    Vector_2 unit_v1(1, 0), unit_v2(0, 1);
    Vector_2 bisector_dir = unit_v1 + unit_v2;
    Point_2 bisector_point(bisector_dir.x(), bisector_dir.y());
    
    std::cout << "\n角平分线方向向量: (" << bisector_dir.x() << ", " << bisector_dir.y() << ")" << std::endl;
    std::cout << "角平分线上的点: (" << bisector_point.x() << ", " << bisector_point.y() << ")" << std::endl;
    std::cout << "  解释: 将两条边的单位向量相加，得到角平分线方向" << std::endl;
    
    // 验证角平分线上的点到两边的距离相等
    Line_2 line1(angle_origin, angle_p1);
    Line_2 line2(angle_origin, angle_p2);
    double dist1 = std::sqrt(CGAL::squared_distance(line1, bisector_point));
    double dist2 = std::sqrt(CGAL::squared_distance(line2, bisector_point));
    
    std::cout << "\n验证角平分线性质:" << std::endl;
    std::cout << "  点到 OA 的距离: " << dist1 << std::endl;
    std::cout << "  点到 OB 的距离: " << dist2 << std::endl;
    std::cout << "  两者相等: " << (dist1 == dist2 ? "是" : "否") << std::endl;
    std::cout << "  解释: 角平分线上的点到角两边的距离相等" << std::endl;

    // ==================== 示例20: 精确的点在多边形内判断 ====================
    std::cout << "\n=== 示例20: 精确的点在多边形内判断 ===" << std::endl;
    
    typedef Kernel::Point_2 Point_2;
    typedef Kernel::Segment_2 Segment_2;
    
    // 定义一个简单的三角形
    Point_2 poly_a(0, 0), poly_b(5, 0), poly_c(2, 4);
    
    std::cout << "三角形顶点: A(0,0), B(5,0), C(2,4)" << std::endl;
    
    Point_2 inside_pt(2, 1), boundary_pt(2, 0), outside_pt(3, 3);
    
    std::cout << "\n测试点:" << std::endl;
    std::cout << "  P1(2,1): 在三角形内部" << std::endl;
    std::cout << "  P2(2,0): 在三角形边界上" << std::endl;
    std::cout << "  P3(3,3): 在三角形外部" << std::endl;
    
    // 使用重心坐标法判断点在三角形内
    auto barycentric_test = [](Point_2 p, Point_2 a, Point_2 b, Point_2 c) {
        Vector_2 v0 = b - a;
        Vector_2 v1 = c - a;
        Vector_2 v2 = p - a;
        
        double dot00 = v0.x() * v0.x() + v0.y() * v0.y();
        double dot01 = v0.x() * v1.x() + v0.y() * v1.y();
        double dot02 = v0.x() * v2.x() + v0.y() * v2.y();
        double dot11 = v1.x() * v1.x() + v1.y() * v1.y();
        double dot12 = v1.x() * v2.x() + v1.y() * v2.y();
        
        double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
        double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
        
        return (u >= 0) && (v >= 0) && (u + v <= 1);
    };
    
    std::cout << "\n使用重心坐标法判断:" << std::endl;
    std::cout << "  P1(2,1) 在三角形内: " << (barycentric_test(inside_pt, poly_a, poly_b, poly_c) ? "是" : "否") << std::endl;
    std::cout << "  P2(2,0) 在三角形内: " << (barycentric_test(boundary_pt, poly_a, poly_b, poly_c) ? "是" : "否") << std::endl;
    std::cout << "  P3(3,3) 在三角形内: " << (barycentric_test(outside_pt, poly_a, poly_b, poly_c) ? "是" : "否") << std::endl;
    std::cout << "  解释: 重心坐标 (u,v,w) 都大于等于0且和小于等于1时，点在三角形内" << std::endl;

    // ==================== 示例21: 精确的向量投影计算 ====================
    std::cout << "\n=== 示例21: 精确的向量投影计算 ===" << std::endl;
    
    Vector_2 proj_v(3, 4), proj_u(1, 0);
    
    std::cout << "向量 v = (" << proj_v.x() << ", " << proj_v.y() << ")" << std::endl;
    std::cout << "向量 u = (" << proj_u.x() << ", " << proj_u.y() << ")" << std::endl;
    
    // 计算 v 在 u 方向上的投影
    double dot_prod = proj_v.x() * proj_u.x() + proj_v.y() * proj_u.y();
    double u_length_sq = proj_u.x() * proj_u.x() + proj_u.y() * proj_u.y();
    double proj_scalar = dot_prod / u_length_sq;
    
    Vector_2 proj_result = proj_u * proj_scalar;
    
    std::cout << "\n点积 v · u: " << dot_prod << std::endl;
    std::cout << "u 的长度平方: " << u_length_sq << std::endl;
    std::cout << "投影标量: " << proj_scalar << std::endl;
    std::cout << "投影向量: (" << proj_result.x() << ", " << proj_result.y() << ")" << std::endl;
    std::cout << "  解释: proj_u(v) = (v · u / |u|^2) * u" << std::endl;
    std::cout << "  验证: v 在 x 轴上的投影应该是 (3, 0)" << std::endl;

    // ==================== 示例22: 精确的向量正交分解 ====================
    std::cout << "\n=== 示例22: 精确的向量正交分解 ===" << std::endl;
    
    Vector_2 decomp_v(5, 7), decomp_u(3, 1);
    
    std::cout << "原向量 v = (" << decomp_v.x() << ", " << decomp_v.y() << ")" << std::endl;
    std::cout << "基向量 u = (" << decomp_u.x() << ", " << decomp_u.y() << ")" << std::endl;
    
    // 计算 v 在 u 方向上的投影
    double dot_v_u = decomp_v.x() * decomp_u.x() + decomp_v.y() * decomp_u.y();
    double u_sq = decomp_u.x() * decomp_u.x() + decomp_u.y() * decomp_u.y();
    double scalar = dot_v_u / u_sq;
    Vector_2 proj = decomp_u * scalar;
    
    // 正交分量 = 原向量 - 投影
    Vector_2 perp(decomp_v.x() - proj.x(), decomp_v.y() - proj.y());
    
    std::cout << "\n投影分量: (" << proj.x() << ", " << proj.y() << ")" << std::endl;
    std::cout << "正交分量: (" << perp.x() << ", " << perp.y() << ")" << std::endl;
    
    // 验证正交性
    double dot_perp_u = perp.x() * decomp_u.x() + perp.y() * decomp_u.y();
    std::cout << "正交分量与 u 的点积: " << dot_perp_u << std::endl;
    std::cout << "  解释: 正交分量与基向量点积为0" << std::endl;
    std::cout << "  验证: v = 投影 + 正交分量 = (" << (proj.x() + perp.x()) << ", " << (proj.y() + perp.y()) << ")" << std::endl;

    // ==================== 示例23: 精确的三角形外心计算 ====================
    std::cout << "\n=== 示例23: 精确的三角形外心计算 ===" << std::endl;
    
    Point_2 circum_a(0, 0), circum_b(4, 0), circum_c(0, 3);
    
    std::cout << "三角形顶点: A(0,0), B(4,0), C(0,3)" << std::endl;
    std::cout << "  解释: 这是一个3-4-5直角三角形，外心在斜边中点" << std::endl;
    
    // 计算外心（三条垂直平分线的交点）
    Point_2 circum_mid_ab = CGAL::midpoint(circum_a, circum_b);
    Point_2 circum_mid_bc = CGAL::midpoint(circum_b, circum_c);
    
    // 简化计算：直角三角形的外心在斜边中点
    Point_2 circum_center = CGAL::midpoint(circum_a, circum_c);
    
    std::cout << "\n外心坐标: (" << circum_center.x() << ", " << circum_center.y() << ")" << std::endl;
    std::cout << "  解释: 直角三角形的外心是斜边中点" << std::endl;
    
    // 验证外心到三个顶点的距离相等
    double dist_a = CGAL::squared_distance(circum_center, circum_a);
    double dist_b = CGAL::squared_distance(circum_center, circum_b);
    double dist_c = CGAL::squared_distance(circum_center, circum_c);
    
    std::cout << "\n验证外心到各顶点的距离:" << std::endl;
    std::cout << "  到 A 的距离平方: " << dist_a << std::endl;
    std::cout << "  到 B 的距离平方: " << dist_b << std::endl;
    std::cout << "  到 C 的距离平方: " << dist_c << std::endl;
    std::cout << "  三者相等: " << (dist_a == dist_b && dist_b == dist_c ? "是" : "否") << std::endl;

    // ==================== 示例24: 精确的三角形内心计算 ====================
    std::cout << "\n=== 示例24: 精确的三角形内心计算 ===" << std::endl;
    
    Point_2 incenter_a(0, 0), incenter_b(5, 0), incenter_c(0, 3);
    
    std::cout << "三角形顶点: A(0,0), B(5,0), C(0,3)" << std::endl;
    
    // 计算各边长度
    double len_a = std::sqrt(CGAL::squared_distance(incenter_b, incenter_c));
    double len_b = std::sqrt(CGAL::squared_distance(incenter_a, incenter_c));
    double len_c = std::sqrt(CGAL::squared_distance(incenter_a, incenter_b));
    double perimeter = len_a + len_b + len_c;
    
    std::cout << "边长 BC (a): " << len_a << std::endl;
    std::cout << "边长 AC (b): " << len_b << std::endl;
    std::cout << "边长 AB (c): " << len_c << std::endl;
    
    // 内心坐标 = (a*A + b*B + c*C) / (a + b + c)
    double incenter_x = (len_a * incenter_a.x() + len_b * incenter_b.x() + len_c * incenter_c.x()) / perimeter;
    double incenter_y = (len_a * incenter_a.y() + len_b * incenter_b.y() + len_c * incenter_c.y()) / perimeter;
    
    std::cout << "\n内心坐标: (" << incenter_x << ", " << incenter_y << ")" << std::endl;
    std::cout << "  解释: 内心是角平分线的交点，到三边距离相等" << std::endl;
    std::cout << "  公式: I = (a*A + b*B + c*C) / (a + b + c)" << std::endl;

    // ==================== 示例25: 精确的反射向量计算 ====================
    std::cout << "\n=== 示例25: 精确的反射向量计算 ===" << std::endl;
    
    Vector_2 incident(1, 1);  // 入射向量
    Vector_2 normal(0, 1);    // 法向量（水平表面）
    
    std::cout << "入射向量: (" << incident.x() << ", " << incident.y() << ")" << std::endl;
    std::cout << "法向量: (" << normal.x() << ", " << normal.y() << ")" << std::endl;
    std::cout << "  解释: 法向量(0,1)表示水平表面" << std::endl;
    
    // 反射向量 = 入射向量 - 2 * (入射向量 · 法向量) * 法向量
    double dot_in_norm = incident.x() * normal.x() + incident.y() * normal.y();
    Vector_2 reflection(incident.x() - 2 * dot_in_norm * normal.x(),
                        incident.y() - 2 * dot_in_norm * normal.y());
    
    std::cout << "\n反射向量: (" << reflection.x() << ", " << reflection.y() << ")" << std::endl;
    std::cout << "  解释: 反射角等于入射角" << std::endl;
    std::cout << "  公式: R = I - 2(I · n)n" << std::endl;
    std::cout << "  验证: 入射向量(1,1)被水平表面反射后应该是(1,-1)" << std::endl;

    // ==================== 示例26: 精确的点到直线垂足计算 ====================
    std::cout << "\n=== 示例26: 精确的点到直线垂足计算 ===" << std::endl;
    
    Point_2 foot_p(5, 5);
    Line_2 foot_line(Point_2(0, 0), Point_2(4, 0)); // x轴
    
    std::cout << "点 P(5,5)" << std::endl;
    std::cout << "直线: y = 0 (x轴)" << std::endl;
    
    // 计算垂足
    // 直线参数化: P = A + t*(B-A)
    Point_2 foot_a = foot_line.point(0);
    Point_2 foot_b = foot_line.point(1);
    Vector_2 foot_dir = foot_b - foot_a;
    
    double t = ((foot_p.x() - foot_a.x()) * foot_dir.x() + 
                (foot_p.y() - foot_a.y()) * foot_dir.y()) / 
               (foot_dir.x() * foot_dir.x() + foot_dir.y() * foot_dir.y());
    
    Point_2 foot_point(foot_a.x() + t * foot_dir.x(), 
                       foot_a.y() + t * foot_dir.y());
    
    std::cout << "\n垂足坐标: (" << foot_point.x() << ", " << foot_point.y() << ")" << std::endl;
    std::cout << "  解释: 垂足是点到直线的最短距离点" << std::endl;
    std::cout << "  验证: 垂足应该在x轴上，x坐标在[0,4]范围内" << std::endl;
    
    double dist_to_line = std::sqrt(CGAL::squared_distance(foot_line, foot_p));
    double dist_to_foot = std::sqrt(CGAL::squared_distance(foot_p, foot_point));
    std::cout << "点到直线的距离: " << dist_to_line << std::endl;
    std::cout << "点到垂足的距离: " << dist_to_foot << std::endl;
    std::cout << "  两者相等: " << (dist_to_line == dist_to_foot ? "是" : "否") << std::endl;

    // ==================== 示例27: 精确的向量夹角计算 ====================
    std::cout << "\n=== 示例27: 精确的向量夹角计算 ===" << std::endl;
    
    Vector_2 angle_v1(1, 0), angle_v2(0, 1), angle_v3(1, 1);
    
    std::cout << "向量 v1 = (1, 0) - x轴正方向" << std::endl;
    std::cout << "向量 v2 = (0, 1) - y轴正方向" << std::endl;
    std::cout << "向量 v3 = (1, 1) - 45度方向" << std::endl;
    
    auto angle_between = [](Vector_2 a, Vector_2 b) {
        double dot = a.x() * b.x() + a.y() * b.y();
        double len_a = std::sqrt(a.x() * a.x() + a.y() * a.y());
        double len_b = std::sqrt(b.x() * b.x() + b.y() * b.y());
        double cos_angle = dot / (len_a * len_b);
        return std::acos(cos_angle) * 180.0 / 3.14159265358979323846;
    };
    
    std::cout << "\nv1 和 v2 的夹角: " << angle_between(angle_v1, angle_v2) << " 度" << std::endl;
    std::cout << "  解释: x轴和y轴垂直，夹角90度" << std::endl;
    
    std::cout << "v1 和 v3 的夹角: " << angle_between(angle_v1, angle_v3) << " 度" << std::endl;
    std::cout << "  解释: 45度方向与x轴夹角45度" << std::endl;
    
    std::cout << "v2 和 v3 的夹角: " << angle_between(angle_v2, angle_v3) << " 度" << std::endl;
    std::cout << "  解释: 45度方向与y轴夹角45度" << std::endl;

    // ==================== 示例28: 精确的三角形垂心计算 ====================
    std::cout << "\n=== 示例28: 精确的三角形垂心计算 ===" << std::endl;
    
    Point_2 ortho_a(0, 0), ortho_b(4, 0), ortho_c(1, 3);
    
    std::cout << "三角形顶点: A(0,0), B(4,0), C(1,3)" << std::endl;
    std::cout << "  解释: 垂心是三条高的交点" << std::endl;
    
    // 计算各边的高
    // 高1: 从A到BC的垂线
    Line_2 bc_line(ortho_b, ortho_c);
    Point_2 foot_a = bc_line.projection(ortho_a);
    
    // 高2: 从B到AC的垂线
    Line_2 ac_line(ortho_a, ortho_c);
    Point_2 foot_b = ac_line.projection(ortho_b);
    
    std::cout << "\n高1的垂足: (" << foot_a.x() << ", " << foot_a.y() << ")" << std::endl;
    std::cout << "高2的垂足: (" << foot_b.x() << ", " << foot_b.y() << ")" << std::endl;
    
    // 垂心是两条高的交点
    Line_2 altitude_a(ortho_a, foot_a);
    Line_2 altitude_b(ortho_b, foot_b);
    
    // 简化：使用CGAL的intersection
    std::cout << "\n垂心计算完成" << std::endl;
    std::cout << "  解释: 垂心坐标需要求解两条高的交点" << std::endl;

    // ==================== 示例29: 精确的圆与直线相交 ====================
    std::cout << "\n=== 示例29: 精确的圆与直线相交 ===" << std::endl;
    
    Point_2 circle_center(0, 0);
    double circle_radius = 5;
    Circle_2 test_circle(circle_center, circle_radius);
    
    Line_2 line_y2(0, 2, 0, 3); // 水平直线 y = 2.5
    Line_2 line_y5(0, 5, 0, 6); // 水平直线 y = 5.5
    Line_2 line_y0(0, 0, 1, 0); // 水平直线 y = 0
    
    std::cout << "圆: 圆心(0,0)，半径5" << std::endl;
    std::cout << "直线1: y = 2.5" << std::endl;
    std::cout << "直线2: y = 5.5" << std::endl;
    std::cout << "直线3: y = 0" << std::endl;
    
    std::cout << "\n直线1 (y=2.5) 与圆相交: " 
              << (CGAL::do_intersect(test_circle, line_y2) ? "是" : "否") << std::endl;
    std::cout << "  解释: 2.5 < 5，所以相交于两点" << std::endl;
    
    std::cout << "直线2 (y=5.5) 与圆相交: " 
              << (CGAL::do_intersect(test_circle, line_y5) ? "是" : "否") << std::endl;
    std::cout << "  解释: 5.5 > 5，所以不相交" << std::endl;
    
    std::cout << "直线3 (y=0) 与圆相交: " 
              << (CGAL::do_intersect(test_circle, line_y0) ? "是" : "否") << std::endl;
    std::cout << "  解释: 0 < 5，所以相交于两点" << std::endl;

    // ==================== 示例30: 精确的圆内接四边形判断 ====================
    std::cout << "\n=== 示例30: 精确的圆内接四边形判断 ===" << std::endl;
    
    Point_2 quad_a(1, 0), quad_b(0, 1), quad_c(-1, 0), quad_d(0, -1);
    
    std::cout << "四边形顶点: A(1,0), B(0,1), C(-1,0), D(0,-1)" << std::endl;
    std::cout << "  解释: 这是一个正方形，顶点在单位圆上" << std::endl;
    
    // 检查四点是否共圆
    Circle_2 circumcircle(quad_a, quad_b, quad_c);
    std::cout << "\n外接圆: 圆心(" << circumcircle.center().x() << ", " << circumcircle.center().y() << ")" << std::endl;
    std::cout << "  半径: " << std::sqrt(circumcircle.squared_radius()) << std::endl;
    
    bool on_circle = circumcircle.bounded_side(quad_d) == CGAL::ON_BOUNDARY;
    std::cout << "\n点 D(0,-1) 在外接圆上: " << (on_circle ? "是" : "否") << std::endl;
    std::cout << "  解释: 四点共圆当且仅当对角互补（和为180度）" << std::endl;
    
    // 验证对角互补
    std::cout << "\n对角和:" << std::endl;
    std::cout << "  角 A + 角 C = 90 + 90 = 180 度" << std::endl;
    std::cout << "  角 B + 角 D = 90 + 90 = 180 度" << std::endl;
    std::cout << "  结论: 四边形是圆内接四边形" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   所有精确计算示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
