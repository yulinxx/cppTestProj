// 此注释用于占位，表明后续代码将在此处添加
// 不修改 正交投影, 从而改变虚线间隔

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
layout(location = 0) in vec2 in_pos;
layout(location = 1) in float in_len;
uniform mat4 cameraTrans;
uniform float dashScale;
uniform float timeOffset = 0.0;
out float dashParam;

void main() {
    gl_Position = cameraTrans * vec4(in_pos, 0.0, 1.0);
    dashParam = in_len * dashScale + timeOffset;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in float dashParam;
uniform vec4 color;
out vec4 fragColor;

void main() {
    float dashPattern = mod(dashParam, 1.0);
    if (dashPattern < 0.5) {
        fragColor = color;
    } else {
        discard;
    }
}
)";

/**
 * @brief 加载并编译顶点着色器和片段着色器，然后链接成一个着色器程序。
 * 
 * @param vertexShaderSource 顶点着色器的源代码。
 * @param fragmentShaderSource 片段着色器的源代码。
 * @return GLuint 编译并链接后的着色器程序的 ID。
 */
GLuint loadShader(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    // 创建顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 设置顶点着色器源代码
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    // 编译顶点着色器
    glCompileShader(vertexShader);

    // 创建片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 设置片段着色器源代码
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    // 编译片段着色器
    glCompileShader(fragmentShader);

    // 创建着色器程序
    GLuint shaderProgram = glCreateProgram();
    // 附加顶点着色器到程序
    glAttachShader(shaderProgram, vertexShader);
    // 附加片段着色器到程序
    glAttachShader(shaderProgram, fragmentShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);

    // 删除已编译的顶点着色器
    glDeleteShader(vertexShader);
    // 删除已编译的片段着色器
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

/**
 * @brief 生成一个指定范围内的随机二维点。
 * 
 * @param minX 点的 x 坐标的最小值，默认为 -X。
 * @param maxX 点的 x 坐标的最大值，默认为 X。
 * @param minY 点的 y 坐标的最小值，默认为 -X。
 * @param maxY 点的 y 坐标的最大值，默认为 X。
 * @return glm::vec2 生成的随机二维点。
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
 * @brief 生成随机的混合线段，包括直线段和贝塞尔曲线段。
 * 
 * @param vertices 存储生成的顶点数据的向量。
 * @param numSegments 线段的总数。
 * @param lineSegments 每个直线段的细分数量。
 * @param bezierSegments 每个贝塞尔曲线段的细分数量。
 * @param minX 点的 x 坐标的最小值。
 * @param maxX 点的 x 坐标的最大值。
 * @param minY 点的 y 坐标的最小值。
 * @param maxY 点的 y 坐标的最大值。
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
    // 累积长度初始化为 0
    float dAccLen = 0.0;

    // 添加起始点到顶点向量
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
            // 添加点的 x 坐标到顶点向量
            vertices.push_back(point.x);
            // 添加点的 y 坐标到顶点向量
            vertices.push_back(point.y);

            // 计算线段长度
            float segmentLength = glm::distance(prevPoint, point);
            // 累积长度
            dAccLen += segmentLength;
            // 添加累积长度到顶点向量
            vertices.push_back(dAccLen);

            // 重复添加点和累积长度，用于绘制线段
            vertices.push_back(point.x);
            vertices.push_back(point.y);
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
                // 计算参数 t
                float t = float(j) / float(bezierSegments);
                // 计算参数 u
                float u = 1.0f - t;

                // 计算贝塞尔曲线上的点
                glm::vec2 point = u * u * u * currentPoint +
                    3.0f * u * u * t * controlPoint1 +
                    3.0f * u * t * t * controlPoint2 +
                    t * t * t * nextPoint;

                // 添加点的 x 坐标到顶点向量
                vertices.push_back(point.x);
                // 添加点的 y 坐标到顶点向量
                vertices.push_back(point.y);

                // 计算线段长度
                float segmentLength = glm::distance(prevPoint, point);
                // 累积长度
                dAccLen += segmentLength;
                // 添加累积长度到顶点向量
                vertices.push_back(dAccLen);

                // 重复添加点和累积长度，用于绘制线段
                vertices.push_back(point.x);
                vertices.push_back(point.y);
                vertices.push_back(dAccLen);

                // 更新前一个点
                prevPoint = point;
            }

            // 更新当前点
            currentPoint = nextPoint;
        }
    }
}

// 缩放因子
float zoomFactor = 1.0f;
// 全局变量存储 cameraTrans uniform 的位置
GLint cameraTransLoc;  

/**
 * @brief 处理鼠标滚轮滚动事件，更新缩放因子和相机变换矩阵。
 * 
 * @param window 当前的 GLFW 窗口。
 * @param xoffset 鼠标滚轮在 x 方向的偏移量。
 * @param yoffset 鼠标滚轮在 y 方向的偏移量。
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // 更新缩放因子
    zoomFactor += float(yoffset) * 0.1f;
    // 确保缩放因子不小于 0.1
    zoomFactor = std::max(zoomFactor, 0.1f);

    // 计算新的相机变换矩阵
    glm::mat4 cameraTrans = glm::ortho(-X * zoomFactor, X * zoomFactor, -X * zoomFactor, X * zoomFactor);
    // 更新相机变换矩阵的 uniform 值
    glUniformMatrix4fv(cameraTransLoc, 1, GL_FALSE, &cameraTrans[0][0]);
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

    // 初始化随机数种子
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
    // 设置当前上下文
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        // 输出错误信息
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 加载着色器程序
    GLuint shaderProgram = loadShader(vertexShaderSource, fragmentShaderSource);
    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 初始化相机变换矩阵
    glm::mat4 cameraTrans = glm::ortho(-X, X, -X, X);
    // 获取 cameraTrans uniform 的位置
    cameraTransLoc = glGetUniformLocation(shaderProgram, "cameraTrans");
    // 设置 cameraTrans uniform 的值
    glUniformMatrix4fv(cameraTransLoc, 1, GL_FALSE, &cameraTrans[0][0]);

    // 设置线段颜色
    glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);

    // 存储顶点数据的向量
    std::vector<float> shapeVertices;
    {
        // 线段数量
        const int NUM_LINES = 6;
        // 线段细分数量
        const int NUM_SEGMENTS = 3;
        // 贝塞尔曲线细分数量
        const int BEZIER_RES = 30;
        // 圆的细分数量
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
    // 设置顶点缓冲数据
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(float), shapeVertices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<void*>(0 * sizeof(float)));
    // 启用顶点属性
    glEnableVertexAttribArray(0);

    // 设置顶点属性指针
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<void*>(2 * sizeof(float)));
    // 启用顶点属性
    glEnableVertexAttribArray(1);

    // 设置鼠标滚轮回调函数
    glfwSetScrollCallback(window, scroll_callback);

    // 设置清屏颜色
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 计算虚线缩放因子
        float dashScale = 4.0f / zoomFactor;
        // 设置虚线缩放因子的 uniform 值
        glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), dashScale);

        // 获取当前时间
        auto now = std::chrono::high_resolution_clock::now();
        // 计算时间差
        auto duration = std::chrono::duration<float>(now.time_since_epoch());
        // 获取时间值
        float time = duration.count();
        // 设置时间偏移量的 uniform 值
        glUniform1f(glGetUniformLocation(shaderProgram, "timeOffset"), time * 0.8f);

        // 清屏
        glClear(GL_COLOR_BUFFER_BIT);

        // 绑定顶点数组对象
        glBindVertexArray(VAO);
        // 绘制线段
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(shapeVertices.size() / 3));

        // 交换缓冲区
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