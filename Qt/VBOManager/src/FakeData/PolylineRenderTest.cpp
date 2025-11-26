
#include "FakeData/PolylineRenderTest.h"
#include <QApplication>
#include <iostream>

namespace GLRhi
{
    // 检查是否已有QApplication实例
    bool hasQApplication()
    {
        return QCoreApplication::instance() != nullptr;
    }

    // 创建并显示渲染窗口的函数
    void showPolylineRenderWindow(PolylinesVboManager* vboManager)
    {
        // 检查是否已有QApplication实例
        if (!hasQApplication())
        {
            int argc = 0;
            new QApplication(argc, nullptr);
        }

        // 创建渲染窗口
        SimpleRenderWindow* renderWindow = new SimpleRenderWindow(vboManager);
        renderWindow->setWindowTitle("Polyline Render Test");
        renderWindow->resize(800, 600);
        renderWindow->show();

        // 设置定时器定期更新窗口
        QTimer* timer = new QTimer(renderWindow);
        QObject::connect(timer, &QTimer::timeout, renderWindow, [renderWindow]() {
            renderWindow->update();
            });

        timer->start(16); // 约60fps

        //std::cout << "渲染窗口已创建并显示，请手动关闭窗口继续程序" << std::endl;

        //// 运行事件循环（如果这是主程序）
        //if (QCoreApplication::instance()->thread() == QThread::currentThread())
        //{
        //    QApplication::exec();
        //}
    }
}