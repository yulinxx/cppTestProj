/**
 * @file boolean_operations.cpp
 * @brief CGAL 布尔运算示例
 *
 * 什么是 CGAL 布尔运算？
 * ====================
 * 布尔运算（Boolean Operations）是计算两个几何对象集合运算的技术。
 * 这是CAD/CAM、GIS和计算机图形学中的基础操作。
 *
 * 布尔运算的数学定义：
 * - 并集（Union）：A ∪ B = {x | x ∈ A 或 x ∈ B}
 * - 交集（Intersection）：A ∩ B = {x | x ∈ A 且 x ∈ B}
 * - 差集（Difference）：A - B = {x | x ∈ A 且 x ∉ B}
 * - 对称差（Symmetric Difference）：A ⊕ B = (A - B) ∪ (B - A)
 *
 * 布尔运算的几何解释：
 * 1. 并集：两个对象覆盖的所有区域
 * 2. 交集：两个对象重叠的区域
 * 3. 差集：一个对象减去另一个
 * 4. 对称差：只在一个对象中的区域
 *
 * 布尔运算的算法：
 * 1. 基于排列的算法：
 *    - 将两个多边形的边插入排列
 *    - 标记每个面的归属
 *    - 根据运算类型选择面
 * 2. 基于分割的算法：
 *    - 将多边形分割成简单部分
 *    - 对每个部分进行运算
 *    - 合并结果
 * 3. 基于扫描线的算法：
 *    - 使用扫描线分割区域
 *    - 计算每段的覆盖情况
 *    - 生成结果多边形
 *
 * 布尔运算的挑战：
 * 1. 数值精度：浮点误差导致错误结果
 * 2. 退化情况：共线点、重合边
 * 3. 复杂拓扑：自相交、非流形
 * 4. 性能优化：大数据量处理
 *
 * CGAL 布尔运算的特点：
 * 1. 使用精确算术避免精度问题
 * 2. 系统化处理退化情况
 * 3. 高效的数据结构
 * 4. 与排列集成
 *
 * 布尔运算的应用场景：
 * 1. CAD/CAM：零件设计、布尔建模
 * 2. 地理信息系统：空间叠加分析
 * 3. 计算机图形学：CSG建模、图像处理
 * 4. 3D打印：模型修改、支撑生成
 * 5. 机器人学：构型空间、碰撞检测
 * 6. 游戏开发：区域判断、碰撞检测
 * 7. 建筑设计：空间规划
 * 8. 电路设计：版图处理
 *
 * 本示例演示：
 * 1. 创建测试多边形
 * 2. 并集（Union）概念
 * 3. 交集（Intersection）概念
 * 4. 差集（Difference）概念
 * 5. 对称差（Symmetric Difference）概念
 * 6. CAD/CAM应用
 * 7. GIS应用
 * 8. 计算机图形学应用
 * 9. 3D打印应用
 * 10. 机器人应用
 * 11. 布尔运算的挑战
 * 12. 布尔运算的算法
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
 * @brief 辅助函数：打印多边形
 */
void printPolygon(const Polygon_2& poly, const std::string& name) {
    std::cout << name << " (" << poly.size() << " 个顶点):" << std::endl;
    for (auto it = poly.vertices_begin(); it != poly.vertices_end(); ++it) {
        std::cout << "  (" << it->x() << ", " << it->y() << ")" << std::endl;
    }
}

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 多边形布尔运算的概念
 * 2. 布尔运算的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 布尔运算示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建测试多边形 ====================
    std::cout << "\n=== 示例1: 创建测试多边形 ===" << std::endl;
    
    // 多边形 A：矩形
    Polygon_2 polyA;
    polyA.push_back(Point_2(0, 0));
    polyA.push_back(Point_2(4, 0));
    polyA.push_back(Point_2(4, 3));
    polyA.push_back(Point_2(0, 3));
    
    // 多边形 B：矩形（与 A 部分重叠）
    Polygon_2 polyB;
    polyB.push_back(Point_2(2, 1));
    polyB.push_back(Point_2(6, 1));
    polyB.push_back(Point_2(6, 4));
    polyB.push_back(Point_2(2, 4));
    
    printPolygon(polyA, "多边形 A");
    printPolygon(polyB, "多边形 B");
    
    std::cout << "\n两个多边形的位置关系:" << std::endl;
    std::cout << "  A: (0,0) 到 (4,3)" << std::endl;
    std::cout << "  B: (2,1) 到 (6,4)" << std::endl;
    std::cout << "  重叠区域: (2,1) 到 (4,3)" << std::endl;

    // ==================== 示例2: 并集（Union）====================
    std::cout << "\n=== 示例2: 并集（Union）=== " << std::endl;
    
    std::cout << "并集（A ∪ B）:" << std::endl;
    std::cout << "  定义: 属于 A 或 B 的所有点" << std::endl;
    std::cout << "  形状: L 形状" << std::endl;
    std::cout << "  面积: 面积A + 面积B - 面积(A ∩ B)" << std::endl;
    std::cout << "  计算: " << polyA.area() << " + " << polyB.area() 
              << " - " << (2.0 * 2.0) << " = " << (12.0 + 12.0 - 4.0) << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. CAD 中的合并操作" << std::endl;
    std::cout << "  2. GIS 中的区域合并" << std::endl;
    std::cout << "  3. 图像处理中的区域填充" << std::endl;

    // ==================== 示例3: 交集（Intersection）====================
    std::cout << "\n=== 示例3: 交集（Intersection）=== " << std::endl;
    
    std::cout << "交集（A ∩ B）:" << std::endl;
    std::cout << "  定义: 同时属于 A 和 B 的所有点" << std::endl;
    std::cout << "  形状: 矩形 (2,1) 到 (4,3)" << std::endl;
    std::cout << "  面积: 2 * 2 = 4" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. CAD 中的布尔运算" << std::endl;
    std::cout << "  2. GIS 中的区域重叠分析" << std::endl;
    std::cout << "  3. 碰撞检测" << std::endl;

    // ==================== 示例4: 差集（Difference）====================
    std::cout << "\n=== 示例4: 差集（Difference）=== " << std::endl;
    
    std::cout << "差集（A - B）:" << std::endl;
    std::cout << "  定义: 属于 A 但不属于 B 的所有点" << std::endl;
    std::cout << "  形状: A 减去重叠部分" << std::endl;
    std::cout << "  面积: 面积A - 面积(A ∩ B) = 12 - 4 = 8" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. CAD 中的挖孔操作" << std::endl;
    std::cout << "  2. GIS 中的区域裁剪" << std::endl;
    std::cout << "  3. 3D 打印中的模型修改" << std::endl;

    // ==================== 示例5: 对称差（Symmetric Difference）====================
    std::cout << "\n=== 示例5: 对称差（Symmetric Difference）=== " << std::endl;
    
    std::cout << "对称差（A ⊕ B）:" << std::endl;
    std::cout << "  定义: 只属于 A 或 B，但不同时属于两者的点" << std::endl;
    std::cout << "  等价于: (A ∪ B) - (A ∩ B)" << std::endl;
    std::cout << "  面积: 面积A + 面积B - 2 * 面积(A ∩ B) = 12 + 12 - 8 = 16" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 图像处理中的边缘检测" << std::endl;
    std::cout << "  2. 版本控制中的差异比较" << std::endl;
    std::cout << "  3. 网格划分中的边界检测" << std::endl;

    // ==================== 示例6: 布尔运算的应用 - CAD/CAM ====================
    std::cout << "\n=== 示例6: 布尔运算的应用 - CAD/CAM ===" << std::endl;
    
    std::cout << "CAD/CAM 中的布尔运算:" << std::endl;
    std::cout << "  1. 合并零件（Union）" << std::endl;
    std::cout << "  2. 挖孔（Difference）" << std::endl;
    std::cout << "  3. 重叠部分（Intersection）" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 创建带孔的零件" << std::endl;
    std::cout << "  2. 计算两个零件的接触区域" << std::endl;
    std::cout << "  3. 生成刀具路径" << std::endl;

    // ==================== 示例7: 布尔运算的应用 - GIS ====================
    std::cout << "\n=== 示例7: 布尔运算的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS 中的布尔运算:" << std::endl;
    std::cout << "  1. 土地利用叠加分析" << std::endl;
    std::cout << "  2. 行政区划重叠分析" << std::endl;
    std::cout << "  3. 环境影响评估" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到森林和保护区的重叠区域" << std::endl;
    std::cout << "  2. 计算洪水区域和居民区的交集" << std::endl;
    std::cout << "  3. 分析道路和生态保护区的关系" << std::endl;

    // ==================== 示例8: 布尔运算的应用 - 计算机图形学 ====================
    std::cout << "\n=== 示例8: 布尔运算的应用 - 计算机图形学 ===" << std::endl;
    
    std::cout << "计算机图形学中的布尔运算:" << std::endl;
    std::cout << "  1. CSG（Constructive Solid Geometry）建模" << std::endl;
    std::cout << "  2. 粒子系统中的碰撞处理" << std::endl;
    std::cout << "  3. 网格分割" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 创建带孔的模型" << std::endl;
    std::cout << "  2. 计算两个物体的接触区域" << std::endl;
    std::cout << "  3. 生成碎片效果" << std::endl;

    // ==================== 示例9: 布尔运算���应用 - 3D 打印 ====================
    std::cout << "\n=== 示例9: 布尔运算的应用 - 3D 打印 ===" << std::endl;
    
    std::cout << "3D 打印中的布尔运算:" << std::endl;
    std::cout << "  1. 模型组合" << std::endl;
    std::cout << "  2. 模型切割" << std::endl;
    std::cout << "  3. 支撑结构生成" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 将多个部件组合成一个模型" << std::endl;
    std::cout << "  2. 将大模型切割成小块以便打印" << std::endl;
    std::cout << "  3. 生成支撑结构" << std::endl;

    // ==================== 示例10: 布尔运算的应用 - 机器人 ====================
    std::cout << "\n=== 示例10: 布尔运算的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的布尔运算:" << std::endl;
    std::cout << "  1. 构型空间（C-Space）计算" << std::endl;
    std::cout << "  2. 可达性分析" << std::endl;
    std::cout << "  3. 碰撞检测" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 计算机器人手臂的可达区域" << std::endl;
    std::cout << "  2. 计算障碍物的碰撞区域" << std::endl;
    std::cout << "  3. 规划无碰撞路径" << std::endl;

    // ==================== 示例11: 布尔运算的挑战 ====================
    std::cout << "\n=== 示例11: 布尔运算的挑战 ===" << std::endl;
    
    std::cout << "布尔运算的挑战:" << std::endl;
    std::cout << "  1. 数值精度问题" << std::endl;
    std::cout << "  2. 退化情况处理（共线、重合）" << std::endl;
    std::cout << "  3. 复杂拓扑结构" << std::endl;
    std::cout << "  4. 性能优化" << std::endl;
    
    std::cout << "CGAL 的解决方案:" << std::endl;
    std::cout << "  1. 使用精确算术" << std::endl;
    std::cout << "  2. 系统化处理退化情况" << std::endl;
    std::cout << "  3. 高效的数据结构" << std::endl;

    // ==================== 示例12: 布尔运算的算法 ====================
    std::cout << "\n=== 示例12: 布尔运算的算法 ===" << std::endl;
    
    std::cout << "布尔运算的算法:" << std::endl;
    std::cout << "  1. 基于排列的算法" << std::endl;
    std::cout << "  2. 基于分割的算法" << std::endl;
    std::cout << "  3. 基于体素的算法" << std::endl;
    
    std::cout << "CGAL 的实现:" << std::endl;
    std::cout << "  1. 使用排列计算交点" << std::endl;
    std::cout << "  2. 标记每个面的归属" << std::endl;
    std::cout << "  3. 根据布尔运算类型选择面" << std::endl;
    std::cout << "  4. 重构结果多边形" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   布尔运算示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
