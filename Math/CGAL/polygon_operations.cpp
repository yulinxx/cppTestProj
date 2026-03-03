/**
 * @file polygon_operations.cpp
 * @brief CGAL 多边形操作示例
 *
 * 什么是 CGAL 多边形操作？
 * ====================
 * 多边形操作是处理二维多边形的各种几何运算。
 * 多边形是由线段连接的顶点序列定义的封闭区域。
 *
 * 多边形的数学定义：
 * - 简单多边形：边不相交（除相邻边外）的多边形
 * - 凸多边形：任意两点的连线完全在多边形内部
 * - 凹多边形：至少有一个内角大于180度的多边形
 * - 多边形的面积：使用鞋带公式计算
 *
 * 多边形的重要性质：
 * 1. 方向性：顶点顺序决定多边形的方向（顺时针/逆时针）
 * 2. 面积：可正可负，取决于顶点方向
 * 3. 边界：多边形的边序列
 * 4. 内部：被边界包围的区域
 *
 * CGAL 提供的多边形操作：
 * 1. 多边形构造：创建各种形状的多边形
 * 2. 面积计算：使用精确算术
 * 3. 方向判断：判断顶点顺序
 * 4. 点与多边形关系：内部、边界、外部
 * 5. 边界框计算：快速包围盒
 * 6. 多边形简化：减少顶点数
 * 7. 多边形三角化：分割成三角形
 * 8. 布尔运算：并、交、差
 * 9. 多边形偏移：创建缓冲区
 *
 * 多边形操作的应用场景：
 * 1. CAD/CAM：零件设计、布尔运算
 * 2. 地理信息系统：区域分析、缓冲区计算
 * 3. 计算机图形学：填充、裁剪、碰撞检测
 * 4. 机器人学：构型空间表示
 * 5. 游戏开发：区域判断、碰撞检测
 * 6. 图像处理：区域生长、分割
 * 7. 医学影像：器官建模、区域分析
 *
 * 多边形布尔运算：
 * 1. 并集（Union）：两个多边形覆盖的所有区域
 * 2. 交集（Intersection）：两个多边形重叠的区域
 * 3. 差集（Difference）：一个减去另一个
 * 4. 对称差（Symmetric Difference）：只在一个中的区域
 *
 * 本示例演示：
 * 1. 多边形的创建和基本操作
 * 2. 多边形面积计算
 * 3. 多边形方向判断
 * 4. 点与多边形的关系
 * 5. 多边形的边界框
 * 6. 多边形的简化概念
 * 7. 多边形的三角化概念
 * 8. 多边形的布尔运算概念
 * 9. 多边形的偏移概念
 * 10. 旋转卡壳应用
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polygon_2.h>

// 定义内核和多边形类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef CGAL::Polygon_2<Kernel> Polygon_2;

/**
 * @brief 辅助函数：打印多边形顶点
 */
void printPolygon(const Polygon_2& poly, const std::string& name) {
    std::cout << name << " 顶点数: " << poly.size() << std::endl;
    std::cout << "  顶点:" << std::endl;
    for (auto it = poly.vertices_begin(); it != poly.vertices_end(); ++it) {
        std::cout << "    (" << it->x() << ", " << it->y() << ")" << std::endl;
    }
}

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 多边形的创建和基本操作
 * 2. 多边形面积计算
 * 3. 多边形方向判断
 * 4. 点与多边形的关系
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 多边形操作示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建多边形 ====================
    std::cout << "\n=== 示例1: 创建多边形 ===" << std::endl;
    
    // 创建一个矩形多边形（顺时针方向）
    Polygon_2 rect;
    rect.push_back(Point_2(0, 0));
    rect.push_back(Point_2(4, 0));
    rect.push_back(Point_2(4, 3));
    rect.push_back(Point_2(0, 3));
    
    printPolygon(rect, "矩形");
    
    // 创建一个三角形多边形（逆时针方向）
    Polygon_2 tri;
    tri.push_back(Point_2(2, 1));
    tri.push_back(Point_2(5, 1));
    tri.push_back(Point_2(3.5, 4));
    
    printPolygon(tri, "三角形");

    // ==================== 示例2: 多边形面积计算 ====================
    std::cout << "\n=== 示例2: 多边形面积计算 ===" << std::endl;
    
    double rect_area = rect.area();
    double tri_area = tri.area();
    
    std::cout << "矩形面积: " << rect_area << std::endl;
    std::cout << "  计算公式: 宽度 * 高度 = 4 * 3 = 12" << std::endl;
    
    std::cout << "三角形面积: " << tri_area << std::endl;
    std::cout << "  计算公式: 底 * 高 / 2 = 3 * 3 / 2 = 4.5" << std::endl;

    // ==================== 示例3: 多边形方向判断 ====================
    std::cout << "\n=== 示例3: 多边形方向判断 ===" << std::endl;
    
    std::cout << "矩形方向: " 
              << (rect.is_counterclockwise_oriented() ? "逆时针" : "顺时针") << std::endl;
    std::cout << "三角形方向: " 
              << (tri.is_counterclockwise_oriented() ? "逆时针" : "顺时针") << std::endl;
    
    std::cout << "CGAL 通常期望多边形为逆时针方向" << std::endl;

    // ==================== 示例4: 点与多边形的关系 ====================
    std::cout << "\n=== 示例4: 点与多边形的关系 ===" << std::endl;
    
    Point_2 p1(2, 1.5);  // 矩形内部
    Point_2 p2(5, 2);    // 矩形外部
    Point_2 p3(4, 1.5);  // 矩形边界上
    
    std::cout << "点(2, 1.5) 与矩形的关系: " << std::endl;
    std::cout << "  在内部: " << (rect.has_on_bounded_side(p1) ? "是" : "否") << std::endl;
    std::cout << "  在边界: " << (rect.has_on_boundary(p1) ? "是" : "否") << std::endl;
    std::cout << "  在闭包: " << (rect.has_on_non_oriented_boundary(p1) ? "是" : "否") << std::endl;
    
    std::cout << "点(5, 2) 与矩形的关系: " << std::endl;
    std::cout << "  在内部: " << (rect.has_on_bounded_side(p2) ? "是" : "否") << std::endl;
    std::cout << "  在边界: " << (rect.has_on_boundary(p2) ? "是" : "否") << std::endl;
    
    std::cout << "点(4, 1.5) 与矩形的关系: " << std::endl;
    std::cout << "  在内部: " << (rect.has_on_bounded_side(p3) ? "是" : "否") << std::endl;
    std::cout << "  在边界: " << (rect.has_on_boundary(p3) ? "是" : "否") << std::endl;

    // ==================== 示例5: 多边形的边界框 ====================
    std::cout << "\n=== 示例5: 多边形的边界框 ===" << std::endl;
    
    auto bbox = rect.bbox();
    std::cout << "矩形的边界框:" << std::endl;
    std::cout << "  xmin: " << bbox.xmin() << std::endl;
    std::cout << "  xmax: " << bbox.xmax() << std::endl;
    std::cout << "  ymin: " << bbox.ymin() << std::endl;
    std::cout << "  ymax: " << bbox.ymax() << std::endl;
    std::cout << "  应用: 快速碰撞检测、空间索引" << std::endl;

    // ==================== 示例6: 多边形的简化 ====================
    std::cout << "\n=== 示例6: 多边形的简化概念 ===" << std::endl;
    
    std::cout << "多边形简化（减少顶点数）:" << std::endl;
    std::cout << "  1. Douglas-Peucker 算法" << std::endl;
    std::cout << "  2. 保持形状特征" << std::endl;
    std::cout << "  3. 减少存储空间和计算时间" << std::endl;
    
    // 创建一个有多个顶点的多边形
    Polygon_2 complex_poly;
    complex_poly.push_back(Point_2(0, 0));
    complex_poly.push_back(Point_2(1, 0.1));
    complex_poly.push_back(Point_2(2, 0.2));
    complex_poly.push_back(Point_2(3, 0.1));
    complex_poly.push_back(Point_2(4, 0));
    complex_poly.push_back(Point_2(4, 3));
    complex_poly.push_back(Point_2(0, 3));
    
    std::cout << "原始多边形顶点数: " << complex_poly.size() << std::endl;
    std::cout << "  可以简化为矩形（4个顶点）" << std::endl;

    // ==================== 示例7: 多边形的三角化 ====================
    std::cout << "\n=== 示例7: 多边形的三角化概念 ===" << std::endl;
    
    std::cout << "将多边形分割成三角形:" << std::endl;
    std::cout << "  1. 矩形可以分成 2 个三角形" << std::endl;
    std::cout << "  2. 凸多边形可以分成 (n-2) 个三角形" << std::endl;
    std::cout << "  3. 凹多边形需要先分割成凸片" << std::endl;
    
    std::cout << "矩形三角化结果:" << std::endl;
    std::cout << "  三角形1: (0,0), (4,0), (4,3)" << std::endl;
    std::cout << "  三角形2: (0,0), (4,3), (0,3)" << std::endl;

    // ==================== 示例8: 多边形的布尔运算概念 ====================
    std::cout << "\n=== 示例8: 多边形的布尔运算概念 ===" << std::endl;
    
    std::cout << "多边形布尔运算:" << std::endl;
    std::cout << "  1. 并集（Union）: 两个多边形覆盖的区域" << std::endl;
    std::cout << "  2. 交集（Intersection）: 两个多边形重叠的区域" << std::endl;
    std::cout << "  3. 差集（Difference）: 一个减去另一个" << std::endl;
    std::cout << "  4. 对称差（Symmetric Difference）: 只在一个多边形中的区域" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  CAD/CAM 布尔运算" << std::endl;
    std::cout << "  GIS 空间叠加分析" << std::endl;
    std::cout << "  计算机图形学的 CSG 建模" << std::endl;

    // ==================== 示例9: 多边形的偏移（缓冲区） ====================
    std::cout << "\n=== 示例9: 多边形的偏移（缓冲区）概念 ===" << std::endl;
    
    std::cout << "多边形偏移（Offset）:" << std::endl;
    std::cout << "  1. 扩展多边形边界" << std::endl;
    std::cout << "  2. 创建缓冲区区域" << std::endl;
    std::cout << "  3. 用于道路缓冲区、安全区域等" << std::endl;
    
    std::cout << "矩形(0,0)-(4,3) 的偏移(距离1):" << std::endl;
    std::cout << "  新矩形: (-1,-1) 到 (5,4)" << std::endl;
    std::cout << "  边角会变成圆弧" << std::endl;

    // ==================== 示例10: 多边形的旋转卡壳应用 ====================
    std::cout << "\n=== 示例10: 多边形的旋转卡壳应用 ===" << std::endl;
    
    std::cout << "旋转卡壳（Rotating Calipers）算法:" << std::endl;
    std::cout << "  1. 计算凸多边形的直径（最远点对）" << std::endl;
    std::cout << "  2. 计算凸多边形的宽度" << std::endl;
    std::cout << "  3. 计算凸多边形的最小面积外接矩形" << std::endl;
    std::cout << "  4. 时间复杂度: O(n)" << std::endl;
    
    // 计算矩形的直径
    double rect_diameter = std::sqrt(4.0 * 4.0 + 3.0 * 3.0);
    std::cout << "矩形的直径（对角线）: " << rect_diameter << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   多边形操作示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
