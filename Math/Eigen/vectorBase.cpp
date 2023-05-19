// Eigen 库对向量的基本操作与矩阵操作类似，下面列举了一些常用的 Eigen 向量操作：

// 创建向量：

// VectorXd：动态大小的向量。
// Vector2d、Vector3d、Vector4d：固定大小的向量。
// Vector 类有多种构造函数和初始化方式。
// 访问向量元素：

// 使用括号运算符 () 或 [] 访问元素。
// 使用 head()、tail()、segment() 函数访问子向量。
// 向量运算：

// 向量加法和减法：+ 和 - 运算符。
// 向量乘法和除法：* 和 / 运算符（乘法是按元素逐个相乘）。
// 标量乘法和除法：* 和 / 运算符。
// 向量点乘和叉乘：dot() 和 cross() 函数。
// 向量操作：

// 向量范数：norm() 函数。
// 向量归一化：normalized() 函数。
// 向量叉积：cross() 函数。
// 向量内积：dot() 函数。
// 其他功能：

// 向量大小和维度：size() 函数。
// 向量转换为数组或列表：data() 和 std::vector 转换函数。
// 这些是 Eigen 库中对向量进行常见操作的一些示例。Eigen 还提供了更多高级功能，如向量插值、求解线性方程组等。
// 你可以参考 Eigen 的文档和示例代码，了解更多关于 Eigen 向量操作的详细功能和用法。

#include <iostream>
#include <Eigen/Dense>

int main() {
    // 创建向量
    Eigen::Vector3d v1(1.0, 2.0, 3.0);
    Eigen::Vector3d v2(4.0, 5.0, 6.0);

    // 访问向量元素
    std::cout << "v1: " << v1 << std::endl;
    std::cout << "v2: " << v2 << std::endl;

    // 向量加法
    Eigen::Vector3d sum = v1 + v2;
    std::cout << "v1 + v2: " << sum << std::endl;

    // 向量减法
    Eigen::Vector3d diff = v1 - v2;
    std::cout << "v1 - v2: " << diff << std::endl;

    // 向量点乘
    double dotProduct = v1.dot(v2);
    std::cout << "v1 dot v2: " << dotProduct << std::endl;

    // 向量叉乘
    Eigen::Vector3d crossProduct = v1.cross(v2);
    std::cout << "v1 cross v2: " << crossProduct << std::endl;

    // 向量长度（范数）
    double norm = v1.norm();
    std::cout << "v1 norm: " << norm << std::endl;

    return 0;
}
