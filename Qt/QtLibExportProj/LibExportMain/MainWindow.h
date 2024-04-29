#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QVBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void addSubDlg();

private:
    QVBoxLayout* m_pMainLayout{ nullptr };
};

#endif // MAINWINDOW_H