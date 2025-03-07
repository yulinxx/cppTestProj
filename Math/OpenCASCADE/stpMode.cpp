// ### 代码用途总结
// 1. STEP 文件读取 ：使用 Open CASCADE 库的 STEPControl_Reader 类读取指定路径的 STEP 格式三维模型文件，并将其转换为 TopoDS_Shape 对象。
// 2. STL 文件转换 ：利用 Open CASCADE 库的 StlAPI_Writer 类将读取的三维模型形状保存为 STL 格式文件。
// 3. 模型可视化 ：借助 VTK 库，读取转换后的 STL 文件，将其映射为可渲染的图形对象，添加到渲染场景中，并通过渲染窗口显示出来，同时支持用户与模型进行交互操作。
// 通过这个程序，你可以实现不同格式三维模型文件的转换和可视化，有助于你在三维建模和可视化领域的学习和实践。

// 引入 Open CASCADE 库中用于读取 STEP 文件的类
#include <opencascade/STEPControl_Reader.hxx>
// 引入 Open CASCADE 库中表示拓扑形状的类
#include <opencascade/TopoDS_Shape.hxx>
// 引入 Open CASCADE 库中用于将拓扑形状写入 STL 文件的类
#include <opencascade/StlAPI_Writer.hxx>

// 引入 VTK 库的智能指针类，用于管理 VTK 对象的生命周期
#include <vtkSmartPointer.h>
// 引入 VTK 库中用于读取 STL 文件的类
#include <vtkSTLReader.h>
// 引入 VTK 库中用于将多边形数据映射到图形渲染的类
#include <vtkPolyDataMapper.h>
// 引入 VTK 库中表示场景中可渲染对象的类
#include <vtkActor.h>
// 引入 VTK 库中用于管理渲染场景的类
#include <vtkRenderer.h>
// 引入 VTK 库中用于创建渲染窗口的类
#include <vtkRenderWindow.h>
// 引入 VTK 库中用于处理渲染窗口交互事件的类
#include <vtkRenderWindowInteractor.h>

/**
 * @brief 主函数，程序的入口点。
 *
 * 此函数实现了从 STEP 文件读取三维模型，将其转换为 STL 文件，
 * 并使用 VTK 库进行可视化显示的功能。
 *
 * @param argc 命令行参数的数量。
 * @param argv 命令行参数的数组。
 * @return int 程序的退出状态码，0 表示正常退出。
 */
int main(int argc, char* argv[])
{
    // 定义一个 TopoDS_Shape 对象，用于存储从 STEP 文件中读取的三维模型形状
    TopoDS_Shape shape;
    // 创建一个 STEPControl_Reader 对象，用于读取 STEP 文件
    STEPControl_Reader reader;
    // 调用 ReadFile 方法读取指定路径的 STEP 文件
    // 这里指定的文件路径为 "D:/a.stp"，可根据实际情况修改
    reader.ReadFile("D:/a.stp");
    if (!reader.ReadFile("D:/a.stp"))
    {
        std::cerr << "Failed to read STEP file: D:/a.stp" << std::endl;
        return 1;
    }
    // 调用 TransferRoots 方法将读取的文件中的模型数据转换为 TopoDS_Shape 对象
    reader.TransferRoots();
    // 获取转换后的单个模型形状
    shape = reader.OneShape();
    if (shape.IsNull())
    {
        std::cerr << "The shape read from STEP file is null." << std::endl;
        return 1;
    }
    // 创建一个 StlAPI_Writer 对象，用于将 TopoDS_Shape 对象导出为 STL 文件
    StlAPI_Writer writer;
    // 调用 Write 方法将模型形状写入指定路径的 STL 文件
    // 这里指定的输出文件路径为 "D:/a.stl"，可根据实际情况修改
    writer.Write(shape, "D:/a.stl");
    if (!writer.Write(shape, "D:/a.stl"))
    {
        std::cerr << "Failed to write STL file: D:/a.stl" << std::endl;
        return 1;
    }
    // 创建一个 vtkSmartPointer<vtkSTLReader> 对象，用于读取 STL 文件
    vtkSmartPointer<vtkSTLReader> stlReader =
        vtkSmartPointer<vtkSTLReader>::New();
    if (!stlReader)
    {
        std::cerr << "Failed to create vtkSTLReader." << std::endl;
        return 1;
    }
    // 设置要读取的 STL 文件的路径
    stlReader->SetFileName("D:/a.stl");
    // 调用 Update 方法更新读取器，确保数据被正确读取
    stlReader->Update();
    // 创建一个 vtkSmartPointer<vtkPolyDataMapper> 对象，用于将读取的 STL 数据映射到图形表示
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    // 设置映射器的输入数据为读取器的输出数据
    mapper->SetInputData(stlReader->GetOutput());
    // 创建一个 vtkSmartPointer<vtkActor> 对象，用于在渲染场景中表示可视化对象
    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    // 设置演员的映射器为之前创建的映射器
    actor->SetMapper(mapper);

    // 创建一个 vtkSmartPointer<vtkRenderer> 对象，用于管理渲染场景
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
    // 将演员添加到渲染器中
    renderer->AddActor(actor);
    // 设置渲染器的背景颜色为 RGB 值 (0.1, 0.2, 0.3)，即一种偏暗的蓝绿色
    renderer->SetBackground(0.1, 0.2, 0.3);
    // 创建一个 vtkSmartPointer<vtkRenderWindow> 对象，用于显示渲染结果
    vtkSmartPointer<vtkRenderWindow> renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();
    // 将渲染器添加到渲染窗口中
    renderWindow->AddRenderer(renderer);
    // 创建一个 vtkSmartPointer<vtkRenderWindowInteractor> 对象，用于处理用户与渲染窗口的交互
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    // 设置交互器的渲染窗口为之前创建的渲染窗口
    renderWindowInteractor->SetRenderWindow(renderWindow);
    // 调用 Render 方法进行渲染
    renderWindow->Render();
    // 启动交互器，开始处理用户的交互事件，如鼠标拖动、缩放等
    renderWindowInteractor->Start();

    // 返回 0 表示程序正常退出
    return 0;
}