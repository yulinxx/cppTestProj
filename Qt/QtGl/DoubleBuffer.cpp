/*
 * DoubleBuffer.cpp
 *
 * 这是一个使用Qt和OpenGL实现的双缓冲动画渲染示例程序
 * 功能：
 * - 使用双缓冲技术实现流畅的动画效果
 * - 渲染大量随机彩色线段并实时更新它们的位置
 * - 使用OpenGL fence同步技术确保GPU渲染正确
 * - 展示了Qt OpenGL框架与现代OpenGL的结合使用
 */

 /*
 * GLsync 是 OpenGL 3.2+ 引入的同步对象（Sync Object），用于实现 CPU 与 GPU 之间的显式同步，解决多线程共享 OpenGL 资源时的竞争问题。
 它是一个不透明句柄（opaque handle），代表 GPU 命令流中的一个栅栏（fence）点。
 为什么需要 GLsync？
 OpenGL 命令是异步执行的。当你调用 glDrawArrays 后，命令只是被放入驱动缓存，并未立即在 GPU 上执行。
 在多线程环境中，若线程 B 使用线程 A 刚渲染的纹理，可能因 GPU 未完成渲染而导致异常。
 传统方案缺陷：
 glFinish()：仅阻塞当前线程，无法跨线程等待
 glFlush()：仅保证命令送入队列，不保证执行完成
 GLsync 优势：允许在一个线程中插入"栅栏"(fence)，在另一个线程中等待该栅栏信号，实现跨线程同步。
 GLsync 让你能精确知道：GPU 何时真正完成了特定命令。
 ------
 glFenceSync - 创建同步对象
 GLsync glFenceSync(GLenum condition, GLbitfield flags);
 参数：
 condition：必须是 GL_SYNC_GPU_COMMANDS_COMPLETE（等待所有前置 GPU 命令完成）
 flags：必须为 0（保留供未来扩展）
 返回值：
 同步对象句柄，若创建失败返回 NULL
 作用：
 在 OpenGL 命令流中插入栅栏，创建后状态为未触发(unsignaled)

 在 GPU 命令流中插入一个栅栏, 返回一个处于 未触发（unsignaled） 状态的同步对象,不阻塞 CPU，立即返回
 ------
 glClientWaitSync - CPU 等待
 GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
 参数：
 sync：glFenceSync 返回的同步对象
 flags：GL_SYNC_FLUSH_COMMANDS_BIT（强制刷新命令，确保 GPU 收到）
 timeout：等待超时时间（纳秒），或 GL_TIMEOUT_IGNORED

 返回值：
 GL_ALREADY_SIGNALED：调用前已触发
 GL_TIMEOUT_EXPIRED：超时未触发
 GL_CONDITION_SATISFIED：在超时时间内触发
 GL_WAIT_FAILED：发生错误

 作用：
 阻塞当前 CPU 调用线程，直到同步对象触发或超时
 在客户端（CPU）等待

 */

 // Qt核心和GUI库
#include <QApplication>        // Qt应用程序类
#include <QOpenGLWidget>       // OpenGL窗口部件基类
#include <QOpenGLFunctions_3_3_Core>  // OpenGL 3.3核心功能封装
#include <QTimer>             // 定时器，用于动画更新
#include <QDebug>             // 调试输出

// C++标准库
#include <vector>             // 动态数组，用于存储顶点数据
#include <random>             // 随机数生成，用于创建和更新线段
#include <algorithm>          // 用于std::clamp函数

// 常量定义
constexpr size_t MAX_VERTICES = 500000;   // 最大顶点数量
constexpr size_t VERTEX_STRIDE = 5;       // 每个顶点的数据步长：x, y, r, g, b（2个坐标 + 3个颜色分量）

/**
 * @class DoubleBufferWidget
 * @brief 双缓冲OpenGL渲染窗口部件
 *
 * 该类继承自QOpenGLWidget和QOpenGLFunctions_3_3_Core，实现了一个使用双缓冲技术
 * 的OpenGL渲染窗口，能够高效地渲染和更新大量线段，同时保持流畅的动画效果。
 */
class DoubleBufferWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT  // Qt信号槽机制所需的宏

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口部件
     */
    DoubleBufferWidget(QWidget* parent = nullptr) : QOpenGLWidget(parent)
    {
        // 设置更新行为：不进行部分更新，每次都重绘整个窗口
        setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    }

    /**
     * @brief 析构函数
     *
     * 清理所有OpenGL资源，包括着色器程序、VAO、VBO和同步对象
     */
    ~DoubleBufferWidget() override
    {
        // 清理OpenGL资源（使用原生OpenGL接口）
        makeCurrent(); // 确保OpenGL上下文是活动的

        // 删除着色器程序
        if (m_program)
        {
            glDeleteProgram(m_program);
        }

        // 删除VBO
        for (int i = 0; i < 2; ++i)
        {
            if (m_vbo[i])
            {
                glDeleteBuffers(1, &m_vbo[i]);
            }

            // 删除VAO
            if (m_vao[i])
            {
                glDeleteVertexArrays(1, &m_vao[i]);
            }

            // 删除同步对象
            if (m_fences[i])
            {
                glDeleteSync(m_fences[i]);
            }
        }

        doneCurrent(); // 释放OpenGL上下文
    }

protected:
    /**
     * @brief OpenGL初始化函数
     *
     * 重写QOpenGLWidget的initializeGL方法，用于初始化OpenGL上下文、创建着色器程序、
     * 设置VAO/VBO等OpenGL资源，以及准备初始数据。
     */
    void initializeGL() override
    {
        // 初始化OpenGL函数指针
        initializeOpenGLFunctions();

        // 输出OpenGL版本信息
        qDebug() << "OpenGL:" << (const char*)glGetString(GL_VERSION);
        qDebug() << "GLSL :" << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

        // 顶点着色器源代码
        const char* vs = R"(
            #version 330 core
            layout(location = 0) in vec2 aPos;     // 顶点位置输入
            layout(location = 1) in vec3 aColor;   // 顶点颜色输入
            out vec3 vColor;                      // 传递给片段着色器的颜色
            void main(){
                gl_Position = vec4(aPos, 0.0, 1.0);  // 设置顶点位置
                vColor = aColor;                     // 传递颜色
            }
        )";

        // 片段着色器源代码
        const char* fs = R"(
            #version 330 core
            in vec3 vColor;    // 从顶点着色器接收的颜色
            out vec4 fragColor; // 输出片段颜色
            void main(){
                fragColor = vec4(vColor, 1.0);  // 设置输出颜色，alpha通道为1.0
            }
        )";

        // 创建并编译顶点着色器
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vs, nullptr);
        glCompileShader(vertexShader);

        // 检查顶点着色器编译状态
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            qCritical() << "顶点着色器编译失败:" << infoLog;
            return;
        }

        // 创建并编译片段着色器
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fs, nullptr);
        glCompileShader(fragmentShader);

        // 检查片段着色器编译状态
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            qCritical() << "片段着色器编译失败:" << infoLog;
            glDeleteShader(vertexShader); // 清理已创建的着色器
            return;
        }

        // 创建着色器程序
        m_program = glCreateProgram();
        // 附加着色器
        glAttachShader(m_program, vertexShader);
        glAttachShader(m_program, fragmentShader);
        // 链接着色器程序
        glLinkProgram(m_program);

        // 检查链接状态
        glGetProgramiv(m_program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
            qCritical() << "着色器程序链接失败:" << infoLog;
            // 清理资源
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(m_program);
            m_program = 0;
            return;
        }

        // 着色器链接成功后，可以删除着色器对象（使用原生OpenGL接口）
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // 创建双缓冲的VAO和VBO（使用原生OpenGL接口）
        for (int i = 0; i < 2; ++i)
        {
            // 创建顶点数组对象(VAO)
            glGenVertexArrays(1, &m_vao[i]);
            // 绑定VAO
            glBindVertexArray(m_vao[i]);

            // 创建顶点缓冲对象(VBO)
            glGenBuffers(1, &m_vbo[i]);
            // 绑定VBO
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[i]);
            // 预分配缓冲区内存，设置为动态绘制
            glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * VERTEX_STRIDE * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

            // 设置顶点属性指针 - 位置坐标
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                VERTEX_STRIDE * sizeof(float), (void*)0);

            // 设置顶点属性指针 - 颜色数据
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                VERTEX_STRIDE * sizeof(float), (void*)(2 * sizeof(float)));

            // 释放VBO绑定
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // 释放VAO绑定
            glBindVertexArray(0);
        }

        // 生成初始的随机线段数据
        generateRandomLines();

        // 将初始数据上传到两个缓冲区
        uploadToBuffer(0);
        uploadToBuffer(1);

        // 创建定时器，用于动画更新（约60fps）
        QTimer* timer = new QTimer(this);
        // 连接定时器超时信号到窗口的更新槽函数
        connect(timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
        // 每16毫秒触发一次（约60fps）
        timer->start(16);
    }

    /**
     * @brief OpenGL渲染函数
     *
     * 重写QOpenGLWidget的paintGL方法，实现双缓冲渲染流程：
     * 1. 等待前一帧的写入操作完成
     * 2. 更新数据并写入当前写缓冲区
     * 3. 从读缓冲区读取数据进行渲染
     * 4. 插入同步围栏确保渲染完成
     * 5. 交换读写缓冲区索引
     */
    void paintGL() override
    {
        // 1. 等待前一次对写缓冲区的操作完成（如果有）
        if (m_fences[m_writeIdx])
        {
            // 等待GPU完成指定的同步对象对应的命令，超时时间设为1秒
            glClientWaitSync(m_fences[m_writeIdx], GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000ULL);

            // 删除同步对象
            glDeleteSync(m_fences[m_writeIdx]);
            m_fences[m_writeIdx] = nullptr;
        }

        // 2. 更新顶点数据并上传到写缓冲区
        animateLines();
        uploadToBuffer(m_writeIdx);

        // 3. 从读缓冲区绘制
        // 设置清屏颜色背景
        glClearColor(0.15f, 0.15f, 0.01f, 1.0f);
        // 清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);

        // 如果读缓冲区有顶点数据
        if (m_vertexCount[m_drawIdx] > 0)
        {
            // 绑定着色器程序
            glUseProgram(m_program);
            // 绑定读缓冲区对应的VAO
            glBindVertexArray(m_vao[m_drawIdx]);
            // 使用读缓冲区的数据绘制线段
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_vertexCount[m_drawIdx]));
            // 释放VAO绑定
            glBindVertexArray(0);
            // 释放着色器程序
            glUseProgram(0);
        }

        // 4. 创建同步对象并刷新  插入同步围栏，标记当前帧的绘制命令完成点
        m_fences[m_drawIdx] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        // 5. 交换读写缓冲区索引，实现双缓冲轮换
        std::swap(m_writeIdx, m_drawIdx);
    }

    /**
     * @brief 窗口大小调整处理函数
     *
     * 重写QOpenGLWidget的resizeGL方法，当窗口大小改变时更新视口大小
     *
     * @param w 新的宽度
     * @param h 新的高度
     */
    void resizeGL(int w, int h) override
    {
        // 设置OpenGL视口大小与窗口一致
        glViewport(0, 0, w, h);
    }

private:
    /**
     * @brief 生成随机线段数据
     *
     * 创建初始的随机彩色线段，每个线段有两个顶点（起点和终点），
     * 每个顶点包含位置坐标和颜色信息。
     */
    void generateRandomLines()
    {
        // 使用固定种子的随机数生成器，确保结果可重现
        std::mt19937 rng(42);
        // 位置分布：[-0.9, 0.9]，避免线段靠近视口边缘
        std::uniform_real_distribution<float> pos(-0.9f, 0.9f);
        // 颜色分布：[0.3, 1.0]，确保颜色足够明亮可见
        std::uniform_real_distribution<float> col(0.3f, 1.0f);

        // 要生成的线段数量：5000条线段（每条线段2个顶点，共10000个顶点）
        const size_t lineCount = 5000;
        // 调整顶点数组大小，为所有线段预留空间
        m_vertices.resize(lineCount * 2 * VERTEX_STRIDE);

        // 为每条线段生成起点和终点
        for (size_t i = 0; i < lineCount; ++i)
        {
            // 计算当前线段在顶点数组中的起始位置
            size_t base = i * 2 * VERTEX_STRIDE;

            // 为这条线段生成一个随机颜色
            float r = col(rng), g = col(rng), b = col(rng);

            // 设置起点顶点数据（位置+颜色）
            m_vertices[base + 0] = pos(rng);  // x坐标
            m_vertices[base + 1] = pos(rng);  // y坐标
            m_vertices[base + 2] = r;         // 红色分量
            m_vertices[base + 3] = g;         // 绿色分量
            m_vertices[base + 4] = b;         // 蓝色分量

            // 设置终点顶点数据（位置+颜色）
            m_vertices[base + 5] = pos(rng);  // x坐标
            m_vertices[base + 6] = pos(rng);  // y坐标
            m_vertices[base + 7] = r;         // 红色分量
            m_vertices[base + 8] = g;         // 绿色分量
            m_vertices[base + 9] = b;         // 蓝色分量
        }

        // 更新两个缓冲区的顶点计数
        m_vertexCount[0] = m_vertexCount[1] = lineCount * 2;
    }

    /**
     * @brief 动画更新线段位置
     *
     * 每帧随机选择部分顶点并略微调整它们的位置，
     * 创造出随机波动的动画效果。
     */
    void animateLines()
    {
        // 跟踪时间进度（每帧递增）
        static float t = 0.0f;
        t += 0.016f;  // 约等于16毫秒/帧的时间增量

        // 使用基于时间的种子，确保每帧的随机变化不同
        std::mt19937 rng(static_cast<unsigned int>(t * 1000));
        // 位置分布范围
        std::uniform_real_distribution<float> pos(-0.9f, 0.9f);
        // 位置偏移量：每个顶点每次最多移动0.02个单位
        std::uniform_real_distribution<float> offset(-0.02f, 0.02f);

        // 每帧随机调整200个顶点的位置
        for (int i = 0; i < 200; ++i)
        {
            // 随机选择一个顶点索引
            size_t idx = (rng() % (m_vertices.size() / VERTEX_STRIDE)) * VERTEX_STRIDE;

            // 为选中的顶点添加随机偏移
            m_vertices[idx + 0] += offset(rng);  // x坐标偏移
            m_vertices[idx + 1] += offset(rng);  // y坐标偏移

            // 使用clamp函数确保顶点坐标始终在[-1.0, 1.0]范围内
            m_vertices[idx + 0] = std::clamp(m_vertices[idx + 0], -1.0f, 1.0f);
            m_vertices[idx + 1] = std::clamp(m_vertices[idx + 1], -1.0f, 1.0f);
        }
    }

    /**
     * @brief 将顶点数据上传到指定的VBO（使用原生OpenGL接口）
     *
     * 使用原生OpenGL函数glBindBuffer和glBufferSubData更新缓冲区数据，
     * 对于频繁更新的顶点数据，这种方法比映射缓冲区更高效。
     *
     * @param idx 要上传到的缓冲区索引（0或1）
     */
    void uploadToBuffer(int idx)
    {
        // 绑定目标VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[idx]);

        // 更新缓冲区数据 注意：此处假设缓冲区大小已足够，没有重新分配内存
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(float), m_vertices.data());

        // 释放VBO绑定
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

private:
    // 成员变量
    GLuint m_program = 0;  // 着色器程序ID（使用原生OpenGL）

    // 双缓冲相关资源 - 原生OpenGL ID
    GLuint m_vao[2] = { 0, 0 };       // 顶点数组对象ID（使用原生OpenGL，双缓冲）
    GLuint m_vbo[2] = { 0, 0 };       // 顶点缓冲对象ID（使用原生OpenGL，双缓冲）

    // 顶点数据存储
    std::vector<float> m_vertices;      // 顶点数据容器
    size_t m_vertexCount[2] = { 0, 0 }; // 每个缓冲区的顶点数量

    // 双缓冲索引管理
    int m_writeIdx = 0;  // 当前写缓冲区索引
    int m_drawIdx = 1;   // 当前读（绘制）缓冲区索引

    // OpenGL同步对象，用于确保GPU操作顺序
    GLsync m_fences[2] = { nullptr, nullptr };  // 同步围栏对象（双缓冲）
};

/**
 * @brief 程序入口函数
 *
 * 创建Qt应用程序实例，配置OpenGL上下文格式，
 * 创建并显示DoubleBufferWidget窗口。
 *
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 应用程序退出码
 */
int main(int argc, char* argv[])
{
    // 创建Qt应用程序实例
    QApplication app(argc, argv);

    // 配置OpenGL上下文格式
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);                   // 使用OpenGL 3.3版本
    fmt.setProfile(QSurfaceFormat::CoreProfile);  // 使用核心配置文件
    fmt.setDepthBufferSize(24);             // 深度缓冲区大小
    fmt.setStencilBufferSize(8);            // 模板缓冲区大小
    fmt.setSamples(4);                      // 启用4x多重采样抗锯齿（MSAA）
    // 设置为默认格式，影响后续创建的所有OpenGL窗口
    QSurfaceFormat::setDefaultFormat(fmt);

    // 创建DoubleBufferWidget实例
    DoubleBufferWidget w;
    // 设置窗口标题
    w.setWindowTitle("Qt OpenGL 3.3 - Double-Buffered Animated Lines (Fixed)");
    // 设置窗口大小
    w.resize(1400, 1400);
    // 显示窗口
    w.show();

    // 运行应用程序事件循环
    return app.exec();
}

// 由于在.cpp文件中定义了带有Q_OBJECT宏的类，需要包含moc生成的文件
// 注意：这是一种特殊情况，如果类定义在.h文件中，则不需要此行
#include "DoubleBuffer.moc"