/*
此文件是一个使用 OpenGL 和 GLFW 库绘制 B 样条曲线的 C++ 程序。程序的主要功能包括：

1. 定义 B 样条曲线的数据结构，包含控制点、节点向量、权重等信息。
2. 计算 B 样条曲线的基函数和曲线上的点。
3. 使用 OpenGL 着色器程序渲染 B 样条曲线、控制多边形和控制点。
运行此程序前，请确保已经正确安装并配置了 GLFW 和 GLAD 库。
*/

// 引入 GLAD 库，用于加载 OpenGL 函数
#include <glad/glad.h>
// 引入 GLFW 库，用于创建窗口和处理输入事件
#include <GLFW/glfw3.h>
// 引入标准输入输出流库
#include <iostream>
// 引入标准向量库
#include <vector>
// 引入标准字符串库
#include <string>

/**
 * @brief 二维向量点结构体，用于表示平面上的点。
 */
struct VecPt
{
    VecPt(double x_ = 0.0, double y_ = 0.0) : x(x_), y(y_)
    {
    }
};

/**
 * @brief B 样条曲线结构体，包含 B 样条曲线的相关信息。
 */
struct Bspline
{
    // B 样条曲线的阶数
    int degree;
    // 节点向量
    std::vector<double> knots;
    // 权重向量
    std::vector<double> weights;
    // 控制点向量
    std::vector<VecPt> controlPts;
    // 基点
    VecPt basePoint;
};

/**
 * @brief 计算 B 样条基函数的值。
 * @param i 基函数的索引。
 * @param k 基函数的阶数。
 * @param t 参数值。
 * @param knots 节点向量。
 * @return 基函数在参数 t 处的值。
 */
double computeBasisFunction(int i, int k, double t, const std::vector<double>& knots)
{
    if (k == 0)
    {
        // 修正边界条件：当 t 等于最后一个节点时，仍然返回 1
        if (i == knots.size() - 2 && t == knots[i + 1])
        {
            return 1.0;
        }
        return (t >= knots[i] && t < knots[i + 1]) ? 1.0 : 0.0;
    }
    // 计算分母 1
    double denom1 = knots[i + k] - knots[i];
    // 计算分母 2
    double denom2 = knots[i + k + 1] - knots[i + 1];
    // 计算第一项的值
    double term1 = (denom1 > 0) ? ((t - knots[i]) / denom1) * computeBasisFunction(i, k - 1, t, knots) : 0.0;
    // 计算第二项的值
    double term2 = (denom2 > 0) ? ((knots[i + k + 1] - t) / denom2) * computeBasisFunction(i + 1, k - 1, t, knots) : 0.0;
    return term1 + term2;
}

/**
 * @brief 计算 B 样条曲线上的点（强制最后一个点为最后一个控制点）。
 * @param spline B 样条曲线结构体。
 * @param numPoints 采样点的数量。
 * @return 包含 B 样条曲线上点的向量。
 */
std::vector<VecPt> computeBSplinePoints(const Bspline& spline, int numPoints)
{
    std::vector<VecPt> curvePoints;
    // 控制点数量 - 1
    int n = spline.controlPts.size() - 1;
    // B 样条曲线的阶数
    int k = spline.degree;

    if (spline.knots.size() < n + k + 2)
    {
        std::cerr << "Invalid knot vector" << std::endl;
        return curvePoints;
    }

    // 参数 t 的最小值
    double tMin = spline.knots[k];
    // 参数 t 的最大值
    double tMax = spline.knots[n + 1];
    // 参数 t 的步长
    double step = (tMax - tMin) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i)
    {
        // 计算当前参数 t 的值
        double t = tMin + i * step;
        // 初始化当前点的坐标
        VecPt point(0.0, 0.0);

        // 强制最后一个点为最后一个控制点
        if (i == numPoints - 1)
        {
            point = spline.controlPts.back();
        }
        else
        {
            for (int j = 0; j <= n; ++j)
            {
                // 计算基函数的值
                double basis = computeBasisFunction(j, k, t, spline.knots);
                // 计算当前点的 x 坐标
                point.x += basis * spline.controlPts[j].x;
                // 计算当前点的 y 坐标
                point.y += basis * spline.controlPts[j].y;
            }
        }
        // 将当前点添加到曲线点向量中
        curvePoints.push_back(point);
    }

    // 打印最后一个点
    if (!curvePoints.empty())
    {
        std::cout << "Last point: (" << curvePoints.back().x << ", " << curvePoints.back().y << ")" << std::endl;
    }
    return curvePoints;
}

// GLSL 顶点着色器代码
const char* vertexShaderSource = R"(
#version 460 core
// 顶点属性，位置坐标
layout (location = 0) in vec2 aPos;
void main() {
    // 设置顶点的位置
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// GLSL 片段着色器代码
const char* fragmentShaderSource = R"(
#version 460 core
// 输出的片段颜色
out vec4 FragColor;
// 统一变量，颜色
uniform vec3 color;
void main() {
    // 设置片段的颜色
    FragColor = vec4(color, 1.0);
}
)";

/**
 * @brief 编译着色器。
 * @param type 着色器类型（顶点着色器或片段着色器）。
 * @param source 着色器源代码。
 * @return 编译后的着色器对象 ID。
 */
GLuint compileShader(GLenum type, const char* source)
{
    // 创建着色器对象
    GLuint shader = glCreateShader(type);
    // 指定着色器源代码
    glShaderSource(shader, 1, &source, nullptr);
    // 编译着色器
    glCompileShader(shader);

    GLint success;
    // 获取编译状态
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        // 获取编译错误信息
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

/**
 * @brief 创建着色器程序。
 * @return 着色器程序对象 ID。
 */
GLuint createShaderProgram()
{
    // 编译顶点着色器
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    // 编译片段着色器
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // 创建着色器程序对象
    GLuint program = glCreateProgram();
    // 将顶点着色器附加到着色器程序
    glAttachShader(program, vertexShader);
    // 将片段着色器附加到着色器程序
    glAttachShader(program, fragmentShader);
    // 链接着色器程序
    glLinkProgram(program);

    GLint success;
    // 获取链接状态
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        // 获取链接错误信息
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    // 删除已附加的着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置 OpenGL 版本为 4.6
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    // 使用核心配置文件
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建 GLFW 窗口
    GLFWwindow* window = glfwCreateWindow(1280, 720, "B-Spline Curve", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        // 终止 GLFW
        glfwTerminate();
        return -1;
    }
    // 设置当前上下文为创建的窗口
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        // 终止 GLFW
        glfwTerminate();
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

    // 创建着色器程序
    GLuint shaderProgram = createShaderProgram();

    // 设置 B 样条曲线数据
    Bspline spline;
    // 设置基点
    spline.basePoint = VecPt(0, 0);
    // 设置 B 样条曲线的阶数
    spline.degree = 3;
    // 设置控制点
    spline.controlPts = { VecPt(-0.8, -0.8), VecPt(-0.4, 0.8), VecPt(0.4, -0.8), VecPt(0.8, 0.8) };
    // 设置节点向量
    spline.knots = { 0, 0, 0, 0, 1, 1, 1, 1 }; // 均匀节点向量
    // 设置权重向量
    spline.weights = { 1, 1, 1, 1 };           // 等权重

    // 计算曲线点
    auto points = computeBSplinePoints(spline, 200); // 200 个采样点
    // 打印曲线点的数量
    std::cout << "Number of points: " << points.size() << std::endl;

    // 设置曲线缓冲区
    GLuint curveVAO, curveVBO;
    // 生成顶点数组对象
    glGenVertexArrays(1, &curveVAO);
    // 生成顶点缓冲对象
    glGenBuffers(1, &curveVBO);

    // 绑定顶点数组对象
    glBindVertexArray(curveVAO);
    // 绑定顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
    // 将曲线点数据复制到顶点缓冲对象
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(VecPt), points.data(), GL_STATIC_DRAW);
    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, sizeof(VecPt), (void*)0);
    // 启用顶点属性
    glEnableVertexAttribArray(0);
    // 解绑顶点数组对象
    glBindVertexArray(0);

    // 设置控制点缓冲区
    GLuint controlVAO, controlVBO;
    // 生成顶点数组对象
    glGenVertexArrays(1, &controlVAO);
    // 生成顶点缓冲对象
    glGenBuffers(1, &controlVBO);

    // 绑定顶点数组对象
    glBindVertexArray(controlVAO);
    // 绑定顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, controlVBO);
    // 将控制点数据复制到顶点缓冲对象
    glBufferData(GL_ARRAY_BUFFER, spline.controlPts.size() * sizeof(VecPt), spline.controlPts.data(), GL_STATIC_DRAW);
    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, sizeof(VecPt), (void*)0);
    // 启用顶点属性
    glEnableVertexAttribArray(0);
    // 解绑顶点数组对象
    glBindVertexArray(0);

    // 设置控制多边形缓冲区（连接控制点的折线）
    GLuint polylineVAO, polylineVBO;
    // 生成顶点数组对象
    glGenVertexArrays(1, &polylineVAO);
    // 生成顶点缓冲对象
    glGenBuffers(1, &polylineVBO);

    // 绑定顶点数组对象
    glBindVertexArray(polylineVAO);
    // 绑定顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, polylineVBO);
    // 将控制点数据复制到顶点缓冲对象
    glBufferData(GL_ARRAY_BUFFER, spline.controlPts.size() * sizeof(VecPt), spline.controlPts.data(), GL_STATIC_DRAW);
    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, sizeof(VecPt), (void*)0);
    // 启用顶点属性
    glEnableVertexAttribArray(0);
    // 解绑顶点数组对象
    glBindVertexArray(0);

    // 主渲染循环
    // 设置背景色
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
        // 清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);
        // 获取统一变量的位置
        GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

        // 绘制曲线（红色）
        // 设置颜色为红色
        glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
        // 绑定曲线的顶点数组对象
        glBindVertexArray(curveVAO);
        // 绘制曲线
        glDrawArrays(GL_LINE_STRIP, 0, points.size());
        // 解绑顶点数组对象
        glBindVertexArray(0);

        // 绘制控制多边形（绿色）
        // 设置颜色为绿色
        glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
        // 绑定控制多边形的顶点数组对象
        glBindVertexArray(polylineVAO);
        // 绘制控制多边形
        glDrawArrays(GL_LINE_STRIP, 0, spline.controlPts.size());
        // 解绑顶点数组对象
        glBindVertexArray(0);

        // 绘制控制点（黄色）
        // 设置颜色为黄色
        glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);
        // 绑定控制点的顶点数组对象
        glBindVertexArray(controlVAO);
        // 设置点的大小
        glPointSize(5.0f);
        // 绘制控制点
        glDrawArrays(GL_POINTS, 0, spline.controlPts.size());
        // 解绑顶点数组对象
        glBindVertexArray(0);

        // 交换前后缓冲区
        glfwSwapBuffers(window);
        // 处理输入事件
        glfwPollEvents();
    }

    // 清理资源
    // 删除曲线的顶点数组对象
    glDeleteVertexArrays(1, &curveVAO);
    // 删除曲线的顶点缓冲对象
    glDeleteBuffers(1, &curveVBO);
    // 删除控制点的顶点数组对象
    glDeleteVertexArrays(1, &controlVAO);
    // 删除控制点的顶点缓冲对象
    glDeleteBuffers(1, &controlVBO);
    // 删除控制多边形的顶点数组对象
    glDeleteVertexArrays(1, &polylineVAO);
    // 删除控制多边形的顶点缓冲对象
    glDeleteBuffers(1, &polylineVBO);
    // 删除着色器程序
    glDeleteProgram(shaderProgram);

    // 终止 GLFW
    glfwTerminate();
    return 0;
}