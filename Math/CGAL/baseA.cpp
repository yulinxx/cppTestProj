// CGAL 入门基础_点云侠的博客-CSDN博客
// https://blog.csdn.net/qq_36686437/article/details/125552700

#include <iostream>
#include <CGAL/Simple_cartesian.h> // 笛卡尔坐标相关头文件
#include <CGAL/Exact_predicates_exact_constructions_kernel.h> // 精确内核
#include <CGAL/convex_hull_2.h> // 凸包算法

typedef CGAL::Simple_cartesian<double> Kernel; // 内核使用双精度浮点数作为该点的笛卡尔坐标
typedef Kernel::Point_2 Point_2;               // 二维点
typedef Kernel::Segment_2 Segment_2;           // 二维线段
typedef Kernel::Circle_2 Circle_2;             // 二维圆
typedef Kernel::Iso_rectangle_2 Iso_rectangle_2; // 二维矩形
typedef Kernel::Vector_2 Vector_2;             // 二维向量
typedef Kernel::Line_2 Line_2;                 // 二维直线

// ==================== 辅助函数 ====================

// 计算三角形面积（使用叉积）
double triangleArea(const Point_2& a, const Point_2& b, const Point_2& c) {
    Vector_2 v1 = b - a;
    Vector_2 v2 = c - a;
    return std::abs(v1.x() * v2.y() - v1.y() * v2.x()) / 2.0;
}

// 判断点是否在矩形内
bool pointInRectangle(const Point_2& p, const Iso_rectangle_2& rect) {
    return (p.x() >= rect.xmin() && p.x() <= rect.xmax() &&
            p.y() >= rect.ymin() && p.y() <= rect.ymax());
}

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 计算几何库示例程序（初学者版）" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 基础点和线段操作 ====================
    // 这是原始示例，保留作为基础参考
    std::cout << "\n=== 示例1: 基础点和线段操作 ===" << std::endl;
    Point_2 p(1, 1), q(10, 10);
    std::cout << "p = " << p << std::endl;
    std::cout << "q = " << q.x() << " " << q.y() << std::endl;
    std::cout << "两点之间的平方距离: " << CGAL::squared_distance(p, q) << std::endl;
    
    Segment_2 s(p, q);
    Point_2 m(5, 9);
    std::cout << "点m到线段pq的平方距离: " << CGAL::squared_distance(s, m) << std::endl;
    std::cout << "p和q的中点为: " << CGAL::midpoint(p, q) << std::endl;

    // ==================== 示例2: 三点位置关系判断 ====================
    std::cout << "\n=== 示例2: 三点位置关系判断 ===" << std::endl;
    Point_2 p1(0, 0), p2(5, 5), p3(3, 4);
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

    // ==================== 示例3: 三角形和外接圆 ====================
    std::cout << "\n=== 示例3: 三角形和外接圆 ===" << std::endl;
    Point_2 t1(0, 0), t2(4, 0), t3(2, 3);
    std::cout << "三角形顶点: " << t1 << ", " << t2 << ", " << t3 << std::endl;
    
    // 计算三角形的外接圆（通过三个点的圆）
    Circle_2 circ(t1, t2, t3);
    std::cout << "外接圆圆心: " << circ.center() << std::endl;
    std::cout << "外接圆半径平方: " << circ.squared_radius() << std::endl;
    std::cout << "外接圆半径: " << std::sqrt(circ.squared_radius()) << std::endl;

    // ==================== 示例4: 点与线段的相对位置 ====================
    std::cout << "\n=== 示例4: 点与线段的相对位置 ===" << std::endl;
    Point_2 seg_start(0, 0), seg_end(10, 0);
    Segment_2 horizontal_seg(seg_start, seg_end);
    Point_2 above(5, 3), below(5, -3), on_segment(5, 0);
    
    std::cout << "测试线段: (" << seg_start << ") 到 (" << seg_end << ")" << std::endl;
    
    std::cout << "点(5,3) 相对于线段的位置: ";
    switch (CGAL::orientation(horizontal_seg, above)) {
    case CGAL::LEFT_TURN: std::cout << "左侧" << std::endl; break;
    case CGAL::RIGHT_TURN: std::cout << "右侧" << std::endl; break;
    case CGAL::COLLINEAR: std::cout << "共线（在线上）" << std::endl; break;
    }
    
    std::cout << "点(5,-3) 相对于线段的位置: ";
    switch (CGAL::orientation(horizontal_seg, below)) {
    case CGAL::LEFT_TURN: std::cout << "左侧" << std::endl; break;
    case CGAL::RIGHT_TURN: std::cout << "右侧" << std::endl; break;
    case CGAL::COLLINEAR: std::cout << "共线（在线上）" << std::endl; break;
    }

    // ==================== 示例5: 线段相交测试 ====================
    std::cout << "\n=== 示例5: 线段相交测试 ===" << std::endl;
    Segment_2 s1(Point_2(0, 0), Point_2(10, 10));
    Segment_2 s2(Point_2(0, 10), Point_2(10, 0));
    Segment_2 s3(Point_2(20, 20), Point_2(30, 30));
    
    std::cout << "线段 s1(0,0)->(10,10) 和 s2(0,10)->(10,0) 相交: "
              << (CGAL::do_intersect(s1, s2) ? "是" : "否") << std::endl;
    std::cout << "线段 s1(0,0)->(10,10) 和 s3(20,20)->(30,30) 相交: "
              << (CGAL::do_intersect(s1, s3) ? "是" : "否") << std::endl;

    // ==================== 示例6: 二维向量运算 ====================
    std::cout << "\n=== 示例6: 二维向量运算 ===" << std::endl;
    Point_2 origin(0, 0);
    Point_2 v1(3, 4), v2(1, 2);
    
    // 向量加法
    Point_2 sum = v1 + v2;
    std::cout << "向量 v1(3,4) + v2(1,2) = (" << sum.x() << ", " << sum.y() << ")" << std::endl;
    
    // 向量点积（内积）
    double dot_product = v1.x() * v2.x() + v1.y() * v2.y();
    std::cout << "向量 v1 和 v2 的点积: " << dot_product << std::endl;
    
    // 向量叉积（外积）- 在二维中结果是一个标量
    double cross_product = v1.x() * v2.y() - v1.y() * v2.x();
    std::cout << "向量 v1 和 v2 的叉积: " << cross_product << std::endl;
    std::cout << "叉积绝对值 = 平行四边形面积: " << std::abs(cross_product) << std::endl;

    // ==================== 示例7: 圆和点的关系 ====================
    std::cout << "\n=== 示例7: 圆和点的关系 ===" << std::endl;
    Circle_2 circle(Point_2(0, 0), 100); // 圆心在原点，半径平方为100（半径=10）
    Point_2 p_inside(3, 4), p_on(10, 0), p_outside(15, 0);
    
    std::cout << "圆: 圆心(0,0), 半径=10" << std::endl;
    
    std::cout << "点(3,4) 与圆的关系: ";
    switch (circle.bounded_side(p_inside)) {
    case CGAL::ON_BOUNDED_SIDE: std::cout << "在圆内" << std::endl; break;
    case CGAL::ON_BOUNDARY: std::cout << "在圆上" << std::endl; break;
    case CGAL::ON_UNBOUNDED_SIDE: std::cout << "在圆外" << std::endl; break;
    }
    
    std::cout << "点(10,0) 与圆的关系: ";
    switch (circle.bounded_side(p_on)) {
    case CGAL::ON_BOUNDED_SIDE: std::cout << "在圆内" << std::endl; break;
    case CGAL::ON_BOUNDARY: std::cout << "在圆上" << std::endl; break;
    case CGAL::ON_UNBOUNDED_SIDE: std::cout << "在圆外" << std::endl; break;
    }

    // ==================== 示例8: 矩形操作 ====================
    std::cout << "\n=== 示例8: 矩形操作 ===" << std::endl;
    Iso_rectangle_2 rect(Point_2(0, 0), Point_2(10, 5));
    
    std::cout << "矩形左下角: " << rect.min() << std::endl;
    std::cout << "矩形右上角: " << rect.max() << std::endl;
    std::cout << "矩形宽度: " << rect.xmax() - rect.xmin() << std::endl;
    std::cout << "矩形高度: " << rect.ymax() - rect.ymin() << std::endl;
    
    Point_2 pt_in(5, 2), pt_out(15, 3);
    std::cout << "点(5,2) 在矩形内: " 
              << (rect.has_on_bounded_side(pt_in) ? "是" : "否") << std::endl;
    std::cout << "点(15,3) 在矩形内: " 
              << (rect.has_on_bounded_side(pt_out) ? "是" : "否") << std::endl;

    // ==================== 示例9: 垂直平分线 ====================
    std::cout << "\n=== 示例9: 垂直平分线 ===" << std::endl;
    Point_2 a(0, 0), b(6, 4);
    std::cout << "线段端点: " << a << " 到 " << b << std::endl;
    
    Point_2 mid = CGAL::midpoint(a, b);
    std::cout << "中点: " << mid << std::endl;
    std::cout << "垂直平分线是到两个端点距离相等的点的集合" << std::endl;

    // ==================== 示例10: 三角形的特殊点 ====================
    std::cout << "\n=== 示例10: 三角形的特殊点 ===" << std::endl;
    Point_2 tri_a(0, 0), tri_b(6, 0), tri_c(3, 5);
    
    // 重心（Centroid）- 三条中线的交点
    Point_2 centroid((tri_a.x() + tri_b.x() + tri_c.x()) / 3.0,
                     (tri_a.y() + tri_b.y() + tri_c.y()) / 3.0);
    std::cout << "重心（Centroid）: " << centroid << std::endl;
    std::cout << "重心是三个顶点坐标的平均值" << std::endl;
    
    // 外心（Circumcenter）- 外接圆的圆心
    Circle_2 tri_circum(tri_a, tri_b, tri_c);
    std::cout << "外心（Circumcenter）: " << tri_circum.center() << std::endl;
    std::cout << "外心是三角形三条边的垂直平分线的交点" << std::endl;

    // ==================== 示例11: 点到直线的距离 ====================
    std::cout << "\n=== 示例11: 点到直线的距离 ===" << std::endl;
    Line_2 x_axis(Point_2(0, 0), Point_2(10, 0)); // x轴
    Point_2 test_point(5, 3);
    
    std::cout << "点(5,3) 到 x轴的距离: " 
              << std::sqrt(CGAL::squared_distance(x_axis, test_point)) << std::endl;

    // ==================== 示例12: 三角形面积 ====================
    std::cout << "\n=== 示例12: 三角形面积 ===" << std::endl;
    Point_2 tri_p1(0, 0), tri_p2(6, 0), tri_p3(3, 4);
    double area = triangleArea(tri_p1, tri_p2, tri_p3);
    std::cout << "三角形顶点: " << tri_p1 << ", " << tri_p2 << ", " << tri_p3 << std::endl;
    std::cout << "三角形面积: " << area << std::endl;
    std::cout << "使用叉积公式计算: 面积 = |(B-A) × (C-A)| / 2" << std::endl;

    // ==================== 示例13: 平行四边形法则 ====================
    std::cout << "\n=== 示例13: 平行四边形法则 ===" << std::endl;
    Point_2 par_a(0, 0), par_b(4, 1), par_c(1, 3);
    Point_2 par_d = par_b + par_c - par_a;
    
    std::cout << "平行四边形顶点:" << std::endl;
    std::cout << "A: " << par_a << std::endl;
    std::cout << "B: " << par_b << std::endl;
    std::cout << "C: " << par_c << std::endl;
    std::cout << "D: " << par_d << std::endl;
    std::cout << "对角线 AC 和 BD 在中点相交" << std::endl;

    // ==================== 示例14: 圆的相交测试 ====================
    std::cout << "\n=== 示例14: 圆的相交测试 ===" << std::endl;
    Circle_2 c1(Point_2(0, 0), 25);   // 半径5
    Circle_2 c2(Point_2(8, 0), 25);   // 半径5，圆心距8 < 10，相交
    Circle_2 c3(Point_2(20, 0), 25);  // 半径5，圆心距20 > 10，不相交
    
    std::cout << "圆c1(0,0,r=5) 和 c2(8,0,r=5) 相交: " 
              << (CGAL::do_intersect(c1, c2) ? "是" : "否") << std::endl;
    std::cout << "圆c1(0,0,r=5) 和 c3(20,0,r=5) 相交: " 
              << (CGAL::do_intersect(c1, c3) ? "是" : "否") << std::endl;

    // ==================== 示例15: 垂直和平行关系 ====================
    std::cout << "\n=== 示例15: 垂直和平行关系 ===" << std::endl;
    Segment_2 seg_h(Point_2(0, 0), Point_2(4, 0)); // 水平
    Segment_2 seg_v(Point_2(2, 0), Point_2(2, 4)); // 垂直
    
    std::cout << "水平线段和垂直线段垂直: " 
              << (CGAL::is_perpendicular(seg_h, seg_v) ? "是" : "否") << std::endl;

    // ==================== 示例16: 旋转方向测试 ====================
    std::cout << "\n=== 示例16: 旋转方向测试 ===" << std::endl;
    Point_2 py(0, 1), origin2(0, 0), px(1, 0);
    
    std::cout << "从 y轴正方向 到 x轴正方向 是: ";
    switch (CGAL::orientation(py, origin2, px)) {
    case CGAL::LEFT_TURN: std::cout << "左转（逆时针）" << std::endl; break;
    case CGAL::RIGHT_TURN: std::cout << "右转（顺时针）" << std::endl; break;
    case CGAL::COLLINEAR: std::cout << "共线" << std::endl; break;
    }

    // ==================== 示例17: 多边形三角剖分 ====================
    std::cout << "\n=== 示例17: 多边形三角剖分 ===" << std::endl;
    Point_2 poly_p1(0, 0), poly_p2(4, 0), poly_p3(4, 3), poly_p4(0, 3);
    std::cout << "矩形可以被分成两个三角形:" << std::endl;
    std::cout << "三角形1: " << poly_p1 << ", " << poly_p2 << ", " << poly_p3 << std::endl;
    std::cout << "三角形2: " << poly_p1 << ", " << poly_p3 << ", " << poly_p4 << std::endl;
    std::cout << "矩形总面积: " << (4.0 * 3.0) << std::endl;
    std::cout << "每个三角形面积: " << (4.0 * 3.0 / 2.0) << std::endl;

    // ==================== 示例18: 凸包概念 ====================
    std::cout << "\n=== 示例18: 凸包概念 ===" << std::endl;
    Point_2 points[] = {Point_2(0,0), Point_2(4,0), Point_2(4,4), 
                        Point_2(0,4), Point_2(2,2), Point_2(1,3)};
    std::cout << "输入点: (0,0), (4,0), (4,4), (0,4), (2,2), (1,3)" << std::endl;
    std::cout << "凸包是包含所有点的最小凸多边形" << std::endl;
    std::cout << "在这个例子中，凸包的顶点是: (0,0), (4,0), (4,4), (0,4)" << std::endl;
    std::cout << "点(2,2)和(1,3)在凸包内部" << std::endl;

    // ==================== 示例19: Voronoi 图概念 ====================
    std::cout << "\n=== 示例19: Voronoi 图概念 ===" << std::endl;
    Point_2 sites[] = {Point_2(0,0), Point_2(4,0), Point_2(2,3)};
    std::cout << "生成点（Site）: (0,0), (4,0), (2,3)" << std::endl;
    std::cout << "Voronoi 图将平面划分为区域" << std::endl;
    std::cout << "每个区域包含到一个生成点最近的点" << std::endl;
    std::cout << "Voronoi 边是到两个生成点距离相等的点的集合" << std::endl;

    // ==================== 示例20: 精确计算 ====================
    std::cout << "\n=== 示例20: 精确计算（使用精确内核）=== " << std::endl;
    typedef CGAL::Exact_predicates_exact_constructions_kernel Exact_kernel;
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
