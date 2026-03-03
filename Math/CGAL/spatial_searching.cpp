/**
 * @file spatial_searching.cpp
 * @brief CGAL 空间搜索示例
 *
 * 什么是 CGAL 空间搜索？
 * ====================
 * 空间搜索（Spatial Searching）是在几何数据结构中高效查询几何对象的
 * 技术。这是计算几何和空间数据库中的核心问题。
 *
 * 空间搜索的数学基础：
 * - 空间索引：加速空间查询的数据结构
 * - 距离度量：欧几里得距离、曼哈顿距离等
 * - 最近邻：找到距离查询点最近的点
 * - 范围查询：找到指定区域内的所有对象
 *
 * CGAL 提供的空间搜索结构：
 * 1. KD-Tree：K维二叉搜索树
 * 2. Range Tree：多维范围查询树
 * 3. Interval Tree：区间查询树
 * 4. R-Tree：矩形对象的层次索引
 * 5. Segment Tree：线段索引树
 * 6. Bounding Volume Hierarchy (BVH)：包围体层次
 *
 * 空间搜索的时间复杂度：
 * 1. KD-Tree：
 *    - 构造：O(n log n)
 *    - 查询：O(log n)（平均），O(n)（最坏）
 * 2. Range Tree：
 *    - 构造：O(n log^(d-1) n)
 *    - 查询：O(log^d n + k)
 * 3. R-Tree：
 *    - 构造：O(n log n)
 *    - 查询：O(log n + k)
 *
 * 空间搜索的应用场景：
 * 1. 地理信息系统（GIS）：空间查询、POI搜索
 * 2. 计算机图形学：碰撞检测、光线追踪
 * 3. 机器人学：最近邻搜索、路径规划
 * 4. 机器学习：KNN算法、聚类
 * 5. 计算机视觉：特征匹配、图像检索
 * 6. 数据库：空间索引、地理查询
 * 7. 游戏开发：碰撞检测、射线查询
 * 8. 推荐系统：相似性搜索
 *
 * 空间搜索的挑战：
 * 1. 维度灾难：高维空间性能下降
 * 2. 动态更新：高效插入和删除
 * 3. 近似算法：平衡精度和速度
 * 4. 大规模数据：分布式处理
 *
 * 本示例演示：
 * 1. 创建点集
 * 2. 最近邻搜索
 * 3. 范围查询
 * 4. KD-Tree搜索概念
 * 5. 范围树概念
 * 6. GIS应用
 * 7. 计算机图形学应用
 * 8. 机器人应用
 * 9. 机器学习应用
 * 10. 计算机视觉应用
 * 11. 性能比较
 * 12. 数据库应用
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
 * 2. 范围查询
 * 3. 空间搜索的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 空间搜索示例" << std::endl;
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
    std::cout << "点集:" << std::endl;
    for (size_t i = 0; i < points.size(); i++) {
        std::cout << "  点 " << i << ": (" << points[i].x() << ", " << points[i].y() << ")" << std::endl;
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

    // ==================== 示例3: 范围查询 ====================
    std::cout << "\n=== 示例3: 范围查询 ===" << std::endl;
    
    Point_2 center(0, 0);
    double radius = 5.0;
    
    std::cout << "范围查询: 圆心(" << center.x() << ", " << center.y() << "), 半径 " << radius << std::endl;
    
    // 查找在圆内的点
    std::vector<Point_2> result;
    for (const auto& p : points) {
        if (CGAL::squared_distance(center, p) <= radius * radius) {
            result.push_back(p);
        }
    }
    
    std::cout << "在圆内的点 (" << result.size() << " 个):" << std::endl;
    for (const auto& p : result) {
        std::cout << "  (" << p.x() << ", " << p.y() << ")" << std::endl;
    }
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 地理围栏（Geofencing）" << std::endl;
    std::cout << "  2. 影响范围分析" << std::endl;
    std::cout << "  3. 粒子系统" << std::endl;

    // ==================== 示例4: KD-Tree 搜索 ====================
    std::cout << "\n=== 示例4: KD-Tree 搜索概念 ===" << std::endl;
    
    std::cout << "KD-Tree（K-Dimensional Tree）:" << std::endl;
    std::cout << "  1. 二叉搜索树，每个节点代表一个超矩形" << std::endl;
    std::cout << "  2. 在每个维度上交替分割空间" << std::endl;
    std::cout << "  3. 支持高效的最近邻和范围查询" << std::endl;
    
    std::cout << "时间复杂度:" << std::endl;
    std::cout << "  构造: O(n log n)" << std::endl;
    std::cout << "  查询: O(log n)（平均情况）" << std::endl;
    std::cout << "  最坏情况: O(n)" << std::endl;
    
    std::cout << "空间复杂度: O(n)" << std::endl;

    // ==================== 示例5: 范围树 ====================
    std::cout << "\n=== 示例5: 范围树概念 ===" << std::endl;
    
    std::cout << "范围树（Range Tree）:" << std::endl;
    std::cout << "  1. 多维搜索树" << std::endl;
    std::cout << "  2. 用于范围查询（找到在给定范围内的所有点）" << std::endl;
    std::cout << "  3. 支持 d 维空间的范围查询" << std::endl;
    
    std::cout << "时间复杂度:" << std::endl;
    std::cout << "  构造: O(n log^(d-1) n)" << std::endl;
    std::cout << "  查询: O(log^d n + k)" << std::endl;
    std::cout << "  其中 k 是结果数量" << std::endl;

    // ==================== 示例6: 空间搜索的应用 - GIS ====================
    std::cout << "\n=== 示例6: 空间搜索的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS（地理信息系统）中的空间搜索:" << std::endl;
    std::cout << "  1. 找到某区域内的所有 POI（兴趣点）" << std::endl;
    std::cout << "  2. 找到某路径附近的设施" << std::endl;
    std::cout << "  3. 空间连接（Spatial Join）" << std::endl;
    
    std::cout << "示例查询:" << std::endl;
    std::cout << "  1. 找到半径 1 公里内的所有餐厅" << std::endl;
    std::cout << "  2. 找到与某条河流相交的所有土地" << std::endl;
    std::cout << "  3. 找到某行政区内的所有建筑" << std::endl;

    // ==================== 示例7: 空间搜索的应用 - 计算机图形学 ====================
    std::cout << "\n=== 示例7: 空间搜索的应用 - 计算机图形学 ===" << std::endl;
    
    std::cout << "计算机图形学中的空间搜索:" << std::endl;
    std::cout << "  1. 碰撞检测" << std::endl;
    std::cout << "  2. 光线追踪（BVH - Bounding Volume Hierarchy）" << std::endl;
    std::cout << "  3. 粒子系统" << std::endl;
    std::cout << "  4. 视锥剔除（Frustum Culling）" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 检测子弹是否击中目标" << std::endl;
    std::cout << "  2. 找到可见的物体（视锥内）" << std::endl;
    std::cout << "  3. 找到附近的粒子进行交互" << std::endl;

    // ==================== 示例8: 空间搜索的应用 - 机器人 ====================
    std::cout << "\n=== 示例8: 空间搜索的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的空间搜索:" << std::endl;
    std::cout << "  1. 最近邻搜索（找到最近的路标）" << std::endl;
    std::cout << "  2. 路径规划（A* 算法的搜索）" << std::endl;
    std::cout << "  3. SLAM（同步定位与建图）" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到最近的充电站" << std::endl;
    std::cout << "  2. 找到路径上的障碍物" << std::endl;
    std::cout << "  3. 找到匹配的特征点" << std::endl;

    // ==================== 示例9: 空间搜索的应用 - 机器学习 ====================
    std::cout << "\n=== 示例9: 空间搜索的应用 - 机器学习 ===" << std::endl;
    
    std::cout << "机器学习中的空间搜索:" << std::endl;
    std::cout << "  1. K 近邻算法（K-Nearest Neighbors）" << std::endl;
    std::cout << "  2. 密度聚类（DBSCAN）" << std::endl;
    std::cout << "  3. 核密度估计" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 分类：找到 K 个最近的训练样本" << std::endl;
    std::cout << "  2. 聚类：找到密度高的区域" << std::endl;
    std::cout << "  3. 异常检测：找到稀疏区域的点" << std::endl;

    // ==================== 示例10: 空间搜索的应用 - 计算机视觉 ====================
    std::cout << "\n=== 示例10: 空间搜索的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的空间搜索:" << std::endl;
    std::cout << "  1. 特征匹配（FLANN - Fast Library for Approximate Nearest Neighbors）" << std::endl;
    std::cout << "  2. 光流跟踪" << std::endl;
    std::cout << "  3. 图像检索" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 找到匹配的特征点（SIFT, SURF）" << std::endl;
    std::cout << "  2. 找到相似的图像" << std::endl;
    std::cout << "  3. 找到运动轨迹" << std::endl;

    // ==================== 示例11: 空间搜索的性能比较 ====================
    std::cout << "\n=== 示例11: 空间搜索的性能比较 ===" << std::endl;
    
    std::cout << "不同空间搜索结构的性能比较:" << std::endl;
    std::cout << "  KD-Tree:" << std::endl;
    std::cout << "    优点: 实现简单，性能好" << std::endl;
    std::cout << "    缺点: 高维性能下降（维度灾难）" << std::endl;
    std::cout << "  Range Tree:" << std::endl;
    std::cout << "    优点: 理论性能好，支持范围查询" << std::endl;
    std::cout << "    缺点: 实现复杂，内存占用大" << std::endl;
    std::cout << "  R-Tree:" << std::endl;
    std::cout << "    优点: 适合矩形对象，动态更新" << std::endl;
    std::cout << "    缺点: 查询性能不稳定" << std::endl;

    // ==================== 示例12: 空间搜索的应用 - 数据库 ====================
    std::cout << "\n=== 示例12: 空间搜索的应用 - 数据库 ===" << std::endl;
    
    std::cout << "数据库中的空间索引:" << std::endl;
    std::cout << "  1. PostgreSQL/PostGIS 使用 R-Tree" << std::endl;
    std::cout << "  2. MongoDB 使用 GeoHash" << std::endl;
    std::cout << "  3. Elasticsearch 使用 BKD-Tree" << std::endl;
    
    std::cout << "空间查询示例:" << std::endl;
    std::cout << "  1. 找到某区域内的所有记录" << std::endl;
    std::cout << "  2. 找到某路径附近的记录" << std::endl;
    std::cout << "  3. 找到某半径内的记录" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   空间搜索示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
