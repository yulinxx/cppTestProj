// https://github.com/md748/QBreakpad

#include "MainWindow.h"

#include <QApplication>

#include "QBreakpad.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QBreakpad::instance()->Init(".", a.arguments().first());
    // QBreakpad::instance()->SetReporter("../bin/reporter.exe");
    QBreakpad::instance()->AppendInfoFile("./breakpad_log.txt");
    QBreakpad::instance()->AppendInfoFile("../breakpad_config.init");

    MainWindow w;
    w.show();
    return a.exec();
}
