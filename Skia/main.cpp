#include <QApplication>
#include <QMainWindow>
#include "SkiaCircleWidget.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QMainWindow mainWindow;
    SkiaCircleWidget skiaWidget;
    mainWindow.setCentralWidget(&skiaWidget);
    mainWindow.resize(800, 600);
    mainWindow.show();

    return a.exec();
}