#include "Vec.h"
#include "Rect.h"
#include "Matrix.h"
#include <iostream>

// 添加到main.cpp的矩阵测试部分

void TestVec()
{
    // 使用二维向量和矩形
    Vector2f v2min(1.0f, 2.0f);
    Vector2f v2max(4.0f, 6.0f);
    std::cout << "2D Vector: (" << v2min.x() << ", " << v2min.y() << ")\n";
    std::cout << "2D Vector: (" << v2max.x() << ", " << v2max.y() << ")\n";

    Vector2f v3(4.0f, 0.0f);
    std::cout << "Len:" << v3.Length() << std::endl;

    // 使用三维向量和矩形
    Vector3d v3min(1.0, 2.0, 3.0);
    Vector3d v3max(4.0, 6.0, 8.0);
    std::cout << "Len:" << v3min.Length() << std::endl;
    std::cout << "Len:" << v3max.Length() << std::endl;

    Vector2f min2d(1.0f, 2.0f);
    Vector2f max2d(5.0f, 6.0f);
    Rect2f rect2d(min2d, max2d);

    std::cout << "2D矩形信息:" << std::endl;
    std::cout << "宽度: " << rect2d.Width() << std::endl;
    std::cout << "高度: " << rect2d.Height() << std::endl;
    std::cout << "面积: " << rect2d.Area() << std::endl;

    {
        Vector3d min3da(1.0, 2.0, 3.0);
        Vector3d max3db(5.0, 6.0, 7.0);
    }
    {
        Vector3d min3da(1.0, 2.0, 3.0);
        Vector3d max3db(5.0, 6.0, 7.0);
        Rect3d rect3d(min3da, max3db);
    }

    // 创建3D矩形
    Vector3d min3d(1.0, 2.0, 3.0);
    Vector3d max3d(5.0, 6.0, 7.0);
    Rect3d rect3d(min3d, max3d);

    std::cout << "\n3D矩形信息:" << std::endl;
    std::cout << "宽度: " << rect3d.Width() << std::endl;
    std::cout << "高度: " << rect3d.Height() << std::endl;
    std::cout << "深度: " << rect3d.Depth() << std::endl;
    std::cout << "体积: " << rect3d.Volume() << std::endl;
}

void TestRect()
{
    Vector2f min2d(1.0f, 2.0f);
    Vector2f max2d(5.0f, 6.0f);
    Rect2f rect2d(min2d, max2d);

    std::cout << "2D矩形信息:" << std::endl;
    std::cout << "宽度: " << rect2d.Width() << std::endl;
    std::cout << "高度: " << rect2d.Height() << std::endl;
    std::cout << "面积: " << rect2d.Area() << std::endl;

    // 创建3D矩形
    Vector3d min3d(1.0, 2.0, 3.0);
    Vector3d max3d(5.0, 6.0, 7.0);
    Rect3d rect3d(min3d, max3d);

    std::cout << "\n3D矩形信息:" << std::endl;
    std::cout << "宽度: " << rect3d.Width() << std::endl;
    std::cout << "高度: " << rect3d.Height() << std::endl;
    std::cout << "深度: " << rect3d.Depth() << std::endl;
    std::cout << "体积: " << rect3d.Volume() << std::endl;
}

void TestMatrix()
{
    // 测试 2x2 矩阵
    {
        Matrix2d m2(1.0, 2.0, 3.0, 4.0); // [[1, 2], [3, 4]]
        std::cout << "Matrix 2x2:\n";
        std::cout << m2(0, 0) << " " << m2(0, 1) << "\n";
        std::cout << m2(1, 0) << " " << m2(1, 1) << "\n";
        std::cout << "Determinant: " << m2.Determinant() << "\n"; // 1*4 - 2*3 = -2
    }

    // 测试 3x3 矩阵
    {
        Matrix3d m3(1.0, 2.0, 3.0,
            0.0, 1.0, 4.0,
            5.0, 6.0, 0.0); // [[1, 2, 3], [0, 1, 4], [5, 6, 0]]
        std::cout << "\nMatrix 3x3:\n";
        std::cout << m3(0, 0) << " " << m3(0, 1) << " " << m3(0, 2) << "\n";
        std::cout << m3(1, 0) << " " << m3(1, 1) << " " << m3(1, 2) << "\n";
        std::cout << m3(2, 0) << " " << m3(2, 1) << " " << m3(2, 2) << "\n";
        std::cout << "Determinant: " << m3.Determinant() << "\n"; // 计算结果应为 -17
    }

    // 测试 4x4 矩阵
    {
        Matrix4d m4(1.0, 0.0, 0.0, 2.0,
            0.0, 1.0, 3.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            4.0, 0.0, 0.0, 1.0); // [[1, 0, 0, 2], [0, 1, 3, 0], [0, 0, 1, 0], [4, 0, 0, 1]]
        std::cout << "\nMatrix 4x4:\n";
        std::cout << m4(0, 0) << " " << m4(0, 1) << " " << m4(0, 2) << " " << m4(0, 3) << "\n";
        std::cout << m4(1, 0) << " " << m4(1, 1) << " " << m4(1, 2) << " " << m4(1, 3) << "\n";
        std::cout << m4(2, 0) << " " << m4(2, 1) << " " << m4(2, 2) << " " << m4(2, 3) << "\n";
        std::cout << m4(3, 0) << " " << m4(3, 1) << " " << m4(3, 2) << " " << m4(3, 3) << "\n";
        std::cout << "Determinant: " << m4.Determinant() << "\n"; // 计算结果应为 -6
    }
}

int main()
{
    {
        {
            Vector<3, double> min3da(1.0, 2.0, 3.0);
            Vector<3, double> max3db(5.0, 6.0, 7.0);
        }

        {
        } // rect3d 超出作用域，析构

        std::cout << "Exited scope safely.\n";
    }

    TestVec();
    TestRect();
    // TestMatrix();

    std::cout << "Hello World" << std::endl;
    return 0;
}