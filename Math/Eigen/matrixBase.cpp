// Eigen 是一个强大的 C++ 线性代数库,提供了许多用于矩阵和向量操作的功能.下面列举了一些 Eigen 库中常用的矩阵操作:

// 矩阵创建、加法、乘法、转置、求逆和行列式计算,
// 线性方程组求解、特征值和特征向量计算、矩阵大小和维度获取、行和列访问以及子矩阵访问等常用函数的示例

// 1. 创建矩阵和向量:
//    - `MatrixXd`:动态大小的矩阵.
//    - `Matrix3d`、`Matrix4d`:固定大小的 3x3 和 4x4 矩阵.
//    - `VectorXd`:动态大小的向量.
//    - `Vector3d`、`Vector4d`:固定大小的 3D 和 4D 向量.
//    - `Matrix` 和 `Vector` 类有多种构造函数和初始化方式.

// 2. 访问矩阵和向量元素:
//    - 使用括号运算符 `()` 或 `[]` 访问元素.
//    - 使用 `row()` 和 `col()` 函数访问行和列.
//    - 使用 `block()` 函数访问子矩阵.

// 3. 矩阵运算:
//    - 矩阵加法和减法:`+` 和 `-` 运算符.
//    - 矩阵乘法:`*` 运算符.
//    - 标量乘法和除法:`*` 和 `/` 运算符.
//    - 转置矩阵:`transpose()` 函数.

// 4. 矩阵操作:
//    - 矩阵逆:`inverse()` 函数.
//    - 矩阵求解线性方程组:`solve()` 函数.
//    - 矩阵行列式:`determinant()` 函数.
//    - 特征值和特征向量:`eigenvalues()` 和 `eigenvectors()` 函数.
//    - 广义特征值和特征向量:`generalizedEigenvalues()` 和 `generalizedEigenvectors()` 函数.

// 5. 其他功能:
//    - 矩阵大小和维度:`rows()` 和 `cols()` 函数.
//    - 矩阵转换为数组或列表:`data()` 和 `std::vector` 转换函数.
//    - 读取和写入矩阵数据:`read()` 和 `write()` 函数.

// 这些仅是 Eigen 库中的一些基本操作,Eigen 还提供了许多其他功能,如矩阵分解(LU 分解、QR 分解等)、矩阵求解器、矩阵块操作等.
// 你可以参考 Eigen 的文档和示例代码,了解更多关于 Eigen 库的详细功能和用法.

#include <iostream>
#include <Eigen/Dense>

int main()
{
    // 创建矩阵
    Eigen::Matrix3d m1;
    m1 << 1.0, 2.0, 3.0,
        4.0, 5.0, 6.0,
        7.0, 8.0, 9.0;

    Eigen::Matrix3d m2;
    m2 << 10.0, 11.0, 12.0,
        13.0, 14.0, 15.0,
        16.0, 17.0, 18.0;

    // 访问矩阵元素
    std::cout << std::endl
        << "m1: \n"
        << m1 << std::endl;
    std::cout << std::endl
        << "m2: \n"
        << m2 << std::endl;

    // 矩阵加法
    Eigen::Matrix3d sum = m1 + m2;
    std::cout << std::endl
        << "m1 + m2: \n"
        << sum << std::endl;

    // 矩阵乘法
    Eigen::Matrix3d product = m1 * m2;
    std::cout << std::endl
        << "m1 * m2: \n"
        << product << std::endl;

    // 矩阵转置
    Eigen::Matrix3d transposed = m1.transpose();
    std::cout << std::endl
        << "m1 transposed: \n"
        << transposed << std::endl;

    // 矩阵求逆
    Eigen::Matrix3d inverse = m1.inverse();
    std::cout << "m1 inverse: \n"
        << inverse << std::endl;

    // 矩阵行列式
    double determinant = m1.determinant();
    std::cout << std::endl
        << "m1 determinant: " << determinant << std::endl;

    // 矩阵求解线性方程组 Ax = b
    Eigen::Vector3d b(1.0, 2.0, 3.0);
    Eigen::Vector3d x = m1.solve(b);
    std::cout << std::endl
        << "Solution of m1 * x = b: \n"
        << x << std::endl;

    // 矩阵特征值
    Eigen::EigenSolver<Eigen::Matrix3d> es(m1);
    std::cout << std::endl
        << "Eigenvalues of m1: \n"
        << es.eigenvalues() << std::endl;

    // 矩阵特征向量
    std::cout << std::endl
        << "Eigenvectors of m1: \n"
        << es.eigenvectors() << std::endl;

    // 矩阵大小和维度
    std::cout << std::endl
        << "m1 rows: " << m1.rows() << ", cols: " << m1.cols() << std::endl;

    // 访问矩阵的行和列
    std::cout << std::endl
        << "m1 first row: \n"
        << m1.row(0) << std::endl;
    std::cout << std::endl
        << "m1 first column: \n"
        << m1.col(0) << std::endl;

    // 访问矩阵的子矩阵
    Eigen::Matrix2d submatrix = m1.block(0, 0, 2, 2);
    std::cout << std::endl
        << "2x2 submatrix of m1: \n"
        << submatrix << std::endl;

    return 0;
}