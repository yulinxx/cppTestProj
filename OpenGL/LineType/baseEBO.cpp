// 生成多条连续的线(而不是单条线段的集合).
// 使用 GL_LINE_STRIP 进行绘制,并确保顶点和索引存储时避免重复.
// 优化后的代码将调整 generateRandomMixedLine 函数以生成多条连续的折线,同时改进索引生成逻辑,使其适配 GL_LINE_STRIP 的连续绘制方式.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>
#include <string>

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"

// 定义一个常量 X,用于指定绘图区域的范围
constexpr float X = 4.0f;

// 顶点着色器代码,使用 OpenGL Shading Language (GLSL) 编写
// 该着色器负责处理顶点的位置和长度信息,并将其转换为裁剪空间坐标
const char* vertexShaderSource = R"(
#version 330 core
// 输入:顶点位置
layout(location = 0) in vec2 in_pos;
// 输入:顶点到起点的累积长度
layout(location = 1) in float in_len;

// 均匀变量:相机变换矩阵
uniform mat4 cameraTrans;
// 均匀变量:虚线缩放因子
uniform float dashScale;
// 均匀变量:时间偏移量,用于动画效果
uniform float timeOffset = 0.0;

// 输出:传递给片段着色器的虚线参数
out float dashParam;

void main() {
    // 将顶点位置转换为裁剪空间坐标
    gl_Position = cameraTrans * vec4(in_pos, 0.0, 1.0);
    // 计算虚线长度
    float dashLength = in_len * dashScale + timeOffset;
    // 将虚线长度传递给片段着色器
    dashParam = dashLength;
}
)";

// 片段着色器代码,使用 OpenGL Shading Language (GLSL) 编写
// 该着色器负责处理每个片段的颜色和虚线绘制逻辑
const char* fragmentShaderSource = R"(
#version 330 core
// 输入:从顶点着色器传递过来的虚线参数
in float dashParam;
// 均匀变量:线段颜色
uniform vec4 color;
// 均匀变量:虚线类型,默认为 0
uniform int dashType = 0;
// 输出:片段颜色
out vec4 fragColor;

void main() {
    // 标志位,用于判断是否绘制该片段
    bool draw = false;
    // 存储虚线模式的临时变量
    float pattern;
    // 存储虚线周期的临时变量
    float cycle;

    // 根据虚线类型选择不同的绘制模式
    switch(dashType) {
        case 0: // 默认等长虚线
            // 计算虚线模式
            pattern = mod(dashParam, 1.0);
            // 判断是否绘制该片段
            draw = (pattern < 0.5);
            break;
        default:
            // 其他情况绘制所有片段
            draw = true;
            break;
    }

    // 如果不绘制该片段,则丢弃
    if (!draw) discard;
    // 设置片段颜色
    fragColor = color;
}
)";

/**
 * @brief 加载并编译顶点着色器和片段着色器,然后链接成一个着色器程序
 *
 * @param vertexShaderSource 顶点着色器的源代码
 * @param fragmentShaderSource 片段着色器的源代码
 * @return GLuint 着色器程序的 ID
 */
GLuint loadShader(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    // 创建顶点着色器对象
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 指定顶点着色器的源代码
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    // 编译顶点着色器
    glCompileShader(vertexShader);

    // 检查顶点着色器编译是否成功
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // 如果编译失败,输出错误信息
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 创建片段着色器对象
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 指定片段着色器的源代码
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    // 编译片段着色器
    glCompileShader(fragmentShader);

    // 检查片段着色器编译是否成功
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // 如果编译失败,输出错误信息
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 创建着色器程序对象
    GLuint shaderProgram = glCreateProgram();
    // 将顶点着色器附加到着色器程序
    glAttachShader(shaderProgram, vertexShader);
    // 将片段着色器附加到着色器程序
    glAttachShader(shaderProgram, fragmentShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);

    // 检查着色器程序链接是否成功
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        // 如果链接失败,输出错误信息
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    // 删除不再需要的顶点着色器和片段着色器
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 返回着色器程序的 ID
    return shaderProgram;
}

/**
 * @brief 生成一个随机的二维点
 *
 * @param minX 点的最小 x 坐标
 * @param maxX 点的最大 x 坐标
 * @param minY 点的最小 y 坐标
 * @param maxY 点的最大 y 坐标
 * @return glm::vec2 生成的随机点
 */
glm::vec2 randomPoint(float minX = -X, float maxX = X, float minY = -X, float maxY = X)
{
    // 生成随机的 x 坐标
    float x = minX + (rand() / static_cast<float>(RAND_MAX)) * (maxX - minX);
    // 生成随机的 y 坐标
    float y = minY + (rand() / static_cast<float>(RAND_MAX)) * (maxY - minY);
    // 返回生成的随机点
    return glm::vec2(x, y);
}

/**
 * @brief 生成多条随机混合的折线,包括直线段和贝塞尔曲线段
 *
 * @param vertices 存储所有顶点数据的向量
 * @param lineIndices 存储每条折线的索引数据的向量
 * @param numLines 要生成的折线数量
 * @param numSegments 每条折线的线段数量
 * @param bezierSegments 每个贝塞尔曲线段的细分数量
 * @param minX 点的最小 x 坐标
 * @param maxX 点的最大 x 坐标
 * @param minY 点的最小 y 坐标
 * @param maxY 点的最大 y 坐标
 */
void generateRandomMixedLines(
    std::vector<float>& vertices,
    std::vector<std::vector<unsigned int>>& lineIndices,
    int numLines,
    int numSegments,
    int bezierSegments,
    float minX, float maxX, float minY, float maxY
)
{
    // 清空顶点数据和索引数据
    vertices.clear();
    lineIndices.clear();

    // 循环生成每条折线
    for (int line = 0; line < numLines; ++line)
    {
        // 存储当前折线的索引数据
        std::vector<unsigned int> currentLineIndices;
        // 生成折线的起始点
        glm::vec2 startPoint = randomPoint(minX, maxX, minY, maxY);
        // 当前点初始化为起始点
        glm::vec2 currentPoint = startPoint;
        // 前一个点初始化为起始点
        glm::vec2 prevPoint = startPoint;
        // 累积长度初始化为 0
        float dAccLen = 0.0;

        // 获取当前顶点的索引
        unsigned int vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
        // 添加起始点的 x 坐标
        vertices.push_back(startPoint.x);
        // 添加起始点的 y 坐标
        vertices.push_back(startPoint.y);
        // 添加起始点的累积长度
        vertices.push_back(dAccLen);
        // 将起始点的索引添加到当前折线的索引数据中
        currentLineIndices.push_back(vertexIndex);

        // 循环生成每条折线的线段
        for (int i = 0; i < numSegments; ++i)
        {
            // 随机决定是直线段还是贝塞尔曲线段
            bool bLine = rand() % 2 == 0;

            if (bLine)
            {
                // 生成一个随机点
                glm::vec2 point = randomPoint(minX, maxX, minY, maxY);
                // 获取当前顶点的索引
                vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
                // 添加随机点的 x 坐标
                vertices.push_back(point.x);
                // 添加随机点的 y 坐标
                vertices.push_back(point.y);

                // 计算当前线段的长度
                float segmentLength = glm::distance(prevPoint, point);
                // 更新累积长度
                dAccLen += segmentLength;
                // 添加当前点的累积长度
                vertices.push_back(dAccLen);

                // 将当前点的索引添加到当前折线的索引数据中
                currentLineIndices.push_back(vertexIndex);

                // 更新前一个点为当前点
                prevPoint = point;
                // 更新当前点为当前点
                currentPoint = point;
            }
            else
            {
                // 生成贝塞尔曲线的控制点和终点
                glm::vec2 controlPoint1 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 controlPoint2 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 nextPoint = randomPoint(minX, maxX, minY, maxY);

                // 细分贝塞尔曲线
                for (int j = 1; j <= bezierSegments; ++j)
                {
                    // 计算当前细分点的参数 t
                    float t = float(j) / float(bezierSegments);
                    // 计算 1 - t
                    float u = 1.0f - t;

                    // 计算当前细分点的坐标
                    glm::vec2 point = u * u * u * currentPoint +
                        3.0f * u * u * t * controlPoint1 +
                        3.0f * u * t * t * controlPoint2 +
                        t * t * t * nextPoint;

                    // 获取当前顶点的索引
                    vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
                    // 添加当前细分点的 x 坐标
                    vertices.push_back(point.x);
                    // 添加当前细分点的 y 坐标
                    vertices.push_back(point.y);

                    // 计算当前细分段的长度
                    float segmentLength = glm::distance(prevPoint, point);
                    // 更新累积长度
                    dAccLen += segmentLength;
                    // 添加当前细分点的累积长度
                    vertices.push_back(dAccLen);

                    // 将当前细分点的索引添加到当前折线的索引数据中
                    currentLineIndices.push_back(vertexIndex);

                    // 更新前一个点为当前细分点
                    prevPoint = point;
                }
                // 更新当前点为贝塞尔曲线的终点
                currentPoint = nextPoint;
            }
        }
        // 将当前折线的索引数据添加到总的索引数据中
        lineIndices.push_back(currentLineIndices);
    }
}

// 缩放因子,用于控制视图的缩放
float zoomFactor = 1.0f;

/**
 * @brief 鼠标滚轮滚动回调函数,用于处理视图的缩放
 *
 * @param window GLFW 窗口对象
 * @param xoffset 鼠标滚轮在 x 方向的偏移量
 * @param yoffset 鼠标滚轮在 y 方向的偏移量
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // 根据鼠标滚轮的偏移量更新缩放因子
    zoomFactor += float(yoffset) * 0.1f;
    // 确保缩放因子不小于 0.1
    zoomFactor = std::max(zoomFactor, 0.1f);
}

/**
 * @brief 主函数,程序的入口点
 *
 * @return int 程序的退出状态码
 */
int main()
{
    // 初始化 GLFW 库
    if (!glfwInit())
    {
        // 如果初始化失败,输出错误信息并返回 -1
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置随机数种子
    srand(static_cast<unsigned int>(time(NULL)));

    // 设置 GLFW 窗口的 OpenGL 上下文版本和配置
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // 创建 GLFW 窗口
    GLFWwindow* window = glfwCreateWindow(1400, 1400, "OpenGL Dash Lines", nullptr, nullptr);
    if (!window)
    {
        // 如果窗口创建失败,输出错误信息,终止 GLFW 并返回 -1
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 将窗口的上下文设置为当前上下文
    glfwMakeContextCurrent(window);

    // 初始化 GLAD 库,用于加载 OpenGL 函数
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        // 如果初始化失败,输出错误信息并返回 -1
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

    // 创建正交投影矩阵
    glm::mat4 cameraTrans = glm::ortho(-X, X, -X, X);
    // 设置着色器程序中的相机变换矩阵
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

    // 设置着色器程序中的线段颜色
    glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);
    // 设置着色器程序中的虚线缩放因子
    glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), 8.0f);

    // 存储所有顶点数据的向量
    std::vector<float> shapeVertices;
    // 存储每条折线的索引数据的向量
    std::vector<std::vector<unsigned int>> shapeIndices;

    {
        // 定义要生成的折线数量
        const int NUM_LINES = 6;
        // 定义每条折线的线段数量
        const int NUM_SEGMENTS = 3;
        // 定义每个贝塞尔曲线段的细分数量
        const int BEZIER_RES = 30;
        // 生成随机混合的折线
        generateRandomMixedLines(shapeVertices, shapeIndices, NUM_LINES, NUM_SEGMENTS, BEZIER_RES, -X, X, -X, X);
    }

    // 顶点数组对象的 ID
    GLuint VAO, VBO, EBO;
    // 生成顶点数组对象
    glGenVertexArrays(1, &VAO);
    // 生成顶点缓冲对象
    glGenBuffers(1, &VBO);
    // 生成元素缓冲对象
    glGenBuffers(1, &EBO);

    // 绑定顶点数组对象
    glBindVertexArray(VAO);

    // 绑定顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 将顶点数据上传到顶点缓冲对象
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(float), shapeVertices.data(), GL_STATIC_DRAW);

    // 存储所有索引数据的一维向量
    std::vector<unsigned int> flatIndices;
    // 将每条折线的索引数据展平为一维向量
    for (const auto& line : shapeIndices)
    {
        flatIndices.insert(flatIndices.end(), line.begin(), line.end());
    }
    // 绑定元素缓冲对象
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // 将索引数据上传到元素缓冲对象
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size() * sizeof(unsigned int), flatIndices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针,指定顶点位置数据
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 启用顶点属性
    glEnableVertexAttribArray(0);
    // 设置顶点属性指针,指定顶点累积长度数据
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    // 启用顶点属性
    glEnableVertexAttribArray(1);

    // 设置清屏颜色为白色
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // 主循环,直到窗口关闭
    while (!glfwWindowShouldClose(window))
    {
        // 根据缩放因子更新相机变换矩阵
        glm::mat4 cameraTrans = glm::ortho(-X * zoomFactor, X * zoomFactor, -X * zoomFactor, X * zoomFactor);
        // 设置着色器程序中的相机变换矩阵
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

        // 根据缩放因子更新虚线缩放因子
        float dashScale = 12.0f / zoomFactor;
        // 设置着色器程序中的虚线缩放因子
        glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), dashScale);

        // 获取当前时间
        auto now = std::chrono::high_resolution_clock::now();
        // 计算从纪元开始的时间差
        auto duration = std::chrono::duration<float>(now.time_since_epoch());
        // 获取时间差的秒数
        float time = duration.count();

        // 设置着色器程序中的时间偏移量
        glUniform1f(glGetUniformLocation(shaderProgram, "timeOffset"), time * 0.8f);

        // 清屏
        glClear(GL_COLOR_BUFFER_BIT);

        // 绑定顶点数组对象
        glBindVertexArray(VAO);

        // 存储每条折线的索引偏移量
        std::vector<const GLvoid*> indicesOffsets;
        // 存储每条折线的索引数量
        std::vector<GLsizei> counts;
        // 索引偏移量的初始值
        size_t offset = 0;
        // 计算每条折线的索引偏移量和索引数量
        for (const auto& line : shapeIndices)
        {
            // 存储当前折线的索引数量
            counts.push_back(static_cast<GLsizei>(line.size()));
            // 存储当前折线的索引偏移量
            indicesOffsets.push_back(reinterpret_cast<const GLvoid*>(offset * sizeof(unsigned int)));
            // 更新索引偏移量
            offset += line.size();
        }

        // 使用 glMultiDrawElements 函数绘制多条折线
        glMultiDrawElements(GL_LINE_STRIP, counts.data(), GL_UNSIGNED_INT, indicesOffsets.data(), static_cast<GLsizei>(counts.size()));

        // 检查 OpenGL 错误
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            // 如果有错误,输出错误信息
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        // static int i = 0;
        // i++;
        // if (i % 4000 == 0 && false) {
        //     int width, height;
        //     glfwGetWindowSize(window, &width, &height);
        //     std::vector<unsigned char> pixels(width * height * 4);
        //     glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        //     stbi_flip_vertically_on_write(1);
        //     std::string filename = "frame_" + std::to_string(i) + ".png";
        //     stbi_write_png(filename.c_str(), width, height, 4, pixels.data(), width * 4);
        // }

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
    // 删除元素缓冲对象
    glDeleteBuffers(1, &EBO);
    // 销毁 GLFW 窗口
    glfwDestroyWindow(window);
    // 终止 GLFW 库
    glfwTerminate();

    // 返回程序的退出状态码
    return 0;
}

// 主要优化点说明
// 生成多条连续的折线:
// 函数 generateRandomMixedLines 替换了原来的 generateRandomMixedLine,增加了 numLines 参数,用于指定生成多少条独立的折线.
// 每条折线是连续的,直线段和贝塞尔曲线段通过索引连续连接.
// 顶点和索引存储优化:
// 顶点数据 (vertices) 只存储一次,避免重复.每个顶点包含 (x, y, dAccLen).
// 索引数据 (lineIndices) 使用 std::vector<std::vector<unsigned int>> 存储,每条折线的索引单独保存在一个子数组中,确保连续性.
// 在生成顶点时,索引是连续添加的(而不是成对添加),适配 GL_LINE_STRIP 的绘制方式.
// 使用 GL_LINE_STRIP 和 glMultiDrawElements:
// 使用 glMultiDrawElements 替代 glDrawElements,以支持绘制多条独立的折线.
// 为每条折线计算起始索引位置 (firsts) 和索引数量 (counts),然后一次性绘制所有折线.
// GL_LINE_STRIP 确保每条折线的顶点按顺序连续绘制,形成完整的折线.
// EBO 数据处理:
// 将 shapeIndices 中的所有索引展平为一个一维数组 (flatIndices),上传到 EBO.
// glMultiDrawElements 使用偏移量和计数来区分每条折线.
// 运行与验证
// 编译运行:确保 GLFW、GLAD、GLM 和 stb_image_write.h 已正确配置.
// 效果:程序将生成 6 条独立的随机折线,每条折线由直线段和贝塞尔曲线段混合组成,显示为蓝色虚线.
// 缩放:使用鼠标滚轮可以缩放视图,虚线长度会根据缩放比例动态调整.
// 输出图像:当前代码中图像保存功能被禁用(if (i % 4000 == 0 && false)),如需启用,可将 false 改为 true.
// 进一步改进建议
// 颜色区分:为每条折线设置不同的颜色,可以通过在顶点数据中添加颜色属性或使用 Uniform 变量实现.
// 性能优化:如果折线数量非常多,考虑将顶点和索引数据改为动态缓冲(GL_DYNAMIC_DRAW),支持实时更新.
// 断开折线:如果需要绘制不连续的折线,可以在 lineIndices 中插入特殊标记(如 GL_UNSIGNED_INT 的最大值),并处理绘制逻辑.