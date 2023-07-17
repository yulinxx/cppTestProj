
#include <iostream>
#include <opencascade/TopoDS.hxx>
#include <opencascade/BRepPrimAPI_MakeBox.hxx>
#include <opencascade/BRepTools.hxx>

int main() 
{
    // 创建一个三维盒子
    Standard_Real xLength = 100.0; // x轴长度
    Standard_Real yLength = 50.0;  // y轴长度
    Standard_Real zLength = 30.0;  // z轴长度

    TopoDS_Shape boxShape = BRepPrimAPI_MakeBox(xLength, yLength, zLength);

    // 将盒子保存为STEP文件
    BRepTools::Write(boxShape, "box.stp");

    std::cout << "Box created and saved to 'box.stp'." << std::endl;

    return 0;
}
