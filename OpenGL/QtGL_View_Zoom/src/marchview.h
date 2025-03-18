#ifndef MARCHVIEW_H
#define MARCHVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_0_Core>  // 修改这里
#include <QOpenGLShaderProgram>
#include <QVector2D>
#include <vector>

class MarchView : public QOpenGLWidget, protected QOpenGLFunctions_4_0_Core  // 修改这里
{
    Q_OBJECT
public:
    explicit MarchView(QWidget *parent = nullptr);
    ~MarchView();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

protected:
    void updateLineBuffer();
    void updateRulerBuffer();

private:
    QOpenGLShaderProgram *m_lineProgram = nullptr;
    GLuint m_lineVao;
    GLuint m_lineVbo;
    
    float m_scale = 1.0f;
    QVector2D m_translation;
    QPoint m_lastPos;
    
    struct Point {
        float x, y;
    };
    std::vector<Point> m_points;
    

private:
    std::vector<Point> m_crossPoints; // 标尺线条顶点
    GLuint m_rulerVao, m_rulerVbo;    // 标尺的 VAO 和 VBO
    QOpenGLShaderProgram* m_rulerProgram; // 标尺的着色器程序
};

#endif // MARCHVIEW_H