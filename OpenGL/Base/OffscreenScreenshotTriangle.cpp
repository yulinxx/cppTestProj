#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFramebufferObject>
#include <QImage>
#include <QDebug>
#include <QFile>
#include <QOpenGLShaderProgram> 

// --- OpenGL 着色器源代码 ---
// 顶点着色器
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";

// 片段着色器
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); // 橙红色
}
)";

/**
 * @brief 在给定的 OpenGL 上下文中执行一个简单的三角形绘制。
 * @param f 当前的 OpenGL 函数指针 (3.3 Core)
 */
void drawTriangle(QOpenGLFunctions_3_3_Core* f)
{
    // 1. 编译并链接着色器
    QOpenGLShaderProgram shaderProgram;
    if (!shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource))
    {
        qCritical() << "Vertex shader compile error:" << shaderProgram.log();
        return;
    }
    if (!shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource))
    {
        qCritical() << "Fragment shader compile error:" << shaderProgram.log();
        return;
    }
    if (!shaderProgram.link())
    {
        qCritical() << "Shader program link error:" << shaderProgram.log();
        return;
    }

    // 2. 准备顶点数据 (一个简单的三角形)
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // 左下角
         0.5f, -0.5f, 0.0f, // 右下角
         0.0f,  0.5f, 0.0f  // 顶部
    };

    GLuint VBO, VAO;

    // 生成 VAO
    f->glGenVertexArrays(1, &VAO);
    f->glBindVertexArray(VAO);

    // 生成 VBO
    f->glGenBuffers(1, &VBO);
    f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);

    // 解绑 VBO 和 VAO (注意顺序：先 VBO，再 VAO)
    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
    f->glBindVertexArray(0);

    // 3. 使用着色器程序并绘制
    shaderProgram.bind(); // 绑定着色器程序
    f->glBindVertexArray(VAO); // 绑定 VAO
    f->glDrawArrays(GL_TRIANGLES, 0, 3); // 绘制三角形
    f->glBindVertexArray(0); // 解绑 VAO
    shaderProgram.release(); // 释放着色器程序

    // 清理 OpenGL 资源
    f->glDeleteVertexArrays(1, &VAO);
    f->glDeleteBuffers(1, &VBO);
}


/**
 * @brief 执行 OpenGL 离屏渲染并将结果保存为文件
 * * @param outputFileName 要保存的图像文件名 (例如: "render_output.png")
 * @param width 渲染宽度
 * @param height 渲染高度
 * @return true 渲染成功并保存
 * @return false 渲染失败
 */
bool renderOffScreen(const QString& outputFileName, int width, int height)
{
    // --- 1. 设置 OpenGL 上下文和表面 ---
    // QGuiApplication 实例是必需的，即使是离屏渲染
    if (!QGuiApplication::instance())
    {
        qCritical() << "QGuiApplication instance is missing. Please create one before calling this function.";
        return false;
    }

    // 设置所需的 OpenGL 版本
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);

    // 禁用双缓冲，FBO不需要
    format.setSwapBehavior(QSurfaceFormat::SingleBuffer);

    // 创建离屏表面
    QOffscreenSurface surface;
    surface.setFormat(format);
    surface.create();

    if (!surface.isValid())
    {
        qCritical() << "Failed to create QOffscreenSurface.";
        return false;
    }

    // 创建 OpenGL 上下文
    QOpenGLContext context;
    context.setFormat(format);
    if (!context.create())
    {
        qCritical() << "Failed to create QOpenGLContext.";
        return false;
    }

    // 将上下文设置为当前
    if (!context.makeCurrent(&surface))
    {
        qCritical() << "Failed to make QOpenGLContext current.";
        return false;
    }

    // 初始化 OpenGL 函数（必须在 makeCurrent 之后）
    // 使用 QOpenGLFunctions_3_3_Core 来支持 OpenGL 3.3 核心配置文件
    QOpenGLFunctions_3_3_Core* f = context.versionFunctions<QOpenGLFunctions_3_3_Core>();
    if (!f)
    {
        qCritical() << "Failed to get QOpenGLFunctions_3_3_Core.";
        context.doneCurrent();
        return false;
    }
    f->initializeOpenGLFunctions();

    qDebug() << "OpenGL Context Version:" << (const char*)f->glGetString(GL_VERSION);
    qDebug() << "GLSL Version:" << (const char*)f->glGetString(GL_SHADING_LANGUAGE_VERSION);

    // --- 2. 创建 FBO ---
    QSize size(width, height);
    QOpenGLFramebufferObjectFormat fboFormat;
    // 使用默认格式，通常包含一个颜色附件和一个深度/模板附件
    fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

    QOpenGLFramebufferObject fbo(size, fboFormat);
    if (!fbo.isValid())
    {
        qCritical() << "Failed to create QOpenGLFramebufferObject.";
        context.doneCurrent();
        return false;
    }

    // 绑定 FBO，使其成为当前的渲染目标
    if (!fbo.bind())
    {
        qCritical() << "Failed to bind QOpenGLFramebufferObject.";
        context.doneCurrent();
        return false;
    }

    // --- 3. 执行渲染指令 ---

    // 设置视口
    f->glViewport(0, 0, width, height);

    // 清空颜色和深度缓冲区
    f->glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 深青色背景
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ************ 调用绘制三角形的函数 ************
    drawTriangle(f);
    // ****************************************************

    // 渲染完毕后，解绑 FBO
    fbo.release();

    // --- 4. 读取 FBO 内容并保存 ---
    // 注意：readImage() 会自动重新绑定 FBO 并读取数据
    QImage image = fbo.toImage();

    // 释放上下文
    context.doneCurrent();

    // 检查图像是否有效并保存
    if (image.isNull())
    {
        qCritical() << "Failed to read image data from FBO.";
        return false;
    }

    // OpenGL 纹理的原点通常在左下角，但 Qt 的 QImage 是左上角
    // fbo.toImage() 在 Qt 5.5+ 默认会处理翻转，但有时仍需要手动翻转，以防万一。
    // 如果图像是倒置的，请使用: image = image.mirrored();

    if (!image.save(outputFileName))
    {
        qCritical() << "Failed to save image to:" << outputFileName;
        return false;
    }

    qDebug() << "Successfully rendered off-screen and saved to:" << outputFileName;
    return true;
}


// --- main 函数：只调用，不显示类 ---
int main(int argc, char* argv[])
{
    // 1. 启动 Qt GUI 应用
    QGuiApplication app(argc, argv);

    // 2. 调用离屏渲染函数
    int renderWidth = 800;
    int renderHeight = 600;
    QString filename = "offscreen_screenshot_triangle.png"; // 修改文件名以区分

    qDebug() << "Starting off-screen rendering...";
    bool success = renderOffScreen(filename, renderWidth, renderHeight);

    if (success)
    {
        qDebug() << "Render successful!";
        // 可以选择退出应用程序，因为渲染已完成
        return 0;
    }
    else
    {
        qDebug() << "Render failed!";
        // 返回非零值表示失败
        return 1;
    }

    return app.exec();
}