/**
 * @file nearest_neighbors.cpp
 * @brief CGAL 最近邻搜索示例
 * 
 * 最近邻搜索是找到最近点的技术
 * 主要功能：
 * 1. 最近邻搜索
 * 2. K 近邻搜索
 * 3. 范围搜索
 * 4. 最近邻图
 */

#include <iostream>
#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/algorithm.h>

// 定义内核和点类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 最近邻搜索
 * 2. K 近邻搜索
 * 3. 最近邻的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 最近邻搜索示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建点集 ====================
    std::cout << "\n=== 示例1: 创建点集 ===" << std::endl;
    
    // 生成随机点
    std::vector<Point_2> points;
    CGAL::Random_points_in_square_2<Point_2> gen(10.0);
    
    for (int i = 0; i < 20; i++) {
        points.push_back(*gen++);
    }
    
    std::cout << "生成了 20 个随机点（在 -10 到 10 的正方形内）" << std::endl;
    std::cout << "前 5 个点:" << std::endl;
    for (size_t i = 0; i < 5; i++) {
        std::cout << "  点 " << i << ": (" 
                  << points[i].x() << ", " 
                  << points[i].y() << ")" << std::endl;
    }

    // ==================== 示例2: 最近邻搜索 ====================
    std::cout << "\n=== 示例2: 最近邻搜索 ===" << std::endl;
    
    Point_2 query(0, 0);
    std::cout << "查询点: (" << query.x() << ", " << query.y() << ")" << std::endl;
    
    // 使用 CGAL 的最近邻算法
    auto nearest = CGAL::nearest_neighbor(points.begin(), points.end(), query);
    
    std::cout << "最近邻点: (" << nearest->x() << ", " << nearest->y() << ")" << std::endl;
    std::cout << "距离: " << std::sqrt(CGAL::squared_distance(query, *nearest)) << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 最近设施搜索" << std::endl;
    std::cout << "  2. 模式识别" << std::endl;
    std::cout << "  3. 机器学习（KNN 算法）" << std::endl;

    // ==================== 示例3: K 近邻搜索 ====================
    std::cout << "\n=== 示例3: K 近邻搜索概念 ===" << std::endl;
    
    std::cout << "K 近邻搜索（K-Nearest Neighbors）:" << std::endl;
    std::cout << "  定义: 找到 K 个最近的点" << std::endl;
    std::cout << "  应用: 分类、回归、聚类" << std::endl;
    
    std::cout << "示例（K=3）:" << std::endl;
    std::cout << "  1. 找到 3 个最近的点" << std::endl;
    std::cout << "  2. 用于分类（投票）" << std::endl;
    std::cout << "  3. 用于回归（平均）" << std::endl;
    
    std::cout << "算法:" << std::endl;
    std::cout << "  1. 暴力搜索（O(n)）" << std::endl;
    std::cout << "  2. KD-Tree（O(log n)）" << std::endl;
    std::cout << "  3. Ball Tree" << std::endl;

    // ==================== 示例4: 最近邻的应用 - 最近设施搜索 ====================
    std::cout << "\n=== 示例4: 最近邻的应用 - 最近设施搜索 ===" << std::endl;
    
    std::cout << "最近设施搜索:" << std::endl;
    std::cout << "  1. 找到最近的加油站" << std::endl;
    std::cout << "  2. 找到最近的医院" << std::endl;
    std::cout << "  3. 找到最近的餐厅" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到最近的充电站" << std::endl;
    std::cout << "  2. 找到最近的维修点" << std::endl;
    std::cout << "  3. 找到最近的服务中心" << std::endl;

    // ==================== 示例5: 最近邻的应用 - 模式识别 ====================
    std::cout << "\n=== 示例5: 最近邻的应用 - 模式识别 ===" << std::endl;
    
    std::cout << "模式识别中的最近邻:" << std::endl;
    std::cout << "  1. KNN 分类" << std::endl;
    std::cout << "  2. KNN 回归" << std::endl;
    std::cout << "  3. 模板匹配" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 字符识别" << std::endl;
    std::cout << "  2. 图像分类" << std::endl;
    std::cout << "  3. 语音识别" << std::endl;

    // ==================== 示例6: 最近邻的应用 - 机器学习 ====================
    std::cout << "\n=== 示例6: 最近邻的应用 - 机器学习 ===" << std::endl;
    
    std::cout << "机器学习中的最近邻:" << std::endl;
    std::cout << "  1. KNN 分类算法" << std::endl;
    std::cout << "  2. KNN 回归算法" << std::endl;
    std::cout << "  3. 离群点检测" << std::endl;
    
    std::cout << "KNN 分类原理:" << std::endl;
    std::cout << "  1. 找到 K 个最近的训练样本" << std::endl;
    std::cout << "  2. 投票决定类别" << std::endl;
    std::cout << "  3. 简单高效" << std::endl;

    // ==================== 示例7: 最近邻的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例7: 最近邻的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的最近邻:" << std::endl;
    std::cout << "  1. 特征匹配（FLANN）" << std::endl;
    std::cout << "  2. 图像检索" << std::endl;
    std::cout << "  3. 3D 重建" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. SIFT 特征匹配" << std::endl;
    std::cout << "  2. SURF 特征匹配" << std::endl;
    std::cout << "  3. ORB 特征匹配" << std::endl;

    // ==================== 示例8: 最近邻的应用 - 机器人 ====================
    std::cout << "\n=== 示例8: 最近邻的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的最近邻:" << std::endl;
    std::cout << "  1. 最近路标搜索" << std::endl;
    std::cout << "  2. 最近障碍物搜索" << std::endl;
    std::cout << "  3. 最近目标搜索" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到最近的路标" << std::endl;
    std::cout << "  2. 找到最近的障碍物" << std::endl;
    std::cout << "  3. 找到最近的目标" << std::endl;

    // ==================== 示例9: 最近邻的应用 - GIS ====================
    std::cout << "\n=== 示例9: 最近邻的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS 中的最近邻:" << std::endl;
    std::cout << "  1. 最近 POI 搜索" << std::endl;
    std::cout << "  2. 最近路径搜索" << std::endl;
    std::cout << "  3. 最近设施搜索" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到最近的餐厅" << std::endl;
    std::cout << "  2. 找到最近的加油站" << std::endl;
    std::cout << "  3. 找到最近的医院" << std::endl;

    // ==================== 示例10: 最近邻的应用 - 数据库 ====================
    std::cout << "\n=== 示例10: 最近邻的应用 - 数据库 ===" << std::endl;
    
    std::cout << "数据库中的最近邻:" << std::endl;
    std::cout << "  1. 空间索引" << std::endl;
    std::cout << "  2. KNN 查询" << std::endl;
    std::cout << "  3. 相似性搜索" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. PostgreSQL/PostGIS 的 KNN 查询" << std::endl;
    std::cout << "  2. MongoDB 的地理空间查询" << std::endl;
    std::cout << "  3. Elasticsearch 的相似性搜索" << std::endl;

    // ==================== 示例11: 最近邻的应用 - 图像处理 ====================
    std::cout << "\n=== 示例11: 最近邻的应用 - 图像处理 ===" << std::endl;
    
    std::cout << "图像处理中的最近邻:" << std::endl;
    std::cout << "  1. 图像插值" << std::endl;
    std::cout << "  2. 图像去噪" << std::endl;
    std::cout << "  3. 图像分割" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 最近邻插值" << std::endl;
    std::cout << "  2. 最近邻去噪" << std::endl;
    std::cout << "  3. 最近邻分割" << std::endl;

    // ==================== 示例12: 最近邻的挑战 ====================
    std::cout << "\n=== 示例12: 最近邻的挑战 ===" << std::endl;
    
    std::cout << "最近邻的挑战:" << std::endl;
    std::cout << "  1. 维度灾难（高维空间性能下降）" << std::endl;
    std::cout << "  2. 数据分布不均匀" << std::endl;
    std::cout << "  3. 动态更新" << std::endl;
    std::cout << "  4. 大规模数据处理" << std::endl;
    
    std::cout << "解决方案:" << std::endl;
    std::cout << "  1. 使用近似算法（ANN）" << std::endl;
    std::cout << "  2. 使用多尺度方法" << std::endl;
    std::cout << "  3. 使用并行计算" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   最近邻搜索示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
