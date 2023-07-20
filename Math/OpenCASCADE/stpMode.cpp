#include <opencascade/TopoDS_Shape.hxx>
#include <opencascade/StlAPI_Writer.hxx>
#include <opencascade/vtkSmartPointer.h>
#include <opencascade/vtkSTLReader.h>
#include <opencascade/vtkPolyDataMapper.h>
#include <opencascade/vtkActor.h>
#include <opencascade/vtkRenderer.h>
#include <opencascade/vtkRenderWindow.h>
#include <opencascade/vtkRenderWindowInteractor.h>

int main(int argc, char *argv[])
{
    // 解析STEP文件
    TopoDS_Shape shape;
    STEPControl_Reader reader;
    reader.ReadFile("example.stp");
    reader.TransferRoots();
    shape = reader.OneShape();

    // 导出为STL文件
    StlAPI_Writer writer;
    writer.Write(shape, "output.stl");

    // 构建vtkPolyData
    vtkSmartPointer<vtkSTLReader> stlReader =
        vtkSmartPointer<vtkSTLReader>::New();
    stlReader->SetFileName("output.stl");
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