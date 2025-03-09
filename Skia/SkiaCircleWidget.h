#ifndef SKIACIRCLEWIDGET_H
#define SKIACIRCLEWIDGET_H

#include <QWidget>

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImage.h"

class SkiaCircleWidget : public QWidget
{
    Q_OBJECT

public:
    SkiaCircleWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    sk_sp<SkSurface> m_surface;
};

#endif // SKIACIRCLEWIDGET_H
