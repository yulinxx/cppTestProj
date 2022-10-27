/*
 * @Author: lyl_Linux april@ubuntu.com
 * @Date: 2022-05-13 16:53:56
 * @LastEditors: lyl_Linux april@ubuntu.com
 * @LastEditTime: 2022-05-16 12:02:46
 * @FilePath: /rdImgProcess/test/mainwindow.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


#include <opencv2/opencv.hpp>
using namespace cv;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace ImgSpace {
    class imgProcess;
}

class QLabel;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
protected:
    void initSlider();
    void initData();

    void setSliderTip(QStringList listTip);

protected slots:
    // void slotImageParamSet(bool bClicked);
    void slotImageFit();
    void slotOpenImage();
    void slotImageSave();

    void slotValueReset();
    void slotZeroSlide();

    void slotSlideValue();
    void slotImageResize();
    void slotImageSet();

private:
    ImgSpace::imgProcess* m_pImgProcess = nullptr;
    Mat m_matRes;
    Mat m_matResPrev;

    Ui::MainWindow *ui = nullptr;

    int m_nType = -1;
    int m_nTypePrev = -1;

    QVector<QLabel*> m_vecLabelName;
};
#endif // MAINWINDOW_H

