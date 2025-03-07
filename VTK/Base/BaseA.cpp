#include <vtkSmartPointer.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOutputWindow.h>
#include <GL/gl.h>
#include <iostream>

int main()
{
    // 设置 VTK 输出窗口
    vtkSmartPointer<vtkOutputWindow> outputWindow =
        vtkSmartPointer<vtkOutputWindow>::New();
    vtkOutputWindow::SetInstance(outputWindow);

    // 创建立方体几何体
    vtkSmartPointer<vtkCubeSource> cubeSource =
        vtkSmartPointer<vtkCubeSource>::New();
    cubeSource->SetXLength(1.0);
    cubeSource->SetYLength(1.0);
    cubeSource->SetZLength(1.0);

    // 创建映射器
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cubeSource->GetOutputPort());

    // 创建演员
    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1.0, 0.0, 0.0); // 设置为红色

    // 创建渲染器
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.2, 0.4); // 设置背景为深蓝色
    std::cout << "Renderer created" << std::endl;

    // 创建渲染窗口
    vtkSmartPointer<vtkRenderWindow> renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(800, 600);
    renderWindow->SetWindowName("VTK Cube Demo");
    std::cout << "Render window created" << std::endl;

    // 检查 OpenGL
    renderWindow->Render();
    const GLubyte* version = glGetString(GL_VERSION);
    if (version)
    {
        std::cout << "OpenGL Version: " << version << std::endl;
    }
    else
    {
        std::cerr << "OpenGL initialization failed!" << std::endl;
        return 1;
    }

    // 创建交互器
    vtkSmartPointer<vtkRenderWindowInteractor> interactor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow);
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    interactor->SetInteractorStyle(style);
    std::cout << "Interactor created" << std::endl;

    // 启动渲染和交互
    renderWindow->Render();
    std::cout << "Rendering started" << std::endl;
    interactor->Initialize();
    interactor->Start();
    std::cout << "Interactor started" << std::endl;

    return 0;
}