// main.cpp
#include <QApplication>
#include "GLTestWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSurfaceFormat fmt;
    fmt.setVersion(4, 6);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSamples(4);
    QSurfaceFormat::setDefaultFormat(fmt);

    GLTestWidget w;
    w.resize(1200, 800);
    w.show();

    return a.exec();
}