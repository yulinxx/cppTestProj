// find_package(VTK REQUIRED)

// include("${VTK_USE_FILE}")
// target_link_libraries(main PRIVATE ${VTK_LIBRARIES})

#include <opencascade/STEPControl_Reader.hxx>
#include <opencascade/TopoDS_Shape.hxx>
#include <opencascade/StlAPI_Writer.hxx>

#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

int main(int argc, char *argv[])
{
    // 解析STEP文件
    TopoDS_Shape shape;
    STEPControl_Reader reader;
    // reader.ReadFile("example.stp");
    reader.ReadFile("D:/a.stp");
    reader.TransferRoots();
    shape = reader.OneShape();

    // 导出为STL文件
    StlAPI_Writer writer;
    writer.Write(shape, "D:/a.stl");

    // 构建vtkPolyData
    vtkSmartPointer<vtkSTLReader> stlReader =
        vtkSmartPointer<vtkSTLReader>::New();
    stlReader->SetFileName("D:/a.stl");
    stlReader->Update();
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(stlReader->GetOutput());
    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // 显示
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.2, 0.3);
    vtkSmartPointer<vtkRenderWindow> renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);
    renderWindow->Render();
    renderWindowInteractor->Start();
    
    return 0;
}
