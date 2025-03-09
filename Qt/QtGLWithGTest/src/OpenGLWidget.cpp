#include "OpenGLWidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QDebug>
#include <random>

QVector3D OpenGLWidget::bezierCurve(const QVector3D& p0, const QVector3D& p1,
    const QVector3D& p2, const QVector3D& p3, float t)
{
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    QVector3D p = uuu * p0;
    p += 3 * uu * t * p1;
    p += 3 * u * tt * p2;
    p += ttt * p3;

    return p;
}

OpenGLWidget::OpenGLWidget(QWidget* parent) : QOpenGLWidget(parent)
{}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent();
    vao.destroy();
    vbo.destroy();
    doneCurrent();
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    QOpenGLShaderProgram shaderProgram;

    // glsl 需放至 EXE 同级目录中
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "vertex.glsl");
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "fragment.glsl");
    shaderProgram.link();
    shaderProgram.bind();

    QVector3D p0(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0.0f);
    QVector3D p1(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0.0f);
    QVector3D p2(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0.0f);
    QVector3D p3(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0.0f);

    const int numPoints = 100;
    QVector<QVector3D> points;
    for (int i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float>(i) / (numPoints - 1);
        points.append(bezierCurve(p0, p1, p2, p3, t));
    }

    vao.create();
    vao.bind();

    vbo.create();
    vbo.bind();
    vbo.allocate(points.data(), points.size() * sizeof(QVector3D));

    shaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
    shaderProgram.enableAttributeArray(0);

    vao.release();
    vbo.release();
    shaderProgram.release();
}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void OpenGLWidget::paintGL()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    vao.bind();
    glDrawArrays(GL_LINE_STRIP, 0, 100);
    vao.release();
}

float OpenGLWidget::randomFloat(float min, float max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return static_cast<float>(dis(gen));
}