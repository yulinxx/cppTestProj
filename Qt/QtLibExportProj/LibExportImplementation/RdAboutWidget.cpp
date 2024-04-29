#include "RdAboutWidget.h"

#include <QVBoxLayout>

#include <QSettings>
#include <QFrame>
#include <QLabel>
#include <QGridLayout>

#include <iostream>

RdAboutWidget::RdAboutWidget(QWidget* parent)
    : IAboutWidget(parent)
{
    //setWindowFlags(Qt::Popup);
    setWindowFlag(Qt::Dialog);
    initWidget();
}

void RdAboutWidget::initWidget()
{
    //auto aboutData = mainWindow->GetConfigData()->getAboutPara();
    //setTitle(tr("MantiSOFT"));
    this->setWindowTitle("MantiSOFT");
    //setWebPath(mainWindow->GetConfigData()->getHelpData()->RdAboutWidget());
    //auto hlayout = new QHBoxLayout(contentWidget());
    auto hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(20, 20, 20, 20);
    setLayout(hlayout);

    auto frame = new QFrame(this);
    hlayout->addWidget(frame);

    auto gridLayout = new QGridLayout();
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(10);
    gridLayout->setContentsMargins(20, 20, 20, 20);

    QLabel* softLab;
    gridLayout->addWidget(m_logoLab = new QLabel, 0, 0, 2, 1, Qt::AlignLeft);
    gridLayout->addWidget(softLab = new QLabel(tr("MantiSOFT"), this), 0, 1, 1, 2, Qt::AlignLeft);
    gridLayout->addWidget(m_labelVersion = new QLabel(), 1, 1, 1, 1, Qt::AlignLeft);
    gridLayout->addWidget(m_labelDate = new QLabel(), 1, 2, 1, 1, Qt::AlignLeft);

    softLab->setObjectName("softLab");

    m_logoLab->setFixedSize(50, 50);
    //m_logoLab->setPixmap(g_icons->Rd_LogoIcon.pixmap(m_logoLab->size()));

    //m_labelVersion->setText(tr("Version:") + aboutData->Version());
    //m_labelDate->setText(tr("Release:") + aboutData->ReleaseDate());  // ReleaseTodo

    gridLayout->setRowMinimumHeight(2, 10);
    //gridLayout->addWidget(CreateHLine(this), 3, 0, 1, 5);
    gridLayout->setRowMinimumHeight(4, 10);

    QLabel* ManufactureLab;
    gridLayout->addWidget(ManufactureLab = new QLabel(tr("Manufacture"), this),
        5, 0, 1, 3, Qt::AlignLeft);
    auto vlayout = new QVBoxLayout;
    vlayout->setSpacing(10);
    vlayout->setContentsMargins(20, 0, 0, 0);
    gridLayout->addLayout(vlayout, 6, 0, 1, 5, Qt::AlignLeft);
    //vlayout->addWidget(new QLabel(aboutData->Manufacture()));
    //vlayout->addWidget(new QLabel(tr("Address:") + aboutData->Address()));
    //vlayout->addWidget(new QLabel(tr("Technical support:") + aboutData->TechnicalSupport()));
    //vlayout->addWidget(new QLabel(tr("Website:") + aboutData->Website()));
    //vlayout->addWidget(new QLabel(tr("Email:") + aboutData->Email()));
    ManufactureLab->setObjectName("OSLab");

    gridLayout->setRowMinimumHeight(7, 10);
    //gridLayout->addWidget(CreateHLine(this), 8, 0, 1, 5);
    gridLayout->setRowMinimumHeight(9, 10);

    QLabel* osLab;
    gridLayout->addWidget(osLab = new QLabel(tr("OS"), this), 10, 0, 1, 3, Qt::AlignLeft);
    vlayout = new QVBoxLayout;
    vlayout->setSpacing(10);
    vlayout->setContentsMargins(20, 0, 0, 0);
    gridLayout->addLayout(vlayout, 11, 0, 1, 5);
    vlayout->addWidget(m_systemLab = new QLabel());
    vlayout->addWidget(m_cpuLab = new QLabel());
    vlayout->addWidget(m_cpuRank = new QLabel(tr("ProcessorRank: 602")));
    osLab->setObjectName("OSLab");

    //m_systemLab->setText(QSysInfo::productType());
#ifdef Q_OS_WIN
    //通过注册表获取
    QSettings* CPU = new QSettings("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", QSettings::NativeFormat);
    QString cpuDescribe = CPU->value("ProcessorNameString").toString();
    delete CPU;
    m_cpuLab->setText(tr("Processor:") + cpuDescribe);
#elif defined(Q_OS_MAC)

    char buf_ps[1024];
    FILE* ptr;
    std::string CPUString;
    //通过管道方式来执行mac中的命令并获取返回值
    if ((ptr = popen("sysctl machdep.cpu.brand_string", "r")) != NULL)
    {
        while (fgets(buf_ps, 1024, ptr) != NULL)
        {
            //可以通过这行来获舰shell俞令行中的每一行的翰出
            CPUString.append(buf_ps);
        }
        pclose(ptr);
        ptr = NULL;
    }
    auto showString = QString::fromStdString(CPUString);
    showString.replace("machdep.cpu.brand_string:", "");
    showString.replace("\r", "");
    m_cpuLab->setText(showString);

#elif defined(Q_OS_LINUX)
    //从liux中的/proc/cpuinfo文件中获取cpu信息
    QFile cpuFile("/proc/cpuinfo");
    QString file;
    if (cpuFile.open(QIODevice::ReadOnly))
    {
        file.clear();
        file = cpuFile.readAll();
    }
    auto list = file.split("\n");
    for (auto ite : list)
    {
        if (ite.contains("model name"))
        {
            auto tempList = ite.split(":");
            if (tempList.size() >= 2)
                m_cpuLab->setText(tempList[1]);
        }
    }
#endif

    QString showString;
    showString.replace("\n", "");

    gridLayout->setColumnStretch(4, 1);
    frame->setLayout(gridLayout);
}