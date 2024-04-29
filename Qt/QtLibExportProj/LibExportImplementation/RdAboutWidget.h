#ifndef RDABOUTWIDGET_H
#define RDABOUTWIDGET_H

#include "IAboutWidget.h"
class QLabel;

class EXPORT_API RdAboutWidget : public IAboutWidget
{
    Q_OBJECT
public:
    RdAboutWidget(QWidget* parent = nullptr);

private:
    void initWidget();

private:
    QLabel* m_logoLab = nullptr;
    QLabel* m_labelVersion = nullptr;
    QLabel* m_labelDate = nullptr;

    QLabel* m_systemLab = nullptr;
    QLabel* m_cpuLab = nullptr;
    QLabel* m_cpuRank = nullptr;

protected slots:
    void Comfirm()
    {
    };
    void Cancel()
    {
    };
};

#endif //RDABOOUTWIDGET_H
