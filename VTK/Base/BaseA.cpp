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
    // ���� VTK �������
    vtkSmartPointer<vtkOutputWindow> outputWindow =
        vtkSmartPointer<vtkOutputWindow>::New();
    vtkOutputWindow::SetInstance(outputWindow);

    // ���������弸����
    vtkSmartPointer<vtkCubeSource> cubeSource =
        vtkSmartPointer<vtkCubeSource>::New();
    cubeSource->SetXLength(1.0);
    cubeSource->SetYLength(1.0);
    cubeSource->SetZLength(1.0);

    // ����ӳ����
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cubeSource->GetOutputPort());

    // ������Ա
    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1.0, 0.0, 0.0); // ����Ϊ��ɫ

    // ������Ⱦ��
    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.2, 0.4); // ���ñ���Ϊ����ɫ
    std::cout << "Renderer created" << std::endl;

    // ������Ⱦ����
    vtkSmartPointer<vtkRenderWindow> renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(800, 600);
    renderWindow->SetWindowName("VTK Cube Demo");
    std::cout << "Render window created" << std::endl;

    // ��� OpenGL
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

    // ����������
    vtkSmartPointer<vtkRenderWindowInteractor> interactor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow);
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    interactor->SetInteractorStyle(style);
    std::cout << "Interactor created" << std::endl;

    // ������Ⱦ�ͽ���
    renderWindow->Render();
    std::cout << "Rendering started" << std::endl;
    interactor->Initialize();
    interactor->Start();
    std::cout << "Interactor started" << std::endl;

    return 0;
}