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

    std::cout << "\n========================================" << std::endl;
    std::cout << "   所有精确计算示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
