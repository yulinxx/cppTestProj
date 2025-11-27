#include "GLTestWidget.h"
#include "Color.h"
#include "PolylinesVboManager.h"
#include "FakeData/FakeDataProvider.h"
#include "FakeData/FakePolyLineData.h"

#include <QRandomGenerator>
#include <QDebug>
#include <QDateTime>
#include <random>

using namespace GLRhi;

static const char* vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main()
{
    //gl_Position = uMVP * vec4(aPos, 1.0);
    gl_Position = vec4(aPos, 1.0);
}
)";

static const char* fragmentShaderSrc = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main()
{
    FragColor = uColor;
}
)";

GLTestWidget::GLTestWidget(QWidget* parent)
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

void GLTestWidget::updateSomeLines()
{
}

void GLTestWidget::keyPressEvent(QKeyEvent* event)
{

    switch (event->key())
    {
    case Qt::Key_F1:
    {
        qDebug() << "F1:重建所有线条数据，F2：添加新数据，F3：删除部分数据，F4：修改部分数据，F5：清除所有数据，F6：更新部分线条\n";

        if (m_linesMgr)
        {
            makeCurrent();
            qDebug() << "F1 - clearAllPrimitives / genFakeData";
            m_linesMgr->clearAllPrimitives();
            genFakeData();
            qDebug() << "PolylinesVboManager rebuilt in paintGL!";
            update();
        }
    }
    break;

    case Qt::Key_F2:
    {
        makeCurrent();
        qDebug() << "\nF2 - addNewFakeData";
        addNewFakeData();
        update();
    }
    break;

    case Qt::Key_F3:
    {
        if (m_linesMgr)
        {
            makeCurrent();
            qDebug() << "\nF3 - delFakeData";
            delFakeData();
            update();
        }
    }
    break;

    case Qt::Key_F4:
    {
        makeCurrent();
        qDebug() << "\nF4 - modifyFakeData";
        modifyFakeData();
        update();
    }
    break;

    case Qt::Key_F5:
    {
        makeCurrent();
        qDebug() << "\nF5 - clearAllPrimitives";
        m_linesMgr->clearAllPrimitives();
        update();
    }
    break;

    case Qt::Key_F6:
    {
        qDebug() << "\nF6 - 更新部分线条";
        updateSomeLines();
        update();
    }
    break;

    default:
        QOpenGLWidget::keyPressEvent(event);
        break;
    }
}

// ============================== 测试数据 ==============================

void GLTestWidget::genFakeData()
{
    std::random_device rd;
    std::mt19937 rng(rd());

    // 生成线段数据
    size_t nGroups = 0;
    size_t nLines = 0;
    long long nStartID = 1;

    if (1)
    {
        // 生成随机数量的线段组，每组包含N条线段
        std::uniform_int_distribution<size_t> group(1, 5);
        std::uniform_int_distribution<size_t> lines(1, 30);
        std::uniform_int_distribution<size_t> maxPts(2, 100);

        m_polylineData = m_dataProvider->genLineData(group(rng), lines(rng), 2, maxPts(rng));

        nGroups = m_polylineData.size();
        qDebug() << "\ngenLineData Groups:" << nGroups;

        size_t nPtSz = 0;
        for (size_t g = 0; g < m_polylineData.size(); g++)
        {

            auto& pls = m_polylineData[g];
            if (g == 0)
                nStartID = pls.vId[g];

            nLines += pls.vId.size();

            for (size_t d = 0; d < pls.vCount.size(); d++)
            {
                nPtSz += pls.vCount[d];
            }
        }
        qDebug() << "Start ID: " << nStartID << " Line Sz: " << nLines << "\tPoint Sz: " << nPtSz;

        if (0)
        {
            for (const auto& pls : m_polylineData)
            {
                qDebug() << "pl count:" << pls.vId.size();

                QString idsStr = "pl ids / counts:";
                // for (auto pl : pls.vId)
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
        }
        m_linesMgr->addPolylines(m_polylineData);
    }

    // 3. 更新线段数据
}

void GLTestWidget::addNewFakeData()
{
    std::random_device rd;
    std::mt19937 rng(rd());

    // 生成随机数量的线段组，每组包含N条线段
    //std::uniform_int_distribution<size_t> group(1, 5);
    //std::uniform_int_distribution<size_t> lines(1, 30);
    std::uniform_int_distribution<size_t> group(1, 1);
    std::uniform_int_distribution<size_t> lines(1, 1);
    std::uniform_int_distribution<size_t> maxPts(2, 10);

    auto polylineData = m_dataProvider->genLineData(1, 1, 2, maxPts(rng));
    for (auto& pls : polylineData)
    {
        Color c = pls.brush.getColor();
        for (auto& id : pls.vId)
        {
            std::vector<float>& vVertices = pls.vVerts;

            m_linesMgr->addPolyline(id, vVertices, c);
        }
    }
}

// 随机删除N%的图元
void GLTestWidget::delFakeData()
{
    if (1)
    {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_real_distribution<double> ratio(0.0, 0.3);
        double dDelRatio = ratio(rng);
        //qDebug() << "删除比例: " << dDelRatio * 100 << "%";

        std::vector<long long> vDelIDs;

        // 遍历每个线段组
        for (size_t i = 0; i < m_polylineData.size(); ++i)
        {
            auto& plData = m_polylineData[i];
            size_t nLineCount = plData.vCount.size();

            if (nLineCount == 0)
                continue;

            for (size_t j = 0; j < plData.vId.size(); ++j)
            {
                std::uniform_real_distribution<double> prob_dist(0.0, 0.9);
                if (prob_dist(rng) < dDelRatio)
                    vDelIDs.push_back(plData.vId[j]);
            }
        }

        // 删除
        std::vector<long long> vDeledIds;
        for (auto id : vDelIDs)
        {
            if (m_linesMgr->removePolyline(id))
                vDeledIds.push_back(id);
        }

        if (!vDeledIds.empty())
            qDebug() << "删除的图元数: " << vDelIDs.size() << "IDs: " << vDelIDs;

    }
}

void GLTestWidget::modifyFakeData()
{
    if (1)
    {
        // 只修改軟件自己生成的线段数据
        // 因为F2新增的数据未加入到m_polylineData中，所以不处理

        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_real_distribution<double> ratio(0.0, 0.3);
        std::uniform_real_distribution<double> prob_dist(0.0, 0.9);

        long long id;
        std::vector<float> vertices;

        // 遍历每个线段组
        for (size_t i = 0; i < m_polylineData.size(); ++i)
        {
            auto& plData = m_polylineData[i];
            size_t nLineCount = plData.vCount.size();
            if (nLineCount == 0)
                continue;

            size_t nBase = 0;

            // 遍历线段
            for (size_t j = 0; j < plData.vId.size(); ++j)
            {
                id = plData.vId[j];
                size_t nPtCount = plData.vCount[j];

                // 调整为30%左右的概率进入修改分支
                double randomValue = prob_dist(rng);
                if (randomValue < 0.3) // 30%的概率
                {
                    FakePolyLineData lineGen;
                    lineGen.generateSingleLine(static_cast<int>(nPtCount));
                    const std::vector<float> vNewPts = lineGen.getVertices();

                    if (m_linesMgr->updatePolyline(id, vNewPts))
                        qDebug() << "修改的ID : " << id << " 点数: " << nPtCount;

                }

                nBase += (nPtCount * 3);
            }

            // 更新
        }
    }
}
