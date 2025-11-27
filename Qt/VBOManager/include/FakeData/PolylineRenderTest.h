#ifndef POLYLINE_RENDER_TEST_H
#define POLYLINE_RENDER_TEST_H

#include "FakeData/FakeDataProvider.h"

#include "PolylinesVboManager.h"

#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QTimer>
#include <QThread>
#include <QOpenGLShaderProgram>

#include <iostream>

namespace GLRhi
{
    // 线段着色器代码
    inline const char* plVS = R"(
    #version 330 core

    layout(location = 0) in vec3 aPos;

    // uniform mat4 uMVP; // 模型视图投影矩阵

    void main()
    {
        // gl_Position = uMVP * vec4(aPos, 1.0);
        gl_Position = vec4(aPos, 1.0);
    }
    )";

    inline const char* plFS = R"(
    #version 330 core
    out vec4 fragColor;

    uniform vec4 uColor; // 线段颜色

    void main()
    {
        fragColor = uColor;
    }
    )";

    // 简单的渲染窗口类
    class SimpleRenderWindow : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
    {
    public:
        SimpleRenderWindow(PolylinesVboManager* vboManager, QWidget* parent = nullptr)
            : QOpenGLWidget(parent), m_vboManager(vboManager), m_program(nullptr)
        {
            QSurfaceFormat format;
            format.setVersion(3, 3);
            format.setProfile(QSurfaceFormat::CoreProfile);
            setFormat(format);
        }

        ~SimpleRenderWindow()
        {
            cleanup();
        }

    protected:
        void initializeGL() override
        {
            initializeOpenGLFunctions();
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_DEPTH_TEST);

            // 初始化着色器
            m_program = new QOpenGLShaderProgram;
            if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, plVS) ||
                !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, plFS) ||
                !m_program->link())
            {
                std::cerr << "着色器链接失败: " << m_program->log().toStdString() << std::endl;
                delete m_program;
                m_program = nullptr;
                return;
            }

            m_program->bind();
            m_uMVPLoc = m_program->uniformLocation("uMVP");
            m_uColorLoc = m_program->uniformLocation("uColor");
            m_program->release();
        }

        void paintGL() override
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (m_vboManager && m_program)
            {
                m_program->bind();
                
                // 设置默认的MVP矩阵（单位矩阵）
                //float mvp[16] = {
                //    1.0f, 0.0f, 0.0f, 0.0f,
                //    0.0f, 1.0f, 0.0f, 0.0f,
                //    0.0f, 0.0f, 1.0f, 0.0f,
                //    0.0f, 0.0f, 0.0f, 1.0f
                //};
                //m_program->setUniformValue(m_uMVPLoc, QMatrix4x4(mvp));
                
                // 渲染
                m_vboManager->renderVisiblePrimitives();
                
                m_program->release();
            }
        }

        void cleanup()
        {
            if (m_program)
            {
                delete m_program;
                m_program = nullptr;
            }
        }

    private:
        PolylinesVboManager* m_vboManager;
        QOpenGLShaderProgram* m_program;
        int m_uMVPLoc;  // MVP矩阵的uniform位置
        int m_uColorLoc; // 颜色的uniform位置
    };

    // 检查是否已有QApplication实例
    bool hasQApplication();

    // 创建并显示渲染窗口的函数
    void showPolylineRenderWindow(PolylinesVboManager* vboManager);
}

// 主函数示例（如果直接运行这个文件）
#if 0
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // 创建FakeDataProvider并生成数据
    GLRhi::FakeDataProvider provider;
    auto lineData = provider.genLineData(1, 10);

    // 初始化PolylinesVboManager
    GLRhi::PolylinesVboManager vboManager;
    vboManager.addPolylines(lineData);

    // 显示渲染窗口
    GLRhi::showPolylineRenderWindow(&vboManager);

    return 0;
}
#endif


#endif // POLYLINE_RENDER_TEST_H