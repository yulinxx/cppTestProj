/**
 * @file offset_polygon.cpp
 * @brief CGAL 多边形偏移（Offset）示例
 * 
 * 多边形偏移是创建多边形的等距线（缓冲区）
 * 应用场景：
 * 1. GIS 中的缓冲区分析
 * 2. CAD 中的偏移操作
 * 3. 机器人路径规划
 * 4. 字体设计
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
 * 1. 多边形偏移的概念
 * 2. 多边形偏移的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 多边形偏移（Offset）示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建测试多边形 ====================
    std::cout << "\n=== 示例1: 创建测试多边形 ===" << std::endl;
    
    // 创建一个矩形多边形
    Polygon_2 rect;
    rect.push_back(Point_2(0, 0));
    rect.push_back(Point_2(4, 0));
    rect.push_back(Point_2(4, 3));
    rect.push_back(Point_2(0, 3));
    
    std::cout << "原始多边形（矩形）:" << std::endl;
    std::cout << "  顶点: (0,0), (4,0), (4,3), (0,3)" << std::endl;
    std::cout << "  宽度: 4, 高度: 3" << std::endl;
    std::cout << "  面积: " << rect.area() << std::endl;

    // ==================== 示例2: 多边形偏移的概念 ====================
    std::cout << "\n=== 示例2: 多边形偏移的概念 ===" << std::endl;
    
    std::cout << "多边形偏移（Offset / Buffer）:" << std::endl;
    std::cout << "  定义: 创建多边形的等距线" << std::endl;
    std::cout << "  参数: 偏移距离（正数向外，负数向内）" << std::endl;
    
    std::cout << "\n偏移距离为 1 的结果:" << std::endl;
    std::cout << "  外部偏移: (-1,-1) 到 (5,4)" << std::endl;
    std::cout << "  内部偏移: (1,1) 到 (3,2)" << std::endl;
    std::cout << "  边角: 变成圆弧（半径为偏移距离）" << std::endl;

    // ==================== 示例3: 多边形偏移的应用 - GIS 缓冲区分析 ====================
    std::cout << "\n=== 示例3: 多边形偏移的应用 - GIS 缓冲区分析 ===" << std::endl;
    
    std::cout << "GIS 中的缓冲区分析:" << std::endl;
    std::cout << "  1. 创建道路的缓冲区（安全区域）" << std::endl;
    std::cout << "  2. 创建河流的缓冲区（保护区）" << std::endl;
    std::cout << "  3. 创建设施的缓冲区（服务范围）" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 道路缓冲区 50 米" << std::endl;
    std::cout << "  2. 河流缓冲区 100 米" << std::endl;
    std::cout << "  3. 学校缓冲区 1 公里" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 环境保护" << std::endl;
    std::cout << "  2. 城市规划" << std::endl;
    std::cout << "  3. 应急响应" << std::endl;

    // ==================== 示例4: 多边形偏移的应用 - CAD 偏移操作 ====================
    std::cout << "\n=== 示例4: 多边形偏移的应用 - CAD 偏移操作 ===" << std::endl;
    
    std::cout << "CAD 中的偏移操作:" << std::endl;
    std::cout << "  1. 创建平行线" << std::endl;
    std::cout << "  2. 创建等距轮廓" << std::endl;
    std::cout << "  3. 创建偏移面" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 墙壁的内外轮廓" << std::endl;
    std::cout << "  2. 机械零件的偏移面" << std::endl;
    std::cout << "  3. PCB 的铜箔轮廓" << std::endl;

    // ==================== 示例5: 多边形偏移的应用 - 机器人路径规划 ====================
    std::cout << "\n=== 示例5: 多边形偏移的应用 - 机器人路径规划 ===" << std::endl;
    
    std::cout << "机器人路径规划中的偏移:" << std::endl;
    std::cout << "  1. 障碍物的偏移（安全距离）" << std::endl;
    std::cout << "  2. 创建自由空间" << std::endl;
    std::cout << "  3. 规划安全路径" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 机器人宽度 1 米，障碍物偏移 0.5 米" << std::endl;
    std::cout << "  2. 创建安全路径（远离障碍物）" << std::endl;
    std::cout << "  3. 规划最优路径" << std::endl;

    // ==================== 示例6: 多边形偏移的应用 - 字体设计 ====================
    std::cout << "\n=== 示例6: 多边形偏移的应用 - 字体设计 ===" << std::endl;
    
    std::cout << "字体设计中的偏移:" << std::endl;
    std::cout << "  1. 字符轮廓的偏移（描边）" << std::endl;
    std::cout << "  2. 字符的变形效果" << std::endl;
    std::cout << "  3. 字符的动画效果" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 字符描边效果" << std::endl;
    std::cout << "  2. 字符的收缩动画" << std::endl;
    std::cout << "  3. 字符的膨胀效果" << std::endl;

    // ==================== 示例7: 多边形偏移的应用 - 网格生成 ====================
    std::cout << "\n=== 示例7: 多边形偏移的应用 - 网格生成 ===" << std::endl;
    
    std::cout << "网格生成中的偏移:" << std::endl;
    std::cout << "  1. 创建边界层网格" << std::endl;
    std::cout << "  2. 创建多层网格" << std::endl;
    std::cout << "  3. 创建结构化网格" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 有限元分析的边界层网格" << std::endl;
    std::cout << "  2. CFD 的边界层网格" << std::endl;
    std::cout << "  3. 结构优化的多层网格" << std::endl;

    // ==================== 示例8: 多边形偏移的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例8: 多边形偏移的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的偏移:" << std::endl;
    std::cout << "  1. 形状的膨胀和腐蚀" << std::endl;
    std::cout << "  2. 形态学操作" << std::endl;
    std::cout << "  3. 图像分割" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 膨胀操作（扩大前景区域）" << std::endl;
    std::cout << "  2. 腐蚀操作（缩小前景区域）" << std::endl;
    std::cout << "  3. 开闭操作" << std::endl;

    // ==================== 示例9: 多边形偏移的应用 - 3D 打印 ====================
    std::cout << "\n=== 示例9: 多边形偏移的应用 - 3D 打印 ===" << std::endl;
    
    std::cout << "3D 打印中的偏移:" << std::endl;
    std::cout << "  1. 切片软件的路径规划" << std::endl;
    std::cout << "  2. 打印路径的偏移" << std::endl;
    std::cout << "  3. 壁厚控制" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 外壁偏移" << std::endl;
    std::cout << "  2. 内壁偏移" << std::endl;
    std::cout << "  3. 填充路径" << std::endl;

    // ==================== 示例10: 多边形偏移的挑战 ====================
    std::cout << "\n=== 示例10: 多边形偏移的挑战 ===" << std::endl;
    
    std::cout << "多边形偏移的挑战:" << std::endl;
    std::cout << "  1. 自相交问题" << std::endl;
    std::cout << "  2. 退化情况处理" << std::endl;
    std::cout << "  3. 边角处理（圆弧拟合）" << std::endl;
    std::cout << "  4. 性能优化" << std::endl;
    
    std::cout << "CGAL 的解决方案:" << std::endl;
    std::cout << "  1. 使用精确算术" << std::endl;
    std::cout << "  2. 自动处理自相交" << std::endl;
    std::cout << "  3. 高效的算法实现" << std::endl;

    // ==================== 示例11: 多边形偏移的算法 ====================
    std::cout << "\n=== 示例11: 多边形偏移的算法 ===" << std::endl;
    
    std::cout << "多边形偏移的算法:" << std::endl;
    std::cout << "  1. 基于排列的算法" << std::endl;
    std::cout << "  2. 基于三角剖分的算法" << std::endl;
    std::cout << "  3. 基于网格的算法" << std::endl;
    
    std::cout << "CGAL 的实现:" << std::endl;
    std::cout << "  1. 使用精确算术" << std::endl;
    std::cout << "  2. 处理退化情况" << std::endl;
    std::cout << "  3. 高效的数据结构" << std::endl;

    // ==================== 示例12: 多边形偏移的实际应用 ====================
    std::cout << "\n=== 示例12: 多边形偏移的实际应用 ===" << std::endl;
    
    std::cout << "实际应用案例:" << std::endl;
    std::cout << "  1. 城市规划：建筑退界分析" << std::endl;
    std::cout << "  2. 交通工程：道路安全距离" << std::endl;
    std::cout << "  3. 环境保护：生态保护区" << std::endl;
    std::cout << "  4. 机器人：避障路径规划" << std::endl;
    std::cout << "  5. 医学：器官的缓冲区分析" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   多边形偏移示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
