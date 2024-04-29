#include <iostream>

#include "Implementation.h"
#include "MainWindow.h"


#include <QApplication>
#include <QWidget>
#include <QApplication>


int main(int argc, char* argv[])
{
    Implementation imp;
    imp.bar();
    imp.foo();

    QApplication app(argc, argv);

    // 创建一个QWidget对象
    //QWidget window;
    //window.setWindowTitle("Hello Qt");
    //window.show();

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}