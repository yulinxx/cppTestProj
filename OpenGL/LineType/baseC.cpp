// 既不在主循环中更新 cameraTrans,也不在 scroll_callback 中更新cameraTrans,因为投影矩阵可能会影响其他 Shader,而你想让缩放逻辑独立于投影矩阵,同时保持虚线间隔固定.

// 在这种情况下,我们需要一种方法,让缩放效果不依赖于修改 cameraTrans,而是将缩放逻辑完全隔离到当前 Shader 的顶点处理中,并且不影响其他 Shader.最好的办法是:

// 在顶点着色器中引入一个独立的缩放因子(zoomFactor),直接作用于顶点位置.
// 将固定的 cameraTrans 设置一次,不再动态修改.
// 通过主循环更新 zoomFactor 和 dashScale,以保持虚线间隔固定.
// 以下是完整实现:

// 关键改动

// 顶点着色器中的缩放:
// 添加 uniform float zoomFactor = 1.0f,并在着色器中对顶点位置应用缩放:scaledPos = in_pos / zoomFactor.
// 使用除法(/)而不是乘法(*),因为 zoomFactor 增大时应缩小视图,反之亦然,这样与你的滚轮方向一致(向上滚放大,向下滚缩小).
// 固定 cameraTrans:
// 在初始化时设置一次 glm::ortho(-X, X, -X, X),之后不再修改,保证不影响其他 Shader.
// 主循环中更新:
// 只更新 dashScale(4.0f / zoomFactor)和 zoomFactor,保持虚线间隔固定.
// scroll_callback 的简化:
// 只更新 zoomFactor,不触及 OpenGL 状态.

// 工作原理
// zoomFactor 在着色器中的作用:
// 当 zoomFactor 增大(例如从 1.0 到 2.0),in_pos / zoomFactor 使顶点坐标变小,视图放大.
// 当 zoomFactor 减小(例如从 1.0 到 0.5),in_pos / zoomFactor 使顶点坐标变大,视图缩小.
// dashScale 的调整:
// dashScale = 4.0f / zoomFactor 确保虚线间隔在屏幕空间中保持固定,因为它与缩放因子反向关联.
// 不影响其他 Shader:
// cameraTrans 保持不变,其他 Shader 不会受到影响.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>

constexpr float X = 4.0f;

const char* vertexShaderSource = R"(
#version 330 core
// 输入顶点位置
layout(location = 0) in vec2 in_pos;
// 输入线段长度
layout(location = 1) in float in_len;
// 相机变换矩阵
uniform mat4 cameraTrans;
// 虚线缩放因子
uniform float dashScale;
// 独立的缩放因子,默认值为 1.0f
uniform float zoomFactor = 1.0f;
// 时间偏移量
uniform float timeOffset = 0.0;
// 输出虚线参数
out float dashParam;

void main() {
    // 在着色器中应用缩放(除以 zoomFactor 表示放大)
    vec2 scaledPos = in_pos / zoomFactor;
    // 计算顶点的最终位置
    gl_Position = cameraTrans * vec4(scaledPos, 0.0, 1.0);
    // 计算虚线参数
    dashParam = in_len * dashScale + timeOffset;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
// 输入虚线参数
in float dashParam;
// 颜色
uniform vec4 color;
// 输出片段颜色
out vec4 fragColor;

void main() {
    // 计算虚线模式
    float dashPattern = mod(dashParam, 1.0);
    // 如果虚线模式小于 0.5,则绘制颜色
    if (dashPattern < 0.5) {
        fragColor = color;
    } else {
        // 否则丢弃片段
        discard;
    }
}
)";

/**
 * @brief 加载并编译顶点着色器和片段着色器,然后链接着色器程序
 *
 * @param vertexShaderSource 顶点着色器的源代码
 * @param fragmentShaderSource 片段着色器的源代码
 * @return GLuint 编译并链接好的着色器程序的 ID
 */
GLuint loadShader(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    // 创建顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 设置顶点着色器的源代码
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    // 编译顶点着色器
    glCompileShader(vertexShader);

    // 创建片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 设置片段着色器的源代码
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    // 编译片段着色器
    glCompileShader(fragmentShader);

    // 创建着色器程序
    GLuint shaderProgram = glCreateProgram();
    // 将顶点着色器附加到着色器程序
    glAttachShader(shaderProgram, vertexShader);
    // 将片段着色器附加到着色器程序
    glAttachShader(shaderProgram, fragmentShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);

    // 删除顶点着色器
    glDeleteShader(vertexShader);
    // 删除片段着色器
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

/**
 * @brief 生成一个指定范围内的随机点
 *
 * @param minX 点的最小 x 坐标
 * @param maxX 点的最大 x 坐标
 * @param minY 点的最小 y 坐标
 * @param maxY 点的最大 y 坐标
 * @return glm::vec2 生成的随机点
 */
glm::vec2 randomPoint(float minX = -X, float maxX = X, float minY = -X, float maxY = X)
{
    return glm::vec2(
        // 生成随机 x 坐标
        minX + (rand() / static_cast<float>(RAND_MAX)) * (maxX - minX),
        // 生成随机 y 坐标
        minY + (rand() / static_cast<float>(RAND_MAX)) * (maxY - minY)
    );
}

/**
 * @brief 生成随机的混合线段(直线和贝塞尔曲线)
 *
 * @param vertices 存储生成的顶点数据的向量
 * @param numSegments 线段的总数
 * @param lineSegments 直线段的数量
 * @param bezierSegments 贝塞尔曲线段的数量
 * @param minX 点的最小 x 坐标
 * @param maxX 点的最大 x 坐标
 * @param minY 点的最小 y 坐标
 * @param maxY 点的最大 y 坐标
 */
void generateRandomMixedLine(
    std::vector<float>& vertices,
    int numSegments,
    int lineSegments,
    int bezierSegments,
    float minX, float maxX, float minY, float maxY
)
{
    // 生成随机起始点
    glm::vec2 startPoint = randomPoint(minX, maxX, minY, maxY);
    // 当前点初始化为起始点
    glm::vec2 currentPoint = startPoint;
    // 前一个点初始化为起始点
    glm::vec2 prevPoint = startPoint;
    // 累计线段长度
    float dAccLen = 0.0;

    // 添加起始点到顶点数据
    vertices.push_back(startPoint.x);
    vertices.push_back(startPoint.y);
    vertices.push_back(dAccLen);

    for (int i = 0; i < numSegments; ++i)
    {
        // 随机决定是否为直线段
        bool bLine = rand() % 2 == 0;

        if (bLine)
        {
            // 生成随机点
            glm::vec2 point = randomPoint(minX, maxX, minY, maxY);
            // 添加点的 x 坐标到顶点数据
            vertices.push_back(point.x);
            // 添加点的 y 坐标到顶点数据
            vertices.push_back(point.y);

            // 计算线段长度
            float segmentLength = glm::distance(prevPoint, point);
            // 累计线段长度
            dAccLen += segmentLength;
            // 添加累计长度到顶点数据
            vertices.push_back(dAccLen);

            // 添加点的 x 坐标到顶点数据
            vertices.push_back(point.x);
            // 添加点的 y 坐标到顶点数据
            vertices.push_back(point.y);
            // 添加累计长度到顶点数据
            vertices.push_back(dAccLen);

            // 更新前一个点
            prevPoint = point;
            // 更新当前点
            currentPoint = point;
        }
        else
        {
            // 生成随机控制点 1
            glm::vec2 controlPoint1 = randomPoint(minX, maxX, minY, maxY);
            // 生成随机控制点 2
            glm::vec2 controlPoint2 = randomPoint(minX, maxX, minY, maxY);
            // 生成随机下一个点
            glm::vec2 nextPoint = randomPoint(minX, maxX, minY, maxY);

            for (int j = 1; j <= bezierSegments; ++j)
            {
                // 计算贝塞尔曲线的参数 t
                float t = float(j) / float(bezierSegments);
                // 计算 1 - t
                float u = 1.0f - t;

                // 计算贝塞尔曲线上的点
                glm::vec2 point = u * u * u * currentPoint +
                    3.0f * u * u * t * controlPoint1 +
                    3.0f * u * t * t * controlPoint2 +
                    t * t * t * nextPoint;

                // 添加点的 x 坐标到顶点数据
                vertices.push_back(point.x);
                // 添加点的 y 坐标到顶点数据
                vertices.push_back(point.y);

                // 计算线段长度
                float segmentLength = glm::distance(prevPoint, point);
                // 累计线段长度
                dAccLen += segmentLength;
                // 添加累计长度到顶点数据
                vertices.push_back(dAccLen);

                // 添加点的 x 坐标到顶点数据
                vertices.push_back(point.x);
                // 添加点的 y 坐标到顶点数据
                vertices.push_back(point.y);
                // 添加累计长度到顶点数据
                vertices.push_back(dAccLen);

                // 更新前一个点
                prevPoint = point;
            }

            // 更新当前点
            currentPoint = nextPoint;
        }
    }
}

// 缩放因子,初始值为 1.0f
float zoomFactor = 1.0f;

/**
 * @brief 处理鼠标滚轮滚动事件,更新缩放因子
 *
 * @param window GLFW 窗口指针
 * @param xoffset 鼠标滚轮在 x 方向的偏移量
 * @param yoffset 鼠标滚轮在 y 方向的偏移量
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // 根据鼠标滚轮的偏移量更新缩放因子
    zoomFactor += float(yoffset) * 0.1f;
    // 确保缩放因子不小于 0.1f
    zoomFactor = std::max(zoomFactor, 0.1f);  // 只更新 zoomFactor,不触动 cameraTrans
}

int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        // 输出错误信息
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置随机数种子
    srand(static_cast<unsigned int>(time(NULL)));

    // 设置 OpenGL 上下文版本和配置
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // 创建 GLFW 窗口
    GLFWwindow* window = glfwCreateWindow(1400, 1400, "OpenGL Dash Circle", nullptr, nullptr);
    if (!window)
    {
        // 输出错误信息
        std::cerr << "Failed to create GLFW window" << std::endl;
        // 终止 GLFW
        glfwTerminate();
        return -1;
    }
    // 将窗口设置为当前上下文
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        // 输出错误信息
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    // 设置鼠标滚轮滚动回调函数
    glfwSetScrollCallback(window, scroll_callback);

    // 加载并编译着色器程序
    GLuint shaderProgram = loadShader(vertexShaderSource, fragmentShaderSource);
    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 设置固定的投影矩阵,只初始化一次
    glm::mat4 cameraTrans = glm::ortho(-X, X, -X, X);
    // 设置相机变换矩阵的统一变量
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

    // 设置颜色的统一变量
    glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);

    // 存储形状的顶点数据
    std::vector<float> shapeVertices;
    {
        // 线段数量
        const int NUM_LINES = 6;
        // 线段段数
        const int NUM_SEGMENTS = 3;
        // 贝塞尔曲线分辨率
        const int BEZIER_RES = 30;
        // 圆形分辨率
        const int CIRCLE_RES = 350;
        // 生成随机混合线段
        generateRandomMixedLine(shapeVertices, NUM_LINES, BEZIER_RES, BEZIER_RES, -X, X, -X, X);
    }

    // 顶点数组对象
    GLuint VAO, VBO;
    // 生成顶点数组对象
    glGenVertexArrays(1, &VAO);
    // 生成顶点缓冲对象
    glGenBuffers(1, &VBO);

    // 绑定顶点数组对象
    glBindVertexArray(VAO);
    // 绑定顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 将顶点数据复制到缓冲对象
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(float), shapeVertices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<void*>(0 * sizeof(float)));
    // 启用顶点属性
    glEnableVertexAttribArray(0);

    // 设置顶点属性指针
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<void*>(2 * sizeof(float)));
    // 启用顶点属性
    glEnableVertexAttribArray(1);

    // 设置清屏颜色
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 更新 dashScale 和 zoomFactor,不触动 cameraTrans
        // 计算虚线缩放因子,保持虚线间隔固定
        float dashScale = 4.0f / zoomFactor;
        // 设置虚线缩放因子的统一变量
        glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), dashScale);
        // 设置缩放因子的统一变量
        glUniform1f(glGetUniformLocation(shaderProgram, "zoomFactor"), zoomFactor);

        // 获取当前时间
        auto now = std::chrono::high_resolution_clock::now();
        // 计算时间差
        auto duration = std::chrono::duration<float>(now.time_since_epoch());
        // 获取时间值
        float time = duration.count();
        // 设置时间偏移量的统一变量
        glUniform1f(glGetUniformLocation(shaderProgram, "timeOffset"), time * 0.8f);

        // 清屏
        glClear(GL_COLOR_BUFFER_BIT);

        // 绑定顶点数组对象
        glBindVertexArray(VAO);
        // 绘制线段
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(shapeVertices.size() / 3));

        // 交换前后缓冲区
        glfwSwapBuffers(window);
        // 处理事件
        glfwPollEvents();
    }

    // 删除着色器程序
    glDeleteProgram(shaderProgram);
    // 删除顶点数组对象
    glDeleteVertexArrays(1, &VAO);
    // 删除顶点缓冲对象
    glDeleteBuffers(1, &VBO);
    // 销毁 GLFW 窗口
    glfwDestroyWindow(window);
    // 终止 GLFW
    glfwTerminate();

    return 0;
}