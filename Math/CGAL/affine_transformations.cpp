/**
 * @file affine_transformations.cpp
 * @brief CGAL 仿射变换示例
 * 
 * 仿射变换是保持共线性和比例的几何变换
 * 主要变换：
 * 1. 平移（Translation）
 * 2. 旋转（Rotation）
 * 3. 缩放（Scaling）
 * 4. 剪切（Shear）
 * 5. 组合变换
 */

#include <iostream>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/aff_transformation_2.h>

// 定义内核和类型
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Vector_2 Vector_2;
typedef Kernel::Line_2 Line_2;
typedef Kernel::Circle_2 Circle_2;

/**
 * @brief 主函数
 * 
 * 演示内容：
 * 1. 基础仿射变换
 * 2. 仿射变换的应用
 */
int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "   CGAL 仿射变换示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // ==================== 示例1: 创建点 ====================
    std::cout << "\n=== 示例1: 创建点 ===" << std::endl;
    
    Point_2 p(1, 1);
    std::cout << "原始点: (" << p.x() << ", " << p.y() << ")" << std::endl;

    // ==================== 示例2: 平移变换 ====================
    std::cout << "\n=== 示例2: 平移变换 ===" << std::endl;
    
    // 平移向量
    Vector_2 translation(2, 3);
    
    // 应用平移
    Point_2 p_translated = p + translation;
    
    std::cout << "平移向量: (" << translation.x() << ", " << translation.y() << ")" << std::endl;
    std::cout << "平移后点: (" << p_translated.x() << ", " << p_translated.y() << ")" << std::endl;
    std::cout << "  计算: (1+2, 1+3) = (3, 4)" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 物体移动" << std::endl;
    std::cout << "  2. 坐标系变换" << std::endl;
    std::cout << "  3. 动画" << std::endl;

    // ==================== 示例3: 旋转变换 ====================
    std::cout << "\n=== 示例3: 旋转变换 ===" << std::endl;
    
    // 旋转角度（弧度）
    double angle = 3.14159265359 / 4.0;  // 45 度
    
    // 旋转矩阵
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    
    // 应用旋转
    double x_new = p.x() * cos_a - p.y() * sin_a;
    double y_new = p.x() * sin_a + p.y() * cos_a;
    
    std::cout << "旋转角度: " << (angle * 180.0 / 3.14159265359) << " 度" << std::endl;
    std::cout << "旋转后点: (" << x_new << ", " << y_new << ")" << std::endl;
    std::cout << "  计算: (1*cos(45) - 1*sin(45), 1*sin(45) + 1*cos(45))" << std::endl;
    std::cout << "  约等于: (0, 1.414)" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 物体旋转" << std::endl;
    std::cout << "  2. 坐标系旋转" << std::endl;
    std::cout << "  3. 动画" << std::endl;

    // ==================== 示例4: 缩放变换 ====================
    std::cout << "\n=== 示例4: 缩放变换 ===" << std::endl;
    
    // 缩放因子
    double scale_x = 2.0;
    double scale_y = 0.5;
    
    // 应用缩放
    Point_2 p_scaled(p.x() * scale_x, p.y() * scale_y);
    
    std::cout << "缩放因子: (" << scale_x << ", " << scale_y << ")" << std::endl;
    std::cout << "缩放后点: (" << p_scaled.x() << ", " << p_scaled.y() << ")" << std::endl;
    std::cout << "  计算: (1*2, 1*0.5) = (2, 0.5)" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 物体缩放" << std::endl;
    std::cout << "  2. 图像缩放" << std::endl;
    std::cout << "  3. 坐标系缩放" << std::endl;

    // ==================== 示例5: 剪切变换 ====================
    std::cout << "\n=== 示例5: 剪切变换 ===" << std::endl;
    
    // 剪切因子
    double shear_x = 0.5;
    double shear_y = 0.0;
    
    // 应用剪切
    Point_2 p_sheared(p.x() + shear_x * p.y(), p.y() + shear_y * p.x());
    
    std::cout << "剪切因子: (" << shear_x << ", " << shear_y << ")" << std::endl;
    std::cout << "剪切后点: (" << p_sheared.x() << ", " << p_sheared.y() << ")" << std::endl;
    std::cout << "  计算: (1 + 0.5*1, 1 + 0*1) = (1.5, 1)" << std::endl;
    
    std::cout << "应用:" << std::endl;
    std::cout << "  1. 物体剪切" << std::endl;
    std::cout << "  2. 字体设计" << std::endl;
    std::cout << "  3. 特效" << std::endl;

    // ==================== 示例6: 组合变换 ====================
    std::cout << "\n=== 示例6: 组合变换 ===" << std::endl;
    
    std::cout << "组合变换:" << std::endl;
    std::cout << "  1. 先平移再旋转" << std::endl;
    std::cout << "  2. 先旋转再缩放" << std::endl;
    std::cout << "  3. 复杂变换" << std::endl;
    
    std::cout << "示例: 先平移(2,3)再旋转45度" << std::endl;
    std::cout << "  1. 平移: (1,1) -> (3,4)" << std::endl;
    std::cout << "  2. 旋转: (3,4) -> (3*cos(45)-4*sin(45), 3*sin(45)+4*cos(45))" << std::endl;
    std::cout << "  3. 结果: 约等于 (-0.707, 4.95)" << std::endl;
    
    std::cout << "注意: 变换顺序很重要" << std::endl;

    // ==================== 示例7: 仿射变换的应用 - 计算机图形学 ====================
    std::cout << "\n=== 示例7: 仿射变换的应用 - 计算机图形学 ===" << std::endl;
    
    std::cout << "计算机图形学中的仿射变换:" << std::endl;
    std::cout << "  1. 物体移动、旋转、缩放" << std::endl;
    std::cout << "  2. 相机变换" << std::endl;
    std::cout << "  3. 视口变换" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 游戏中的角色移动" << std::endl;
    std::cout << "  2. 电影中的动画" << std::endl;
    std::cout << "  3. 虚拟现实中的场景变换" << std::endl;

    // ==================== 示例8: 仿射变换的应用 - CAD/CAM ====================
    std::cout << "\n=== 示例8: 仿射变换的应用 - CAD/CAM ===" << std::endl;
    
    std::cout << "CAD/CAM 中的仿射变换:" << std::endl;
    std::cout << "  1. 零件移动、旋转、缩放" << std::endl;
    std::cout << "  2. 坐标系变换" << std::endl;
    std::cout << "  3. 刀具路径变换" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 零件在工作台上的定位" << std::endl;
    std::cout << "  2. 坐标系的转换" << std::endl;
    std::cout << "  3. 刀具路径的变换" << std::endl;

    // ==================== 示例9: 仿射变换的应用 - 机器人 ====================
    std::cout << "\n=== 示例9: 仿射变换的应用 - 机器人 ===" << std::endl;
    
    std::cout << "机器人中的仿射变换:" << std::endl;
    std::cout << "  1. 机器人手臂的运动" << std::endl;
    std::cout << "  2. 坐标系变换" << std::endl;
    std::cout << "  3. 传感器数据变换" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 机器人手臂的正运动学" << std::endl;
    std::cout << "  2. 机器人手臂的逆运动学" << std::endl;
    std::cout << "  3. 传感器数据的坐标变换" << std::endl;

    // ==================== 示例10: 仿射变换的应用 - 计算���视觉 ====================
    std::cout << "\n=== 示例10: 仿射变换的应用 - 计算机视觉 ===" << std::endl;
    
    std::cout << "计算机视觉中的仿射变换:" << std::endl;
    std::cout << "  1. 图像变换" << std::endl;
    std::cout << "  2. 相机标定" << std::endl;
    std::cout << "  3. 3D 重建" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. 图像的旋转、缩放" << std::endl;
    std::cout << "  2. 相机标定中的变换" << std::endl;
    std::cout << "  3. 3D 重建中的变换" << std::endl;

    // ==================== 示例11: 仿射变换的应用 - GIS ====================
    std::cout << "\n=== 示例11: 仿射变换的应用 - GIS ===" << std::endl;
    
    std::cout << "GIS 中的仿射变换:" << std::endl;
    std::cout << "  1. 坐标系变换" << std::endl;
    std::cout << "  2. 地图投影" << std::endl;
    std::cout << "  3. 地图缩放" << std::endl;
    
    std::cout << "示例:" << std::endl;
    std::cout << "  1. WGS84 到 UTM 的坐标变换" << std::endl;
    std::cout << "  2. 地图投影变换" << std::endl;
    std::cout << "  3. 地图缩放" << std::endl;

    // ==================== 示例12: 仿射变换的性质 ====================
    std::cout << "\n=== 示例12: 仿射变换的性质 ===" << std::endl;
    
    std::cout << "仿射变换的主要性质:" << std::endl;
    std::cout << "  1. 保持共线性: 共线点变换后仍共线" << std::endl;
    std::cout << "  2. 保持比例: 线段的比例保持不变" << std::endl;
    std::cout << "  3. 保持平行性: 平行线变换后仍平行" << std::endl;
    
    std::cout << "\n仿射变换的矩阵表示:" << std::endl;
    std::cout << "  [x']   [a b c] [x]" << std::endl;
    std::cout << "  [y'] = [d e f] [y]" << std::endl;
    std::cout << "  [1 ]   [0 0 1] [1]" << std::endl;
    std::cout << "  其中 (c, f) 是平移向量" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "   仿射变换示例运行完成！" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
