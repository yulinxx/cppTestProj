#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "ImgProcess.h"

#include <QImage>
#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>

using namespace ImgSpace;

#define CONNECT_BTN(N)                                                                        \
    {                                                                                         \
        connect(ui->pushBtnTest_##N, &QPushButton::clicked, this, &MainWindow::slotImageSet); \
        ui->pushBtnTest_##N->setProperty("Index", #N);                                        \
    }

#define CONNECT_SLIDER(N) connect(ui->hSlider##N, &QSlider::sliderReleased, \
                                  this, &MainWindow::slotImageSet);

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_pImgProcess = new imgProcess();
    assert(m_pImgProcess);

    connect(ui->pushBtnFit, &QPushButton::clicked, this, &MainWindow::slotImageFit);     // 自适应
    connect(ui->pushBtnTest_1, &QPushButton::clicked, this, &MainWindow::slotOpenImage); // 打开位图
    connect(ui->pushBtnSave, &QPushButton::clicked, this, &MainWindow::slotImageSave);   // 保存

    // connect(ui->pushBtnTest_0, &QPushButton::clicked, this, &MainWindow::slotImageSet);  // 效果叠加

    ui->pushBtnTest_0->setProperty("Index", "0");
    ui->pushBtnTest_1->setProperty("Index", "1");

    CONNECT_BTN(2); // 灰度
    CONNECT_BTN(3); // 红
    CONNECT_BTN(4); // 绿
    CONNECT_BTN(5); // 蓝

    CONNECT_BTN(6);
    CONNECT_BTN(7);
    CONNECT_BTN(8);
    CONNECT_BTN(9);  // 旋转
    CONNECT_BTN(10); // 缩放

    CONNECT_BTN(11); // 顺90度
    CONNECT_BTN(12); // 腐蚀
    CONNECT_BTN(13); // 模糊
    CONNECT_BTN(14); // 提边
    CONNECT_BTN(15); // 二值化

    CONNECT_BTN(16); // 亮度对比度
    CONNECT_BTN(17); //图像锐化(image sharpening)
    CONNECT_BTN(18); //绘制轮廓
    CONNECT_BTN(19); //几种滤波
    CONNECT_BTN(20); //马赛克

    CONNECT_BTN(21); //浮雕
    CONNECT_BTN(22); //素描
    CONNECT_BTN(23);
    CONNECT_BTN(24);
    CONNECT_BTN(25);

    CONNECT_BTN(26);
    CONNECT_BTN(27);
    CONNECT_BTN(28);
    CONNECT_BTN(29);
    CONNECT_BTN(30);

    CONNECT_BTN(31);
    CONNECT_BTN(32);
    CONNECT_BTN(33);
    CONNECT_BTN(34);
    CONNECT_BTN(35);

    CONNECT_BTN(36);
    CONNECT_BTN(37);
    CONNECT_BTN(38);
    CONNECT_BTN(39);
    CONNECT_BTN(40);

    CONNECT_BTN(41); // Gamma
    CONNECT_BTN(42); // 测试边缘挂网

    connect(ui->pushBtnTest_reset, &QPushButton::clicked, this, &MainWindow::slotValueReset); // 复原
    connect(ui->pushBtnTest_zero, &QPushButton::clicked, this, &MainWindow::slotZeroSlide);   // 归零
    connect(ui->hSliderResize, &QSlider::sliderReleased, this, &MainWindow::slotImageResize);

    CONNECT_SLIDER(A);
    CONNECT_SLIDER(B);
    CONNECT_SLIDER(C);
    CONNECT_SLIDER(D);

    CONNECT_SLIDER(E);
    CONNECT_SLIDER(F);
    CONNECT_SLIDER(G);
    CONNECT_SLIDER(H);
}

MainWindow::~MainWindow()
{
    m_matRes.release();
    m_matResPrev.release();

    delete ui;
}

void MainWindow::initSlider()
{
    ui->labelTipA->setText(QString::number(0));
    ui->labelTipB->setText(QString::number(0));
    ui->labelTipC->setText(QString::number(0));
    ui->labelTipD->setText(QString::number(0));
    ui->labelTipE->setText(QString::number(0));
    ui->labelTipF->setText(QString::number(0));
    ui->labelTipG->setText(QString::number(0));
    ui->labelTipH->setText(QString::number(0));

    ui->hSliderA->setValue(0);
    ui->hSliderB->setValue(0);
    ui->hSliderC->setValue(0);
    ui->hSliderD->setValue(0);
    ui->hSliderE->setValue(0);
    ui->hSliderF->setValue(0);
    ui->hSliderG->setValue(0);
    ui->hSliderH->setValue(0);
}

void MainWindow::initData()
{
    m_nType = -1;
    m_nTypePrev = -1;
    m_matRes = Mat();
    m_matResPrev = Mat();
    this->slotValueReset();

    initSlider();
}

void MainWindow::setSliderTip(QStringList listTip)
{
    if (m_vecLabelName.size() < 1)
    {
        m_vecLabelName.append(ui->labelNameA);
        m_vecLabelName.append(ui->labelNameB);
        m_vecLabelName.append(ui->labelNameC);
        m_vecLabelName.append(ui->labelNameD);
        m_vecLabelName.append(ui->labelNameE);
        m_vecLabelName.append(ui->labelNameF);
        m_vecLabelName.append(ui->labelNameG);
        m_vecLabelName.append(ui->labelNameH);
    }

    size_t nCount = m_vecLabelName.size() > listTip.size() ? listTip.size() : m_vecLabelName.size();
    for (int i = 0; i < m_vecLabelName.size(); i++)
    {
        m_vecLabelName[i]->setText("");
    }

    for (int i = 0; i < nCount; i++)
    {
        m_vecLabelName[i]->setText(listTip[i]);
    }
}

void MainWindow::slotImageResize()
{
    int nA = ui->hSliderResize->value();
    Mat matResize = m_pImgProcess->resizeImg(m_matResPrev, nA);

    cv::Mat matImg;
    QImage qImg;

    QImage::Format f = QImage::Format_BGR888;
    if (matResize.channels() < 3)
        f = QImage::Format_Grayscale8;

    qImg = QImage((const unsigned char*)(matResize.data),
        matResize.cols, matResize.rows, matResize.cols * matResize.channels(),
        f);

    ui->labelImg->clear();
    ui->labelImg->setPixmap(QPixmap::fromImage(qImg));

    auto p = ui->labelImg->pixmap(Qt::ReturnByValue).size();
    ui->labelImg->resize(p);

    float dSize = ui->hSliderResize->value() * 0.1;
    ui->labelResize->setText(QString("%1").arg(dSize));
}

void MainWindow::slotImageFit()
{
    QSize imgSize = ui->labelImg->size();
    // int nW = imgSize.width();
    // int nH = imgSize.height();

    double dLabelW = 1360.0;
    double dLabelH = 950.0;

    double dImgW = m_matResPrev.cols;
    double dImgH = m_matResPrev.rows;

    double dImgRatio = dImgW / dImgH;
    double dLabelRatio = dLabelW / dLabelH;

    double dResizeRatio = 1.0;
    if (dImgRatio > dLabelRatio)
        dResizeRatio = dLabelH / dImgH;
    else // 适应宽
        dResizeRatio = dLabelW / dImgW;

    Mat matResize = m_pImgProcess->resizeImg(m_matResPrev, dResizeRatio);

    cv::Mat matImg;
    QImage qImg;

    QImage::Format f = QImage::Format_BGR888;
    if (matResize.channels() < 3)
        f = QImage::Format_Grayscale8;

    qImg = QImage((const unsigned char*)(matResize.data),
        matResize.cols, matResize.rows, matResize.cols * matResize.channels(),
        f);

    ui->labelImg->clear();
    ui->labelImg->setPixmap(QPixmap::fromImage(qImg));
    ui->labelImg->resize(ui->labelImg->pixmap(Qt::ReturnByValue).size());

    float dSize = ui->hSliderResize->value() * 0.1f;
    ui->labelResize->setText(QString("%1").arg(dSize));
}

void MainWindow::slotOpenImage()
{
    ui->labelProcessTip->setText("Start...");
    QPushButton* pushBtn = qobject_cast<QPushButton*>(sender());
    if (!pushBtn)
    {
        std::cout << "Null" << std::endl;
        return;
    }

    QString strText = pushBtn->text();
    qDebug() << strText;

    QImage::Format f = QImage::Format_BGR888;

    m_nType = pushBtn->objectName().toInt();

    int nA = ui->hSliderA->value();
    int nB = ui->hSliderB->value();
    int nC = ui->hSliderC->value();
    int nD = ui->hSliderD->value();

    QString strImgPath;
    const QStringList picturesLocation = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    if (picturesLocation.isEmpty())
        strImgPath = "./";
    else
        strImgPath = QString("%1").arg(picturesLocation.first());

    QString strFileName = QFileDialog::getOpenFileName(
        this, tr("Open Image"), strImgPath, tr("Image File(*.bmp *.jpg *.jpeg *.png *.webp *.jfif)"));
    std::string filePath = strFileName.toStdString();

    /////////////////////////////////////////////
    if (!m_pImgProcess)
        m_pImgProcess = new imgProcess();
    m_matRes = m_pImgProcess->readImg(filePath);
    m_matResPrev = m_matRes;
    /////////////////////////////////////////////

    QImage qImg = QImage((const unsigned char*)(m_matRes.data),
        m_matRes.cols, m_matRes.rows, m_matRes.cols * m_matRes.channels(),
        f);

    ui->labelImg->clear();
    ui->labelImg->setPixmap(QPixmap::fromImage(qImg));
    ui->labelImg->resize(ui->labelImg->pixmap(Qt::ReturnByValue).size());
}

void MainWindow::slotImageSave()
{
    if (!m_matResPrev.data)
        return;

    QString strImgPath;
    const QStringList picturesLocation = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    if (picturesLocation.isEmpty())
        strImgPath = "./";
    else
        strImgPath = QString("%1").arg(picturesLocation.first());

    QDateTime dateTime = QDateTime::currentDateTime();
    // QDateTime dateTime = QDateTime::currentDateTime();
    QString strData = dateTime.toString("yyyy-MM-dd_hh_mm_ss");
    QString strPath = QString("%1/Out_%2.png").arg(strImgPath).arg(strData);

    m_pImgProcess->saveImg(m_matResPrev, strPath.toStdString());
}

// 复原
void MainWindow::slotValueReset()
{
    if (!m_pImgProcess || !m_matResPrev.data)
        return;

    initSlider();

    Mat matImg = m_pImgProcess->getOriginImg();
    if (!matImg.data)
        return;

    QImage::Format f = QImage::Format_BGR888;
    if (matImg.channels() < 3)
        f = QImage::Format_Grayscale8;

    QImage qImg = QImage((const unsigned char*)(matImg.data),
        matImg.cols, matImg.rows, matImg.cols * matImg.channels(),
        f);

    ui->labelImg->clear();
    ui->labelImg->setPixmap(QPixmap::fromImage(qImg));
    ui->labelImg->resize(ui->labelImg->pixmap(Qt::ReturnByValue).size());

    m_matResPrev = m_pImgProcess->getOriginImg();
}

void MainWindow::slotZeroSlide()
{
    initSlider();
}

void MainWindow::slotSlideValue()
{
    int nA = ui->hSliderA->value();
    ui->labelTipA->setText(QString::number(nA));

    int nB = ui->hSliderB->value();
    ui->labelTipB->setText(QString::number(nB));

    int nC = ui->hSliderC->value();
    ui->labelTipC->setText(QString::number(nC));

    int nD = ui->hSliderD->value();
    ui->labelTipD->setText(QString::number(nD));

    int nE = ui->hSliderE->value();
    ui->labelTipE->setText(QString::number(nE));

    int nF = ui->hSliderF->value();
    ui->labelTipF->setText(QString::number(nF));

    int nG = ui->hSliderG->value();
    ui->labelTipG->setText(QString::number(nG));

    int nH = ui->hSliderH->value();
    ui->labelTipH->setText(QString::number(nH));
}

// 进行图像处理
void MainWindow::slotImageSet()
{
    QTime startTime = QTime::currentTime();

    ui->labelProcessTip->setText("Start...");
    this->update();

    QPushButton* pushBtn = qobject_cast<QPushButton*>(sender());
    if (pushBtn) // 按钮
        m_nType = pushBtn->property("Index").toInt();
    else // 滑动块
        slotSlideValue();

    if (m_nType != m_nTypePrev) // 切换按钮,在上次的效果上进行处理
    {
        if (!m_matResPrev.data)
            m_matResPrev = m_pImgProcess->getOriginImg();

        m_matRes = m_matResPrev;
        m_nTypePrev = m_nType;
    }

    int nA = ui->hSliderA->value();
    int nB = ui->hSliderB->value();
    int nC = ui->hSliderC->value();
    int nD = ui->hSliderD->value();

    int nE = ui->hSliderE->value();
    int nF = ui->hSliderF->value();
    int nG = ui->hSliderG->value();
    int nH = ui->hSliderH->value();

    ui->labelTipA->setText(QString::number(nA));
    ui->labelTipB->setText(QString::number(nB));
    ui->labelTipC->setText(QString::number(nC));
    ui->labelTipD->setText(QString::number(nD));

    ui->labelTipE->setText(QString::number(nE));
    ui->labelTipF->setText(QString::number(nF));
    ui->labelTipG->setText(QString::number(nG));
    ui->labelTipH->setText(QString::number(nH));

    QStringList listName;

    Mat matRes;
    ////////////////////////////////////////////////////
    {
        switch (m_nType)
        {
        case 0: // 混合
            //    matRes = m_pImgProcess->setImgMix(m_matRes, );
            break;
        case 2: // 灰度
            matRes = m_pImgProcess->getGray(m_matRes);
            break;
        case 3: // 红
            matRes = m_pImgProcess->getRedChannel(m_matRes);
            break;
        case 4: // 绿
            matRes = m_pImgProcess->getGreeChannel(m_matRes);
            break;
        case 5: // 蓝
            matRes = m_pImgProcess->getBlueChannel(m_matRes);
            break;
        case 6: // 红+绿
            matRes = m_pImgProcess->getRGChannel(m_matRes);
            break;
        case 7: // 红+蓝
            matRes = m_pImgProcess->getRBChannel(m_matRes);
            break;
        case 8: // 绿+蓝
            matRes = m_pImgProcess->getGBChannel(m_matRes);
            break;
        case 9: // 旋转
            listName << "角度";
            matRes = m_pImgProcess->setRotateImg(m_matRes, nA);
            break;
        case 10: // 缩放
            listName << "倍率x0.1";
            matRes = m_pImgProcess->setScaleImg(m_matRes, nA * 0.1);
            break;
        case 11: // 顺90度
            listName << "方向";
            matRes = m_pImgProcess->rotate90(m_matRes, nA);
            break;
        case 12: // 腐蚀
            listName << "A"
                << "B"
                << "C";
            matRes = m_pImgProcess->setErodeImg(m_matRes, nA, nB, nC);
            break;
        case 13: // 模糊
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setBlurImg(m_matRes, nA, nB);
            break;
        case 14: // 提边"
            listName << "A"
                << "B"
                << "C"
                << "D";
            matRes = m_pImgProcess->setCannyImg(m_matRes, nA, nB, nC, nD);
            break;
        case 15: // 二值化
            listName << "A"
                << "B"
                << "C";
            matRes = m_pImgProcess->setThreshold(m_matRes, nA, nB, nC);
            break;
        case 16: // 亮度对比度
            listName << "A"
                << "B"
                << "C";
            matRes = m_pImgProcess->setContrastAndBright(m_matRes, nA, nB, nC);
            break;
        case 17: //图像锐化(image sharpening)
            listName << "A"
                << "B"
                << "C";
            matRes = m_pImgProcess->setSharpening(m_matRes, nA, nB, nC);
            break;
        case 18: // 绘制轮廓
            listName << "A";
            matRes = m_pImgProcess->setImgContours(m_matRes, nA);
            break;
        case 19: // 几种滤波
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setImgBlur(m_matRes, nA, nB);
            break;
        case 20: // 马赛克
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setMosaic(m_matRes, nA, nB);
            break;
        case 21: // 浮雕
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setRelief(m_matRes, nA, nB);
            break;
        case 22: // 素描
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setImgSketch(m_matRes, nA, nB);
            break;
        case 23: // 颜色变换
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setColorStyle(m_matRes, nA);
            break;
        case 24: // 油画
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setImgOilPaint(m_matRes, nA, nB);
            break;
        case 25: // 报纸
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setDotPaint(m_matRes, nA, nB);
            break;
        case 26: // 散点图
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setDither(m_matRes, nA, nB);
            break;
        case 27:
            matRes = m_pImgProcess->setDither(m_matRes, nA, nB, nC, nD, nE);
            break;
        case 28: // 色彩调节

            break;
        case 29: // 反色
            matRes = m_pImgProcess->setColorReversal(m_matRes);
            break;
        case 30: // 镜像
            listName << "X镜像"
                << "Y镜像";
            matRes = m_pImgProcess->setImgMirror(m_matRes, nA);
            break;
        case 31:
            listName << "A"
                << "B";
            matRes = m_pImgProcess->setBeautify(m_matRes, nA, nB);
            break;
        case 32: // 自动白平衡
            listName << "A"
                << "B"
                << "C";
            matRes = m_pImgProcess->setAutoWhithBalance(m_matRes, nA, nB, nC);
            break;
        case 33: // 色温
            listName << "A";
            matRes = m_pImgProcess->setColorTemperature(m_matRes, nA);
            break;
        case 34: // 激光线
            listName << "A"
                << "B"
                << "C";
            matRes = m_pImgProcess->drawLaserLine(m_matRes, nA, nB, nC);
            break;
        case 35: // 边缘加强
            listName << "A"
                << "B"
                << "C * 0.1";
            matRes = m_pImgProcess->setImgEdgeStrong(m_matRes, nA, nB, nC * 0.1);
            break;
        case 36: // 提取
            listName << "A";
            matRes = m_pImgProcess->setImgMask(m_matRes, nA);
            break;
        case 37: // 叠加
            matRes = m_pImgProcess->setImgPngMerge(m_matRes, "test.png");
            break;
        case 38: // png剪切
            matRes = m_pImgProcess->setImgCut(m_matRes);
            break;
        case 39: // 透明
            listName << "A"
                << "B"
                << "C";
            matRes = m_pImgProcess->imgTransparent(m_matRes, nA, nB, nC);
            break;
        case 40: // 添加文字
        {
            std::string strText = "睿达科技 中文123strText";

            // std::string strFont = "C:/Windows/Fonts/CENTURY.TTF";
            // std::string strFont = "/home/x/.local/share/fonts/GenWanMin.ttf";
            std::string strFont = "/usr/share/fonts/truetype/freefont/FreeSans.ttf";

            listName << "文字大小"
                << "反转";
            bool bReverse = false;
            if (nB % 2 == 0)
                bReverse = true;
            matRes = m_pImgProcess->setImgText(m_matRes, strText, strFont, nA, bReverse);
        }
        break;
        case 41: // Gamma
        {
            listName << "Gamma";
            listName << "A * 0.01";
            matRes = m_pImgProcess->setImgGamma(m_matRes, nA * 0.01);
        }
        break;

        case 42: // 测试边缘挂网
        {
            double dLineInterval = nA;
            double dDPI = nB * 0.05;
            int nContrast = nC; // 对比度
            int nBrightness = nD;
            double dGamma = nE * 0.01;
            int nEnhanceRadius = nF;
            int nEnhanceAmount = nG;
            double dAngle = nH * 0.1;

            listName << QString::fromLocal8Bit("类型")
                << QString::fromLocal8Bit("DPI * 0.05")
                << QString::fromLocal8Bit("dui bi du")
                << QString::fromLocal8Bit("亮度")
                << QString::fromLocal8Bit("Gamma * 0.01")
                << QString::fromLocal8Bit("半径")
                << QString::fromLocal8Bit("数量")
                << QString::fromLocal8Bit("角度 * 0.1");
            matRes = m_pImgProcess->setImgEdgePaperEffect(m_matRes, dLineInterval, dDPI, nContrast, nBrightness, dGamma, nEnhanceRadius, nEnhanceAmount, dAngle);
        }
        break;
        }
    }

    setSliderTip(listName);

    m_matResPrev = matRes;
    ////////////////////////////////////////////////////

    QImage qImg;

    QImage::Format f = QImage::Format_BGR888;
    if (matRes.channels() < 3)
        f = QImage::Format_Grayscale8;

    qImg = QImage((const unsigned char*)(matRes.data),
        matRes.cols, matRes.rows, matRes.cols * matRes.channels(),
        f);

    ui->labelImg->clear();
    ui->labelImg->setPixmap(QPixmap::fromImage(qImg));
    ui->labelImg->resize(ui->labelImg->pixmap(Qt::ReturnByValue).size());

    QString strValue;
    if (pushBtn)
        strValue = pushBtn->text();

    QString strTime = " " + QDateTime::currentDateTime().toString("hh:mm:ss");

    strValue += strTime;

    QTime stopTime = QTime::currentTime();
    int elapsed = startTime.msecsTo(stopTime);

    QString strTip;
    strTip = QString("%1 %2 %3 %4").arg("End...", strValue, " Elapsed: ", QString::number(elapsed));
    ui->labelProcessTip->setText(strTip);
}