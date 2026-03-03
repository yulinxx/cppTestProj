/**
 * @file alpha_shapes.cpp
 * @brief CGAL Alpha 形状示例
 * 
 * Alpha 形状是点云的广义凸包，通过参数 alpha 控制形状的细节程度
 * 应用场景：
 * 1. 点云重建
 * 2. 形状分析
 * 3. 分子表面建模
 * 4. 地形分析
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Alpha_shape_2.h>

// 定义内核和类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef CGAL::Delaunay_triangulation_2<Kernel> Triangulation;
typedef CGAL::Alpha_shape_2<Triangulation> Alpha_shape_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. Alpha 形状的创建
 * 2. Alpha 参数的影响
 * 3. Alpha 形状的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL Alpha 形状示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建点云 ====================
    std::cout << "\n=== 示例1: 创建点云 ===" << std::endl;
    
    // 创建点集（圆形分布）
    std::vector<Point_2> points;
    for (int i = 0; i < 20; i++) {
        double angle = i * 2.0 * 3.14159265359 / 20.0;
        double radius = 5.0;
        points.push_back(Point_2(radius * cos(angle), radius * sin(angle)));
    }
    
    // 添加中心点
    points.push_back(Point_2(0, 0));
    
    std::cout << "创建了 21 个点（圆形分布 + 中心点）" << std::endl;
    std::cout << "  圆形半径: 5" << std::endl;
    std::cout << "  中心点: (0, 0)" << std::endl;

    // ==================== 示例2: 创建 Alpha 形状 ====================
    std::cout << "\n=== 示例2: 创建 Alpha 形状 ===" << std::endl;
    
    // 创建 Delaunay 三角剖分
    Triangulation t;
    t.insert(points.begin(), points.end());
    
    // 创建 Alpha 形状
    Alpha_shape_2 alpha(t);
    
    std::cout << "Alpha 形状创建完成" << std::endl;
    std::cout << "  顶点数: " << alpha.number_of_vertices() << std::endl;
    std::cout << "  Alpha 形状的边数: " << alpha.number_of_alpha_edges() << std::endl;
    
    std::cout << "\nAlpha 形状的边类型:" << std::endl;
    std::cout << "  REGULAR: 属于 Alpha 形状的边" << std::endl;
    std::cout <<  "  SINGULAR: 只属于一个三角形的边" << std::endl;

    // ==================== 示例3: Alpha 参数的影响 ====================
    std::cout << "\n=== 示例3: Alpha 参数的影响 ===" << std::endl;
    
    std::cout << "Alpha 参数的影响:" << std::endl;
    std::cout << "  alpha = 0: 凸包" << std::endl;
    std::cout << "  alpha 很小: 紧贴点云" << std::endl;
    std::cout << "  alpha 很大: 接近凸包" << std::endl;
    
    std::cout << "\n示例:" << std::endl;
    std::cout << "  alpha = 0: 凸包（圆形）" << std::endl;
    std::cout << "  alpha = 1: 紧贴点云，包含中心点" << std::endl;
    std::cout << "  alpha = 10: 接近凸包" << std::endl;

    // ==================== 示例4: Alpha 形状的应用 - 点云重建 ====================
    std::cout << "\n=== 示例4: Alpha 形状的应用 - 点云重建 ===" << std::endl;
    
    std::cout << "点云重建中的 Alpha 形状:" << std::endl;
    std::cout << "  1. 从点云重建表面" << std::endl;
    std::cout << "  2. 保持点云的拓扑结构" << std::endl;
    std::cout << "  3. 处理噪声和缺失数据" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 简单高效" << std::endl;
    std::cout << "  2. 保持点云的拓扑结构" << std::endl;
    std::cout << "  3. 可以处理非均匀采样" << std::endl;

    // ==================== 示例5: Alpha 形状的应用 - 形状分析 ====================
    std::cout << "\n=== 示例5: Alpha 形状的应用 - 形状分析 ===" << std::endl;
    
    std::cout << "形状分析中的 Alpha 形状:" << std::endl;
    std::cout << "  1. 形状的 Alpha 形状表示" << std::endl;
    std::cout << "  2. 形状的简化" << std::endl;
    std::cout << "  3. 形状的比较" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 字符识别" << std::endl;
    std::cout << "  2. 图像检索" << std::endl;
    std::cout << "  3. 生物形态分析" << std::endl;

    // ==================== 示例6: Alpha 形状的应用 - 分子表面建模 ====================
    std::cout << "\n=== 示例6: Alpha 形状的应用 - 分子表面建模 ===" << std::endl;
    
    std::cout << "分子表面建模中的 Alpha 形状:" << std::endl;
    std::cout << "  1. 原子位置作为点" << std::endl;
    std::cout << "  2. Alpha 形状作为分子表面" << std::endl;
    std::cout << "  3. 计算分子的溶剂可及表面" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 保持分子的拓扑结构" << std::endl;
    std::cout << "  2. 计算效率高" << std::endl;
    std::cout << "  3. 可以处理大量原子" << std::endl;

    // ==================== 示例7: Alpha 形状的应用 - 地形分析 ====================
    std::cout << "\n=== 示例7: Alpha 形状的应用 - 地形分析 ===" << std::endl;
    
    std::cout << "地形分析中的 Alpha 形状:" << std::endl;
    std::cout << "  1. 从点云重建地形表面" << std::endl;
    std::cout << "  2. 保持地形特征（山脊、山谷）" << std::endl;
    std::cout << "  3. 处理缺失数据" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 地形分析" << std::endl;
    std::cout << "  2. 洪水模拟" << std::endl;
    std::cout << "  3. 视域分析" << std::endl;

    // ==================== 示例8: Alpha 形状的应用 - 图像分割 ====================
    std::cout << "\n=== 示例8: Alpha 形状的应用 - 图像分割 ===" << std::endl;
    
    std::cout << "图像分割中的 Alpha 形状:" << std::endl;
    std::cout << "  1. 像素作为点" << std::endl;
    std::cout << "  2. Alpha 形状作为分割边界" << std::endl;
    std::cout << "  3. 保持图像的拓扑结构" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 保持图像的拓扑结构" << std::endl;
    std::cout << "  2. 简单高效" << std::endl;
    std::cout << "  3. 可以处理复杂边界" << std::endl;

    // ==================== 示例9: Alpha 形状的应用 - 聚类分析 ====================
    std::cout << "\n=== 示例9: Alpha 形状的应用 - 聚类分析 ===" << std::endl;
    
    std::cout << "聚类分析中的 Alpha 形状:" << std::endl;
    std::cout << "  1. 每个聚类的 Alpha 形状" << std::endl;
    std::cout << "  2. 聚类的边界" << std::endl;
    std::cout << "  3. 聚类的形状分析" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 保持聚类的拓扑结构" << std::endl;
    std::cout << "  2. 简单高效" << std::endl;
    std::cout << "  3. 可以处理任意形状的聚类" << std::endl;

    // ==================== 示例10: Alpha 形状的应用 - 路径规划 ====================
    std::cout << "\n=== 示例10: Alpha 形状的应用 - 路径规划 ===" << std::endl;
    
    std::cout << "路径规划中的 Alpha 形状:" << std::endl;
    std::cout << "  1. 障碍物的 Alpha 形状" << std::endl;
    std::cout << "  2. 自由空间的表示" << std::endl;
    std::cout << "  3. 路径搜索" << std::endl;
    
    std::cout << "优势:" << std::endl;
    std::cout << "  1. 保持障碍物的拓扑结构" << std::endl;
    std::cout << "  2. 简单高效" << std::endl;
    std::cout << "  3. 可以处理复杂障碍物" << std::endl;

    // ==================== 示例11: Alpha 形状的算法 ====================
    std::cout << "\n=== 示例11: Alpha 形状的算法 ===" << std::endl;
    
    std::cout << "Alpha 形状的算法:" << std::endl;
    std::cout << "  1. 基于 Delaunay 三角剖分" << std::endl;
    std::cout << "  2. 计算每个边的 Alpha 值" << std::endl;
    std::cout << "  3. 选择 Alpha 值小于阈值的边" << std::endl;
    
    std::cout << "CGAL 的实现:" << std::endl;
    std::cout << "  1. 使用精确算术" << std::endl;
    std::cout << "  2. 处理退化情况" << std::endl;
    std::cout << "  3. 高效的数据结构" << std::endl;

    // ==================== 示例12: Alpha 形状的性质 ====================
    std::cout << "\n=== 示例12: Alpha 形状的性质 ===" << std::endl;
    
    std::cout << "Alpha 形状的主要性质:" << std::endl;
    std::cout << "  1. 连续性: Alpha 值连续变化时，Alpha 形状连续变化" << std::endl;
    std::cout << "  2. 单调性: Alpha 增大时，Alpha 形状增大" << std::endl;
    std::cout << "  3. 极限情况: alpha=0 时是凸包" << std::endl;
    
    std::cout << "\nAlpha 形状的计算复杂度:" << std::endl;
    std::cout << "  2D: O(n log n)" << std::endl;
    std::cout << "  3D: O(n^2)" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   Alpha 形状示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
