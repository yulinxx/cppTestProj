/**
 * @file point_in_polygon.cpp
 * @brief CGAL 点在多边形内判断示例
 * 
 * 点在多边形内判断是确定点是否在多边形内部的技术
 * 主要方法：
 * 1. 射线法（Ray Casting）
 * 2. 角度和法
 * 3. Winding Number 法
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
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 基础点在多边形内判断
 * 2. 点在多边形外判断
 * 3. 点在多边形边界上判断
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 点在多边形内判断示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建矩形多边形 ====================
    std::cout << "\n=== 示例1: 创建矩形多边形 ===" << std::endl;
    
    // 创建矩形多边形（逆时针方向）
    Polygon_2 rect;
    rect.push_back(Point_2(0, 0));
    rect.push_back(Point_2(4, 0));
    rect.push_back(Point_2(4, 3));
    rect.push_back(Point_2(0, 3));
    
    std::cout << "矩形多边形顶点（逆时针方向）:" << std::endl;
    std::cout << "  (0,0), (4,0), (4,3), (0,3)" << std::endl;
    std::cout << "  宽度: 4, 高度: 3" << std::endl;
    std::cout << "  面积: " << rect.area() << std::endl;

    // ==================== 示例2: 点在多边形内判断 ====================
    std::cout << "\n=== 示例2: 点在多边形内判断 ===" << std::endl;
    
    Point_2 p1(2, 1.5);  // 多边形内部
    Point_2 p2(5, 2);    // 多边形外部
    Point_2 p3(2, 0);    // 多边形边界上
    
    std::cout << "测试点:" << std::endl;
    std::cout << "  p1(2, 1.5): " << (rect.has_on_bounded_side(p1) ? "在内部" : "不在内部") << std::endl;
    std::cout << "  p2(5, 2): " << (rect.has_on_bounded_side(p2) ? "在内部" : "不在内部") << std::endl;
    std::cout << "  p3(2, 0): " << (rect.has_on_boundary(p3) ? "在边界上" : "不在边界上") << std::endl;
    
    std::cout << "\n解释:" << std::endl;
    std::cout << "  p1(2, 1.5): x=2 在 [0,4] 内，y=1.5 在 [0,3] 内，所以" << std::endl;
    std::cout << "  p2(5, 2): x=5 不在 [0,4] 内，所以" << std::endl;
    std::cout << "  p3(2, 0): y=0 在边界上，所以" << std::endl;

    // ==================== 示例3: 凹多边形的点在多边形内判断 ====================
    std::cout << "\n=== 示例3: 凹多边形的点在多边形内判断 ===" << std::endl;
    
    // 创建凹五边形（像字母 'L'）
    Polygon_2 concave;
    concave.push_back(Point_2(0, 0));
    concave.push_back(Point_2(4, 0));
    concave.push_back(Point_2(4, 2));
    concave.push_back(Point_2(2, 2));
    concave.push_back(Point_2(2, 4));
    concave.push_back(Point_2(0, 4));
    
    Point_2 p4(1, 1);  // 凹多边形内部
    Point_2 p5(3, 1);  // 凹多边形内部
    Point_2 p6(3, 3);  // 凹多边形外部
    Point_2 p7(1, 3);  // 凹多边形内部
    
    std::cout << "凹五边形顶点（像字母 'L'）:" << std::endl;
    std::cout << "  (0,0), (4,0), (4,2), (2,2), (2,4), (0,4)" << std::endl;
    
    std::cout << "\n测试点:" << std::endl;
    std::cout << "  p4(1, 1): " << (concave.has_on_bounded_side(p4) ? "在内部" : "不在内部") << std::endl;
    std::cout << "  p5(3, 1): " << (concave.has_on_bounded_side(p5) ? "在内部" : "不在内部") << std::endl;
    std::cout << "  p6(3, 3): " << (concave.has_on_bounded_side(p6) ? "在内部" : "不在内部") << std::endl;
    std::cout << "  p7(1, 3): " << (concave.has_on_bounded_side(p7) ? "在内部" : "不在内部") << std::endl;
    
    std::cout << "\n解释:" << std::endl;
    std::cout << "  p4(1, 1): 在左下角矩形内" << std::endl;
    std::cout << "  p5(3, 1): 在右下角矩形内" << std::endl;
    std::cout << "  p6(3, 3): 在凹陷区域外" << std::endl;
    std::cout << "  p7(1, 3): 在左上角矩形内" << std::endl;

    // ==================== 示例4: 多个孔的多边形 ====================
    std::cout << "\n=== 示例4: 多个孔的多边形概念 ===" << std::endl;
    
    std::cout << "多个孔的多边形:" << std::endl;
    std::cout << "  1. 外边界（逆时针）" << std::endl;
    std::cout << "  2. 孔（顺时针）" << std::endl;
    std::cout << "  3. CGAL 支持带孔的多边形" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 复杂区域的表示" << std::endl;
    std::cout << "  2. GIS 中的区域" << std::endl;
    std::cout << "  3. CAD 中的零件" << std::endl;

    // ==================== 示例5: 点在多边形内判断的应用 - GIS ====================
    std::cout << "\n=== 示例5: 点在多边形内判断的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS 中的点在多边形内判断:" << std::endl;
    std::cout << "  1. 确定坐标点在哪个区域" << std::endl;
    std::cout << "  2. 空间查询" << std::endl;
    std::cout << "  3. 地图服务" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定坐标点在哪个行政区" << std::endl;
    std::cout << "  2. 确定坐标点在哪个土地利用类型" << std::endl;
    std::cout << "  3. 确定坐标点在哪个缓冲区" << std::endl;

    // ==================== 示例6: 点在多边形内判断的应用 - 游戏 ====================
    std::cout << "\n=== 示例6: 点在多边形内判断的应用 - 游戏 ===" << std::endl;
    
    std::cout << "游戏中的点在多边形内判断:" << std::endl;
    std::cout << "  1. 确定角色在哪个区域" << std::endl;
    std::cout << "  2. 确定鼠标点击在哪个物体" << std::endl;
    std::cout << "  3. 确定子弹击中哪个物体" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定角色在哪个地图区域" << std::endl;
    std::cout << "  2. 确定鼠标点击在哪个 UI 元素" << std::endl;
    std::cout << "  3. 确定子弹击中哪个敌人" << std::endl;

    // ==================== 示例7: 点在多边形内判断的应用 - 机器人 ====================
    std::cout << "\n=== 示例7: 点在多边形内判断的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的点在多边形内判断:" << std::endl;
    std::cout << "  1. 确定机器人在哪个区域" << std::endl;
    std::cout << "  2. 确定目标点在哪个区域" << std::endl;
    std::cout << "  3. 确定路径上的点在哪个区域" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定机器人在哪个导航区域" << std::endl;
    std::cout << "  2. 确定目标点在哪个自由空间" << std::endl;
    std::cout << "  3. 确定路径上的点在哪个安全区域" << std::endl;

    // ==================== 示例8: 点在多边形内判断的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例8: 点在多边形内判断的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的点���多边形内判断:" << std::endl;
    std::cout << "  1. 确定点在哪个分割区域" << std::endl;
    std::cout << "  2. 确定点在哪个物体" << std::endl;
    std::cout << "  3. 确定点在哪个特征" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定点在哪个分割区域" << std::endl;
    std::cout << "  2. 确定点在哪个物体" << std::endl;
    std::cout << "  3. 确定点在哪个特征" << std::endl;

    // ==================== 示例9: 点在多边形内判断的应用 - 有限元分析 ====================
    std::cout << "\n=== 示例9: 点在多边形内判断的应用 - 有限元分析 ===" << std::endl;
    
    std::cout << "有限元分析中的点在多边形内判断:" << std::endl;
    std::cout << "  1. 确定点在哪个单元" << std::endl;
    std::cout << "  2. 插值计算" << std::endl;
    std::cout << "  3. 结果可视化" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定应力点在哪个单元" << std::endl;
    std::cout << "  2. 确定位移点在哪个单元" << std::endl;
    std::cout << "  3. 确定温度点在哪个单元" << std::endl;

    // ==================== 示例10: 点在多边形内判断的应用 - CFD ====================
    std::cout << "\n=== 示例10: 点在多边形内判断的应用 - CFD ===" << std::endl;
    
    std::cout << "CFD 中的点在多边形内判断:" << std::endl;
    std::cout << "  1. 确定点在哪个单元" << std::endl;
    std::cout << "  2. 插值计算" << std::endl;
    std::cout << "  3. 后处理" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 确定速度点在哪个单元" << std::endl;
    std::cout << "  2. 确定压力点在哪个单元" << std::endl;
    std::cout << "  3. 确定温度点在哪个单元" << std::endl;

    // ==================== 示例11: 点在多边形内判断的算法 ====================
    std::cout << "\n=== 示例11: 点在多边形内判断的算法 ===" << std::endl;
    
    std::cout << "点在多边形内判断的算法:" << std::endl;
    std::cout << "  1. 射线法（Ray Casting）" << std::endl;
    std::cout << "  2. 角度和法" << std::endl;
    std::cout << "  3. Winding Number 法" << std::endl;
    
    std::cout << "射线法原理:" << std::endl;
    std::cout << "  1. 从点向右发射射线" << std::endl;
    std::cout << "  2. 计算射线与多边形边的交点数" << std::endl;
    std::cout << "  3. 交点数为奇数则在内部，偶数则在外部" << std::endl;

    // ==================== 示例12: 点在多边形内判断的挑战 ====================
    std::cout << "\n=== 示例12: 点在多边形内判断的挑战 ===" << std::endl;
    
    std::cout << "点在多边形内判断的挑战:" << std::endl;
    std::cout << "  1. 边界情况处理" << std::endl;
    std::cout << "  2. 浮点精度问题" << std::endl;
    std::cout << "  3. 复杂多边形处理" << std::endl;
    std::cout << "  4. 多个孔的多边形" << std::endl;
    
    std::cout << "CGAL 的解决方案:" << std::endl;
    std::cout << "  1. 使用精确算术" << std::endl;
    std::cout << "  2. 系统化处理边界情况" << std::endl;
    std::cout << "  3. 高效的数据结构" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   点在多边形内判断示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
