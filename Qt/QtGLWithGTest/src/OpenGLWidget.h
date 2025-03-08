#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_0_Core
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget* parent = nullptr);
    ~OpenGLWidget() override;
    static QVector3D bezierCurve(const QVector3D& p0, const QVector3D& p1,
        const QVector3D& p2, const QVector3D& p3, float t);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    float OpenGLWidget::randomFloat(float min, float max);

private:
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
};

#endif