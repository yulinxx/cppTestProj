#include "marchview.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>

MarchView::MarchView(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMouseTracking(true);

    // 添加OpenGL版本检查
    QSurfaceFormat format;
    format.setVersion(4, 0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);

    // 初始化标尺线条（示例：水平和垂直线）
    cross = {
        {-0.9f, 0.0f}, {0.9f, 0.0f}, // 水平线
        {0.0f, -0.9f},
        {0.0f, 0.9f} // 垂直线
    };


}

MarchView::~MarchView()
{
    makeCurrent();

    glDeleteVertexArrays(1, &m_lineVao);
    glDeleteBuffers(1, &m_lineVbo);
    delete m_lineProgram;

    glDeleteVertexArrays(1, &m_rulerVao);
    glDeleteBuffers(1, &m_rulerVbo);
    delete m_rulerProgram;

    doneCurrent();
}

void MarchView::initializeGL()
{
    if (!initializeOpenGLFunctions())
    {
        qFatal("Could not initialize OpenGL 4.0 functions");
    }

    // Vertex Shader 普通线条的着色器和缓冲区
    const char *vertexShaderSource = R"(
        #version 400
        layout(location = 0) in vec2 position;
        uniform mat4 projection;
        uniform vec2 translation;
        uniform float scale;
        
        void main()
        {
            vec2 scaledPos = position * scale + translation;
            gl_Position = projection * vec4(scaledPos, 0.0, 1.0);
        }
    )";

    // Fragment Shader
    const char *fragmentShaderSource = R"(
        #version 400
        out vec4 fragColor;
        
        void main()
        {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
        }
    )";

    m_lineProgram = new QOpenGLShaderProgram;
    m_lineProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_lineProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_lineProgram->link();

    glGenVertexArrays(1, &m_lineVao);
    glGenBuffers(1, &m_lineVbo);

    // 初始化VAO绑定
    glBindVertexArray(m_lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 标尺线条的着色器和缓冲区
    const char *rulerVertexShaderSource = R"(
        #version 400
        layout(location = 0) in vec2 position;
        void main()
        {
            gl_Position = vec4(position, 0.0, 1.0); // 直接使用屏幕坐标
        }
    )";

    const char *rulerFragmentShaderSource = R"(
        #version 400
        out vec4 fragColor;
        void main()
        {
            fragColor = vec4(0.0, 0.0, 1.0, 1.0); // 标尺线条为蓝色
        }
    )";

    m_rulerProgram = new QOpenGLShaderProgram;
    m_rulerProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, rulerVertexShaderSource);
    m_rulerProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, rulerFragmentShaderSource);
    m_rulerProgram->link();

    glGenVertexArrays(1, &m_rulerVao);
    glGenBuffers(1, &m_rulerVbo);
    glBindVertexArray(m_rulerVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_rulerVbo);
    glBufferData(GL_ARRAY_BUFFER, cross.size() * sizeof(Point),
                 cross.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MarchView::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void MarchView::paintGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    updateRulerBuffer();

    qDebug() << "Drawing" << m_points.size() << "points";
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        qDebug() << "OpenGL error before draw:" << err;

    // 绘制普通线条
    if (m_points.size() >= 2 && m_lineProgram->bind())
    {

        if (!m_lineProgram->bind())
        {
            qDebug() << "Shader program bind failed";
            return;
        }

        // Set up orthographic projection
        float aspect = float(width()) / height();
        float orthoSize = 1000.0f; // 基础范围 -1000 到 1000
        QMatrix4x4 projection;
        projection.ortho(-orthoSize * aspect, orthoSize * aspect,
                         -orthoSize, orthoSize,
                         -1.0f, 1.0f);

        m_lineProgram->setUniformValue("projection", projection);
        m_lineProgram->setUniformValue("translation", m_translation);
        m_lineProgram->setUniformValue("scale", m_scale);

        glBindVertexArray(m_lineVao);
        glDrawArrays(GL_LINES, 0, m_points.size());
        glBindVertexArray(0);

        m_lineProgram->release();
    }

    // 绘制标尺线条
    if (cross.size() >= 2 && m_rulerProgram->bind())
    {
        glBindVertexArray(m_rulerVao);
        glDrawArrays(GL_LINES, 0, cross.size());
        glBindVertexArray(0);

        m_rulerProgram->release();
    }
}

void MarchView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        float x = (float(event->x()) / width() * 2.0f - 1.0f) * 1000.0f / m_scale - m_translation.x();
        float y = -(float(event->y()) / height() * 2.0f - 1.0f) * 1000.0f / m_scale - m_translation.y();

        // if (m_points.size() == 2)
        //     m_points.clear();

        m_points.push_back({x, y});

        updateLineBuffer();
        update();
    }
    else if (event->button() == Qt::MiddleButton)
    {
        m_lastPos = event->pos();
    }
}

void MarchView::wheelEvent(QWheelEvent *event)
{
    // float delta = event->angleDelta().y() > 0 ? 1.1f : 0.9f;
    // m_scale *= delta;
    // update();

    // 滚轮缩放
    float delta = event->angleDelta().y() > 0 ? 1.1f : 0.9f;

    // 在鼠标位置处进行缩放
    QPointF pos = event->position();
    float mouseX = (pos.x() / width() * 2.0f - 1.0f) * 1000.0f * (float(width()) / height()) / m_scale - m_translation.x();
    float mouseY = -(pos.y() / height() * 2.0f - 1.0f) * 1000.0f / m_scale - m_translation.y();

    m_scale *= delta;

    // 调整平移以保持鼠标位置不变
    m_translation.setX(m_translation.x() - mouseX * (delta - 1.0f) * m_scale);
    m_translation.setY(m_translation.y() - mouseY * (delta - 1.0f) * m_scale);

    update();
}

void MarchView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::MiddleButton)
    {
        QPoint delta = event->pos() - m_lastPos;
        float aspect = float(width()) / height();

        // 固定世界坐标中的移动系数，不直接依赖 1/m_scale
        float moveSpeed = 2.0f; // 可调整的移动速度
        float dx = delta.x() * moveSpeed * aspect / width();
        float dy = -delta.y() * moveSpeed / height();

        m_translation += QVector2D(dx, dy) * 1000.0f; // 乘以基础范围
        m_lastPos = event->pos();
        update();
    }
}

void MarchView::updateLineBuffer()
{
    qDebug() << "Uploading" << m_points.size() << "points to GPU";

    glBindVertexArray(m_lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);

    // 检查缓冲区绑定状态
    GLint boundBuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);
    qDebug() << "VBO bound:" << (boundBuffer == (GLint)m_lineVbo);

    glBufferData(GL_ARRAY_BUFFER, m_points.size() * sizeof(Point),
                 m_points.data(), GL_STATIC_DRAW);

    // 检查缓冲区数据大小
    GLint bufferSize = 0;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    qDebug() << "VBO size:" << bufferSize << "bytes";

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glEnableVertexAttribArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        qDebug() << "OpenGL error in updateLineBuffer:" << err;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MarchView::updateRulerBuffer()
{
    glBindVertexArray(m_rulerVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_rulerVbo);
    glBufferData(GL_ARRAY_BUFFER, cross.size() * sizeof(Point),
                 cross.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}