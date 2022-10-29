#include "MainWindow.h"
#include "./ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnCrash, SIGNAL(clicked()), this, SLOT(onBtnCrash()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onBtnCrash()
{
    int a = 1;
    int b = 0;
    int n = a / b;
}

