/**
 * @file mesh_generation.cpp
 * @brief CGAL 网格生成示例
 * 
 * 网格生成是将连续区域离散化为有限个单元（通常是三角形或四面体）
 * 应用场景：
 * 1. 有限元分析（FEA）
 * 2. 计算机图形学
 * 3. 计算流体力学（CFD）
 * 4. 3D 打印
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Timer.h>

// 定义内核和类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef CGAL::Constrained_Delaunay_triangulation_2<Kernel> CDT;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 约束 Delaunay 三角剖分
 * 2. 网格质量评估
 * 3. 网格细化
 * 4. 网格生成的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 网格生成示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 约束 Delaunay 三角剖分 ====================
    std::cout << "\n=== 示例1: 约束 Delaunay 三角剖分 ===" << std::endl;
    
    // 创建 CDT
    CDT cdt;
    
    // 添加顶点
    cdt.insert(Point_2(0, 0));
    cdt.insert(Point_2(4, 0));
    cdt.insert(Point_2(4, 3));
    cdt.insert(Point_2(0, 3));
    
    // 添加约束边（边界）
    cdt.insert_constraint(Point_2(0, 0), Point_2(4, 0));
    cdt.insert_constraint(Point_2(4, 0), Point_2(4, 3));
    cdt.insert_constraint(Point_2(4, 3), Point_2(0, 3));
    cdt.insert_constraint(Point_2(0, 3), Point_2(0, 0));
    
    std::cout << "创建了一个矩形约束三角剖分" << std::endl;
    std::cout << "顶点数: " << cdt.number_of_vertices() << std::endl;
    std::cout << "三角形数: " << cdt.number_of_faces() << std::endl;
    std::cout << "约束边数: " << cdt.number_of_constraints() << std::endl;

    // ==================== 示例2: 添加内部顶点 ====================
    std::cout << "\n=== 示例2: 添加内部顶点 ===" << std::endl;
    
    // 添加内部顶点
    cdt.insert(Point_2(2, 1.5));  // 中心点
    
    std::cout << "添加中心点 (2, 1.5) 后:" << std::endl;
    std::cout << "  顶点数: " << cdt.number_of_vertices() << std::endl;
    std::cout << "  三角形数: " << cdt.number_of_faces() << std::endl;
    std::cout << "  中心点将矩形分成 4 个三角形" << std::endl;

    // ==================== 示例3: 网格质量评估 ====================
    std::cout << "\n=== 示例3: 网格质量评估 ===" << std::endl;
    
    std::cout << "网格质量指标:" << std::endl;
    std::cout << "  1. 最小角: 越大越好（避免狭长三角形）" << std::endl;
    std::cout << "  2. 最大角: 越小越好" << std::endl;
    std::cout << "  3. 面积比: 三角形面积的均匀程度" << std::endl;
    std::cout << "  4. 形状比: 边长比例的均匀程度" << std::endl;
    
    // 计算最小角
    double min_angle = 90.0;
    for (auto face = cdt.finite_faces_begin(); face != cdt.finite_faces_end(); ++face) {
        Point_2 p1 = face->vertex(0)->point();
        Point_2 p2 = face->vertex(1)->point();
        Point_2 p3 = face->vertex(2)->point();
        
        // 计算边长
        double a = std::sqrt(CGAL::squared_distance(p2, p3));
        double b = std::sqrt(CGAL::squared_distance(p1, p3));
        double c = std::sqrt(CGAL::squared_distance(p1, p2));
        
        // 计算最小角
        double cos_angle = (b*b + c*c - a*a) / (2*b*c);
        double angle = std::acos(cos_angle) * 180.0 / 3.14159265359;
        
        if (angle < min_angle) {
            min_angle = angle;
        }
    }
    
    std::cout << "当前网格的最小角: " << min_angle << " 度" << std::endl;
    std::cout << "  一般要求最小角 > 20 度" << std::endl;

    // ==================== 示例4: 网格细化 ====================
    std::cout << "\n=== 示例4: 网格细化概念 ===" << std::endl;
    
    std::cout << "网格细化（Mesh Refinement）:" << std::endl;
    std::cout << "  1. Bubnov-Galerkin 细化" << std::endl;
    std::cout << "  2. Delaunay 细化" << std::endl;
    std::cout << "  3. 基于误差估计的自适应细化" << std::endl;
    
    std::cout << "细化策略:" << std::endl;
    std::cout << "  1. 设置最小角阈值" << std::endl;
    std::cout << "  2. 设置最大边长限制" << std::endl;
    std::cout << "  3. 在高梯度区域加密网格" << std::endl;

    // ==================== 示例5: 网格生成的应用 - 有限元分析 ====================
    std::cout << "\n=== 示例5: 网格生成的应用 - 有限元分析 ===" << std::endl;
    
    std::cout << "有限元分析的网格生成流程:" << std::endl;
    std::cout << "  1. 几何建模（CAD 模型）" << std::endl;
    std::cout << "  2. 网格划分" << std::endl;
    std::cout << "  3. 单元类型选择（线性/二次）" << std::endl;
    std::cout << "  4. 边界条件施加" << std::endl;
    std::cout << "  5. 求解线性方程组" << std::endl;
    std::cout << "  6. 后处理和结果可视化" << std::endl;
    
    std::cout << "网格质量对 FEA 结果的影响:" << std::endl;
    std::cout << "  1. 狭长三角形会导致数值不稳定" << std::endl;
    std::cout << "  2. 网格密度影响计算精度和时间" << std::endl;
    std::cout << "  3. 边界层网格需要特殊处理" << std::endl;

    // ==================== 示例6: 网格生成的应用 - 3D 打印 ====================
    std::cout << "\n=== 示例6: 网格生成的应用 - 3D 打印 ===" << std::endl;
    
    std::cout << "3D 打印的网格要求:" << std::endl;
    std::cout << "  1. 水密性（Watertight）: 网格必须封闭" << std::endl;
    std::cout << "  2. 法向量一致性: 所有面的法向量方向一致" << std::endl;
    std::cout << "  3. 三角形质量: 避免退化三角形" << std::endl;
    std::cout << "  4. 网格密度: 根据打印精度要求调整" << std::endl;

    // ==================== 示例7: 网格生成的应用 - 计算机图形学 ====================
    std::cout << "\n=== 示例7: 网格生成的应用 - 计算机图形学 ===" << std::endl;
    
    std::cout << "计算机图形学的网格应用:" << std::endl;
    std::cout << "  1. 3D 建模和渲染" << std::endl;
    std::cout << "  2. 粒子系统" << std::endl;
    std::cout << "  3. 碰撞检测" << std::endl;
    std::cout << "  4. 网格变形和动画" << std::endl;
    
    std::cout << "图形学对网格的要求:" << std::endl;
    std::cout << "  1. 实时性能（三角形数量适中）" << std::endl;
    std::cout << "  2. 网格拓扑简单" << std::endl;
    std::cout << "  3. 纹理坐标映射方便" << std::endl;

    // ==================== 示例8: 网格生成的应用 - 地形建模 ====================
    std::cout << "\n=== 示例8: 网格生成的应用 - 地形建模 ===" << std::endl;
    
    std::cout << "地形建模的网格生成:" << std::endl;
    std::cout << "  1. DEM（数字高程模型）网格" << std::endl;
    std::cout << "  2. Delaunay 三角剖分用于不规则地形" << std::endl;
    std::cout << "  3. TIN（不规则三角网）" << std::endl;
    
    std::cout << "地形网格的特点:" << std::endl;
    std::cout << "  1. 顶点包含高程信息（Z 坐标）" << std::endl;
    std::cout << "  2. 网格密度随地形复杂度变化" << std::endl;
    std::cout << "  3. 需要保持地形特征（山脊、山谷）" << std::endl;

    // ==================== 示例9: 网格生成的算法 ====================
    std::cout << "\n=== 示例9: 网格生成的算法 ===" << std::endl;
    
    std::cout << "常见的网格生成算法:" << std::endl;
    std::cout << "  1. Delaunay 三角剖分" << std::endl;
    std::cout << "  2. advancing front 法" << std::endl;
    std::cout << "  3. 泊松重建（Poisson Reconstruction）" << std::endl;
    std::cout << "  4. 八叉树细分" << std::endl;
    std::cout << "  5. 边收缩和顶点分裂" << std::endl;
    
    std::cout << "CGAL 支持的网格生成:" << std::endl;
    std::cout << "  1. 2D 约束 Delaunay 三角剖分" << std::endl;
    std::cout << "  2. 3D Delaunay 三角剖分" << std::endl;
    std::cout << "  3. 表面网格生成" << std::endl;
    std::cout << "  4. 体网格生成" << std::endl;

    // ==================== 示例10: 网格生成的性能优化 ====================
    std::cout << "\n=== 示例10: 网格生成的性能优化 ===" << std::endl;
    
    CGAL::Timer timer;
    timer.start();
    
    // 创建一个较大的网格
    CDT large_cdt;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            large_cdt.insert(Point_2(i, j));
        }
    }
    
    timer.stop();
    
    std::cout << "生成 10x10 网格耗时: " << timer.time() << " 秒" << std::endl;
    std::cout << "  顶点数: " << large_cdt.number_of_vertices() << std::endl;
    std::cout << "  三角形数: " << large_cdt.number_of_faces() << std::endl;
    
    std::cout << "性能优化建议:" << std::endl;
    std::cout << "  1. 使用预分配的内存" << std::endl;
    std::cout << "  2. 批量插入顶点" << std::endl;
    std::cout << "  3. 使用并行算法（如果支持）" << std::endl;
    std::cout << "  4. 选择合适的数据结构" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   网格生成示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
