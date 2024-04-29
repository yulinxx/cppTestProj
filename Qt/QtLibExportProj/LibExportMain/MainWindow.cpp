#include "MainWindow.h"

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

#include "RdAboutWidget.h"
#include "moc_RdAboutWidget.cpp"

MainWindow::MainWindow(QWidget* parent /*=nullptr*/) :
    QMainWindow(parent)
{
    // 设置窗口标题
    setWindowTitle("My Qt MainWindow");

    // 创建一个中心部件
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建一个标签并添加到中心部件
    QLabel* label = new QLabel("MainWindow", centralWidget);
    label->setAlignment(Qt::AlignCenter);

    // 设置布局（这里我们简单地将标签放在中心）
    m_pMainLayout = new QVBoxLayout(centralWidget);
    m_pMainLayout->addWidget(label);

    addSubDlg();
}

MainWindow::~MainWindow()
{
    // 在这里进行必要的清理工作
}

void MainWindow::addSubDlg()
{
    RdAboutWidget* pSubWidget = new RdAboutWidget(this);

    m_pMainLayout->addWidget(pSubWidget);
}