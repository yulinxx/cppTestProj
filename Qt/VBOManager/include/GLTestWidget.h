#ifndef GLTEST_WIDGET_H
#define GLTEST_WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QKeyEvent>

#include <vector>
#include "RenderCommon.h"

namespace GLRhi
{
    class PolylinesVboManager;
    class FakeDataProvider;
}

class GLTestWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit GLTestWidget(QWidget* parent = nullptr);
    ~GLTestWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void createShader();
    void updateSomeLines();     // 每帧随机改几条线（演示动态更新）

    void genFakeData();         // 添加几万条测试线
    void addNewFakeData();      // 添加测试线数据
    void delFakeData();         // 删除测试
    void modifyFakeData();      // 修改测试线数据


    QOpenGLShaderProgram* m_program{ nullptr };
    GLRhi::PolylinesVboManager* m_linesMgr{ nullptr };

    GLRhi::FakeDataProvider* m_dataProvider{ nullptr };
    std::vector<GLRhi::PolylineData> m_polylineData;

    QMatrix4x4  m_proj;
    QMatrix4x4  m_view;
    QMatrix4x4  m_model;

    bool m_bUseDrawEx{ true };  // 是否使用高性能绘制
    QTimer      m_timer;
    int         m_frame{ 0 };
};

#endif // GLTEST_WIDGET_H
