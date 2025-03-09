#include "SkiaCircleWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"
#include "include/core/SkEncodedImageFormat.h"

#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkTypes.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"

SkiaCircleWidget::SkiaCircleWidget(QWidget* parent) : QWidget(parent)
{
    // 构造函数的实现
    m_surface = SkSurface::MakeRasterN32Premul(width(), height());
}

void SkiaCircleWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // Get Skia canvas from the surface
    SkCanvas* canvas = m_surface->getCanvas();

    // Clear the canvas
    canvas->clear(SK_ColorBLUE);

    // Create Skia paint for the circle
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setAntiAlias(true);

    // Calculate circle position and radius
    SkPoint center = SkPoint::Make(width() / 2.0f, height() / 2.0f);
    float radius = qMin(width(), height()) / 4.0f;

    // Draw the circle on the canvas
    canvas->drawCircle(center.x(), center.y(), radius, paint);

    // Convert Skia image to QImage
    sk_sp<SkImage> image = m_surface->makeImageSnapshot();
    sk_sp<SkData> data = image->encodeToData(SkEncodedImageFormat::kPNG, 100);
    if (data)
    {
        QImage qImage(reinterpret_cast<const uchar*>(data->data()), image->width(), image->height(), QImage::Format_ARGB32);
        QPainter painter(this);
        painter.drawImage(0, 0, qImage);
    }
}