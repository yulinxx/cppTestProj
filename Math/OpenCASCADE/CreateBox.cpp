// ### 代码用途总结
// 这段代码借助 Open CASCADE 库实现了一个简单的三维建模和文件保存功能。具体来说：

// 1. 三维建模 ：创建了一个具有指定尺寸（ xLength 、 yLength 、 zLength ）的三维盒子模型。
// 2. 文件保存 ：将创建好的三维盒子模型保存为 STEP 格式的文件（ box.stp ），方便后续在其他支持 STEP 格式的软件中打开和使用。

#include <iostream>
// 引入 Open CASCADE 库的拓扑数据结构相关头文件
#include <opencascade/TopoDS.hxx>
// 引入 Open CASCADE 库中用于创建基本体（如盒子）的 API 头文件
#include <opencascade/BRepPrimAPI_MakeBox.hxx>
// 引入 Open CASCADE 库中用于处理边界表示（BRep）数据并进行文件读写的工具头文件
#include <opencascade/BRepTools.hxx>

/**
 * @brief 主函数，程序的入口点。
 *
 * 此程序的主要用途是利用 Open CASCADE 库创建一个三维的盒子形状，
 * 并将这个盒子的几何模型保存为 STEP 格式的文件。
 * STEP 文件是一种常见的用于交换三维几何数据的标准文件格式。
 *
 * @return int 程序的退出状态码，0 表示正常退出。
 */
int main()
{
    // 创建一个三维盒子
    // 定义盒子在 x 轴方向的长度
    Standard_Real xLength = 100.0;
    // 定义盒子在 y 轴方向的长度
    Standard_Real yLength = 50.0;
    // 定义盒子在 z 轴方向的长度
    Standard_Real zLength = 30.0;

    // 使用 BRepPrimAPI_MakeBox 类创建一个盒子形状
    // 该类是 Open CASCADE 库中用于创建基本体（如盒子）的工具类
    // 传入 x、y、z 方向的长度参数，得到一个表示盒子的 TopoDS_Shape 对象
    TopoDS_Shape boxShape = BRepPrimAPI_MakeBox(xLength, yLength, zLength);

    // 将盒子保存为 STEP 文件
    // BRepTools::Write 是 Open CASCADE 库中用于将边界表示（BRep）数据
    // 写入到文件中的工具函数
    // 这里将创建好的盒子形状 boxShape 写入到名为 "box.stp" 的 STEP 文件中
    BRepTools::Write(boxShape, "box.stp");

    // 输出提示信息，告知用户盒子已成功创建并保存为文件
    std::cout << "Box created and saved to 'box.stp'." << std::endl;

    // 返回 0 表示程序正常退出
    return 0;
}