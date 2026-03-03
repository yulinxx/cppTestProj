/**
 * @file segment_tree.cpp
 * @brief CGAL 线段树示例
 * 
 * 线段树是用于存储线段或区间的数据结构
 * 主要功能：
 * 1. 范围查询
 * 2. 线段相交查询
 * 3. 动态更新
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Segment_tree_2.h>
#include <CGAL/Kd_tree_2.h>

// 定义内核和类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 线段树的创建
 * 2. 线段树的查询
 * 3. 线段树的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 线段树示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建线段集 ====================
    std::cout << "\n=== 示例1: 创建线段集 ===" << std::endl;
    
    // 创建线段集
    std::vector<Segment_2> segments;
    segments.push_back(Segment_2(Point_2(0, 0), Point_2(4, 0)));
    segments.push_back(Segment_2(Point_2(0, 1), Point_2(4, 1)));
    segments.push_back(Segment_2(Point_2(0, 2), Point_2(4, 2)));
    segments.push_back(Segment_2(Point_2(0, 3), Point_2(4, 3)));
    segments.push_back(Segment_2(Point_2(0, 4), Point_2(4, 4)));
    
    std::cout << "创建了 5 条水平线段:" << std::endl;
    for (size_t i = 0; i < segments.size(); i++) {
        std::cout << "  线段 " << i << ": (" 
                  << segments[i].source().x() << ", " 
                  << segments[i].source().y() << ") -> (" 
                  << segments[i].target().x() << ", " 
                  << segments[i].target().y() << ")" << std::endl;
    }

    // ==================== 示例2: 线段树的创建 ====================
    std::cout << "\n=== 示例2: 线段树的创建概念 ===" << std::endl;
    
    std::cout << "线段树（Segment Tree）:" << std::endl;
    std::cout << "  定义: 用于存储线段或区间的平衡二叉树" << std::endl;
    std::cout << "  应用: 范围查询、线段相交查询" << std::endl;
    
    std::cout << "构造过程:" << std::endl;
    std::cout << "  1. 收集所有端点" << std::endl;
    std::cout << "  2. 排序并去重" << std::endl;
    std::cout << "  3. 构建平衡二叉树" << std::endl;
    std::cout << "  4. 插入线段" << std::endl;
    
    std::cout << "时间复杂度:" << std::endl;
    std::cout << "  构造: O(n log n)" << std::endl;
    std::cout << "  查询: O(log n + k)" << std::endl;
    std::cout << "  其中 k 是结果数量" << std::endl;

    // ==================== 示例3: KD-Tree 的创建 ====================
    std::cout << "\n=== 示例3: KD-Tree 的创建概念 ===" << std::endl;
    
    std::cout << "KD-Tree（K-Dimensional Tree）:" << std::endl;
    std::cout << "  定义: 用于组织 k 维空间中点的数据结构" << std::endl;
    std::cout << "  应用: 最近邻搜索、范围查询" << std::endl;
    
    std::cout << "构造过程:" << std::endl;
    std::cout << "  1. 选择维度（交替选择）" << std::endl;
    std::cout << "  2. 选择中位数作为分割点" << std::endl;
    std::cout << "  3. 递归构造左右子树" << std::endl;
    
    std::cout << "时间复杂度:" << std::endl;
    std::cout << "  构造: O(n log n)" << std::endl;
    std::cout << "  查询: O(log n)（平均）" << std::endl;
    std::cout << "  最坏: O(n)" << std::endl;

    // ==================== 示例4: 线段树的应用 - 范围查询 ====================
    std::cout << "\n=== 示例4: 线段树的应用 - 范围查询 ===" << std::endl;
    
    std::cout << "范围查询:" << std::endl;
    std::cout << "  1. 找到与给定区间相交的所有线段" << std::endl;
    std::cout << "  2. 找到在给定范围内的所有点" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到 x 范围 [1, 3] 内的所有线段" << std::endl;
    std::cout << "  2. 找到 y 范围 [1, 3] 内的所有线段" << std::endl;
    std::cout << "  3. 找到矩形范围内的所有线段" << std::endl;

    // ==================== 示例5: 线段树的应用 - 线段相交查询 ====================
    std::cout << "\n=== 示例5: 线段树的应用 - 线段相交查询 ===" << std::endl;
    
    std::cout << "线段相交查询:" << std::endl;
    std::cout << "  1. 找到与给定线段相交的所有线段" << std::endl;
    std::cout << "  2. 找到与给定点相交的所有线段" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到与垂直线段 x=2 相交的所有线段" << std::endl;
    std::cout << "  2. 找到与点(2, 2) 相交的所有线段" << std::endl;
    std::cout << "  3. 找到与射线相交的所有线段" << std::endl;

    // ==================== 示例6: KD-Tree 的应用 - 最近邻搜索 ====================
    std::cout << "\n=== 示例6: KD-Tree 的应用 - 最近邻搜索 ===" << std::endl;
    
    std::cout << "KD-Tree 的最近邻搜索:" << std::endl;
    std::cout << "  1. 找到最近的点" << std::endl;
    std::cout << "  2. 找到 K 个最近的点" << std::endl;
    std::cout << "  3. 范围搜索" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到最近的点" << std::endl;
    std::cout << "  2. 找到 5 个最近的点" << std::endl;
    std::cout << "  3. 找到半径 2 内的所有点" << std::endl;

    // ==================== 示例7: KD-Tree 的应用 - 范围查询 ====================
    std::cout << "\n=== 示例7: KD-Tree 的应用 - 范围查询 ===" << std::endl;
    
    std::cout << "KD-Tree 的范围查询:" << std::endl;
    std::cout << "  1. 找到在给定范围内的所有点" << std::endl;
    std::cout << "  2. 矩形范围查询" << std::endl;
    std::cout << "  3. 圆形范围查询" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到 x 范围 [1, 3]，y 范围 [1, 3] 内的所有点" << std::endl;
    std::cout << "  2. 找到圆心(2, 2)，半径 2 内的所有点" << std::endl;

    // ==================== 示例8: KD-Tree 的应用 - GIS ====================
    std::cout << "\n=== 示例8: KD-Tree 的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS 中的 KD-Tree:" << std::endl;
    std::cout << "  1. 空间索引" << std::endl;
    std::cout << "  2. 最近邻搜索" << std::endl;
    std::cout << "  3. 范围查询" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到最近的 POI" << std::endl;
    std::cout << "  2. 找到半径 1 公里内的所有餐厅" << std::endl;
    std::cout << "  3. 找到某区域内的所有建筑" << std::endl;

    // ==================== 示例9: KD-Tree 的应用 - 计算机图形学 ====================
    std::cout << "\n=== 示例9: KD-Tree 的应用 - 计算机图形学 ===" << std::endl;
    
    std::cout << "计算机图形学中的 KD-Tree:" << std::endl;
    std::cout << "  1. 光线追踪（BVH）" << std::endl;
    std::cout << "  2. 碰撞检测" << std::endl;
    std::cout << "  3. 粒子系统" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 光线与物体的相交测试" << std::endl;
    std::cout << "  2. 碰撞检测" << std::endl;
    std::cout << "  3. 粒子之间的相互作用" << std::endl;

    // ==================== 示例10: KD-Tree 的应用 - 机器学习 ====================
    std::cout << "\n=== 示例10: KD-Tree 的应用 - 机器学习 ===" << std::endl;
    
    std::cout << "机器学习中的 KD-Tree:" << std::endl;
    std::cout << "  1. KNN 算法" << std::endl;
    std::cout << "  2. 密度聚类（DBSCAN）" << std::endl;
    std::cout << "  3. 核密度估计" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. KNN 分类" << std::endl;
    std::cout << "  2. KNN 回归" << std::endl;
    std::cout << "  3. 寻找高密度区域" << std::endl;

    // ==================== 示例11: KD-Tree 的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例11: KD-Tree 的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的 KD-Tree:" << std::endl;
    std::cout << "  1. 特征匹配（FLANN）" << std::endl;
    std::cout << "  2. 图像检索" << std::endl;
    std::cout << "  3. 3D 重建" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. SIFT 特征匹配" << std::endl;
    std::cout << "  2. SURF 特征匹配" << std::endl;
    std::cout << "  3. ORB 特征匹配" << std::endl;

    // ==================== 示例12: KD-Tree 的挑战 ====================
    std::cout << "\n=== 示例12: KD-Tree 的挑战 ===" << std::endl;
    
    std::cout << "KD-Tree 的挑战:" << std::endl;
    std::cout << "  1. 维度灾难（高维空间性能下降）" << std::endl;
    std::cout << "  2. 数据分布不均匀" << std::endl;
    std::cout << "  3. 动态更新" << std::endl;
    std::cout << "  4. 大规模数据处理" << std::endl;
    
    std::cout << "解决方案:" << std::endl;
    std::cout << "  1. 使用近似算法（ANN）" << std::endl;
    std::cout << "  2. 使用多尺度方法" << std::endl;
    std::cout << "  3. 使用并行计算" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   线段树示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
