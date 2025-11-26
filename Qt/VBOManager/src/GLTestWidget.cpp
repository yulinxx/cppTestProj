#include "GLTestWidget.h"
#include "Color.h"
#include "PolylinesVboManager.h"
#include "FakeData/FakeDataProvider.h"

#include <QRandomGenerator>
#include <QDebug>
#include <QDateTime>
#include <random>

using namespace GLRhi;

static const char *vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main()
{
    //gl_Position = uMVP * vec4(aPos, 1.0);
    gl_Position = vec4(aPos, 1.0);
}
)";

static const char *fragmentShaderSrc = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main()
{
    FragColor = uColor;
}
)";

GLTestWidget::GLTestWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    // connect(&m_timer, &QTimer::timeout, this, [this]() { update(); });
    // m_timer.start(16);   // ~60fps

    m_dataProvider = new FakeDataProvider();
}

GLTestWidget::~GLTestWidget()
{
    makeCurrent();
    delete m_program;
    delete m_linesMgr;
    doneCurrent();

    delete m_dataProvider;
}

void GLTestWidget::initializeGL()
{
    initializeOpenGLFunctions(); // QOpenGLFunctions
    glClearColor(0.07f, 0.07f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glLineWidth(2.0f); // 线宽

    createShader();

    // 必须在有有效 OpenGL context 之后创建！
    m_linesMgr = new GLRhi::PolylinesVboManager();

    genFakeData();

    // 开启后台自动碎片整理（可选）
    m_linesMgr->startBackgroundDefrag();
}

void GLTestWidget::createShader()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSrc);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSrc);
    m_program->link();

    if (!m_program->isLinked())
        qCritical() << "Shader link failed:" << m_program->log();
}

void GLTestWidget::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, float(w) / float(h ? h : 1), 0.1f, 1000.0f);

    m_view.setToIdentity();
    m_view.lookAt(QVector3D(0, -200, 300),
                  QVector3D(0, 0, 0),
                  QVector3D(0, 1, 0));

    m_model.setToIdentity();
}

void GLTestWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 关键修复：在这里重建！此时 context 100% 可用！

    if (!m_program || !m_linesMgr)
        return;

    m_program->bind();

    // QMatrix4x4 mvp = m_proj * m_view * m_model;
    // m_program->setUniformValue("uMVP", mvp);

    // 关键：让 PolylinesVboManager 使用当前激活的着色器程序
    // 它会自动读取 uColor 并设置颜色
    m_linesMgr->renderVisiblePrimitives();

    m_program->release();

    // 每隔 60 帧随机更新一些线（演示动态能力）
    // if (++m_frame % 60 == 0)
    //    updateSomeLines();
}

// ============================== 测试数据 ==============================

void GLTestWidget::genFakeData()
{
    std::random_device rd;
    std::mt19937 rng(rd());

    // 线段数据
    if (1)
    {
        // 生成随机数量的线段组，每组包含N条线段
        std::uniform_int_distribution<size_t> group(1, 3);
        std::uniform_int_distribution<size_t> lines(1, 30);
        std::uniform_int_distribution<size_t> maxPts(2, 5);

        std::vector<PolylineData> polylineData = m_dataProvider->genLineData(group(rng), lines(rng), 2, maxPts(rng));
        // polylineData = dataProvider.genLineData(1, 3);

        qDebug() << "\ngenLineData Groups:" << polylineData.size();

        for (const auto &pls : polylineData)
        {
            qDebug() << "pl count:" << pls.vId.size();

            QString idsStr = "pl ids / counts:";
            //for (auto pl : pls.vId)
            for (size_t d = 0; d < pls.vId.size(); d++)
            {
                idsStr += " " + QString::number(pls.vId[d]) + " / " + QString::number(pls.vCount[d]) + "  ";
            }
            qDebug() << qPrintable(idsStr);

            break;
            size_t nBase = 0;
            for (size_t i = 0; i < pls.vCount.size(); ++i)
            {
                qDebug() << "\n";
                for (size_t j = 0; j < pls.vCount[i]; ++j)
                {
                    qDebug() << "pl xyz: " << j << " " << pls.vVerts[nBase + j * 3]
                             << "," << pls.vVerts[nBase + j * 3 + 1];
                }

                nBase += (pls.vCount[i] * 3);
            }
        }

        m_linesMgr->addPolylines(polylineData);
    }
}

void GLTestWidget::updateSomeLines()
{
}

void GLTestWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_F1:
        qDebug() << "F1 键被按下 - 重置视图";
        m_view.setToIdentity();
        m_view.lookAt(QVector3D(0, -200, 300),
                      QVector3D(0, 0, 0),
                      QVector3D(0, 1, 0));
        m_model.setToIdentity();
        update();
        break;

    case Qt::Key_F2:
    {
        makeCurrent();

        if (m_linesMgr)
        {
            m_linesMgr->stopBackgroundDefrag();
            delete m_linesMgr;
            m_linesMgr = nullptr;
        }

        {
            m_linesMgr = new GLRhi::PolylinesVboManager();
            genFakeData();
            // m_linesMgr->startBackgroundDefrag();

            qDebug() << "PolylinesVboManager rebuilt in paintGL!";
        }
        update();

        // 2. 必须 makeCurrent！否则新对象拿不到 OpenGL context
        // makeCurrent();

        //// 3. 重新创建
        // m_linesMgr = new GLRhi::PolylinesVboManager();
        // genFakeData();

        // 4. 重新开启后台整理（可选）
        // m_linesMgr->startBackgroundDefrag();

        // doneCurrent();
        // update();
        break;
    }
    break;

    case Qt::Key_F3:
        qDebug() << "F3 键被按下 - 旋转视图";
        m_model.rotate(10, 0, 1, 0);
        update();
        break;

    case Qt::Key_F4:
        qDebug() << "F4 键被按下 - 缩放视图";
        m_model.scale(1.1f);
        update();
        break;

    case Qt::Key_F5:
        qDebug() << "F5 键被按下 - 缩小视图";
        m_model.scale(0.9f);
        update();
        break;

    case Qt::Key_F6:
        qDebug() << "F6 键被按下 - 更新部分线条";
        updateSomeLines();
        update();
        break;

    default:
        // 调用父类的事件处理函数
        QOpenGLWidget::keyPressEvent(event);
        break;
    }
}
