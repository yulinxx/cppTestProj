// 运行程序后，显示随机生成的三阶贝塞尔曲线，
// 同时会在曲线上标记出转折点（红色矩形）、最大曲率点（紫色 X 形状）、
// 与起点近似共线的最近点（绿色 X 形状）和最远拐角点（较大的绿色 X 形状）。

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "bezier/bezier.h"
#include <vector>
#include <iostream>
#include <random>
#include <limits>
#include <cmath>

// 顶点着色器源码
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// 片段着色器源码，添加一个 uniform 变量来控制颜色
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 u_Color;
void main() {
    FragColor = u_Color;
}
)";

/**
 * @brief 创建并编译着色器
 *
 * 该函数接受着色器源代码和着色器类型作为参数，创建并编译着色器。
 * 如果编译失败，会输出错误信息。
 *
 * @param source 着色器源代码
 * @param type 着色器类型，如 GL_VERTEX_SHADER 或 GL_FRAGMENT_SHADER
 * @return unsigned int 编译好的着色器对象的 ID
 */
unsigned int createShader(const char* source, GLenum type)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

/**
 * @brief 创建并链接着色器程序
 *
 * 该函数接受顶点着色器和片段着色器的源代码作为参数，创建并链接着色器程序。
 * 链接完成后，会删除不再需要的着色器对象。
 * 如果链接失败，会输出错误信息。
 *
 * @param vertexSource 顶点着色器源代码
 * @param fragmentSource 片段着色器源代码
 * @return unsigned int 链接着色器程序的 ID
 */
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    unsigned int vertexShader = createShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = createShader(fragmentSource, GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

/**
 * @brief 创建并初始化 VAO 和 VBO
 *
 * 该函数接受顶点数据、VAO 和 VBO 的引用作为参数，创建并初始化 VAO 和 VBO。
 * 将顶点数据存储到 VBO 中，并设置顶点属性指针。
 *
 * @param vertices 顶点数据
 * @param VAO 顶点数组对象的引用
 * @param VBO 顶点缓冲对象的引用
 */
void createVAOAndVBO(const std::vector<float>& vertices, unsigned int& VAO, unsigned int& VBO)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/**
 * @brief 清理 OpenGL 资源
 */
void cleanupOpenGLResources(unsigned int VAO, unsigned int VBO, unsigned int shaderProgram)
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

/**
 * @brief 计算二阶导数为零的点
 *
 * 该函数接受一个三阶贝塞尔曲线对象和一个误差范围作为参数，计算曲线上二阶导数为零的点。
 * 通过遍历曲线上的点，判断二阶导数的 x 和 y 分量是否小于误差范围。
 *
 * @param curve 三阶贝塞尔曲线对象
 * @param epsilon 误差范围，默认为 0.03
 * @return std::vector<double> 包含二阶导数为零的点的参数 t 的向量
 */
std::vector<double> findTurningPoints(const bezier::Bezier<3>& curve, double epsilon = 0.03)
{
    bezier::Bezier<1> secondDerivative = curve.derivative().derivative();
    std::vector<double> turningPoints;

    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        if (std::abs(secondDerivative.valueAt(t).x) < epsilon ||
            std::abs(secondDerivative.valueAt(t).y) < epsilon)
        {
            turningPoints.push_back(t);
        }
    }

    return turningPoints;
}

/**
 * @brief 在指定点处，生成小矩形的顶点数据
 *
 * 该函数接受一个点和矩形的大小作为参数，生成以该点为中心的小矩形的顶点数据。
 *
 * @param pt 矩形的中心点
 * @param size 矩形的边长
 * @return std::vector<float> 包含矩形顶点数据的向量
 */
std::vector<float> genRectangle(const bezier::Point& pt, float size)
{
    return {
        static_cast<float>(pt.x - size / 2), static_cast<float>(pt.y - size / 2),
        static_cast<float>(pt.x + size / 2), static_cast<float>(pt.y - size / 2),
        static_cast<float>(pt.x + size / 2), static_cast<float>(pt.y + size / 2),
        static_cast<float>(pt.x - size / 2), static_cast<float>(pt.y + size / 2) };
}

/**
 * @brief 在指定点处，生成以此点为中心的 X 形状的顶点数据
 *
 * 该函数接受一个点和 X 形状的大小作为参数，生成以该点为中心的 X 形状的顶点数据。
 *
 * @param pt X 形状的中心点
 * @param size X 形状的大小
 * @return std::vector<float> 包含 X 形状顶点数据的向量
 */
std::vector<float> genX(const bezier::Point& pt, float size)
{
    return {
        static_cast<float>(pt.x - size / 2), static_cast<float>(pt.y - size / 2),
        static_cast<float>(pt.x + size / 2), static_cast<float>(pt.y + size / 2),
        static_cast<float>(pt.x - size / 2), static_cast<float>(pt.y + size / 2),
        static_cast<float>(pt.x + size / 2), static_cast<float>(pt.y - size / 2) };
}

bezier::Point findNearestLinePt(const bezier::Bezier<3>& curve, double threshold = 0.0174533)
{
    bezier::Point startPoint = curve.valueAt(0);
    bezier::Point prevPoint = startPoint;

    for (float t = 0.02f; t <= 1.0f; t += 0.01f)
    {
        bezier::Point currentPoint = curve.valueAt(t);

        // 计算向量
        double v1x = prevPoint.x - startPoint.x;
        double v1y = prevPoint.y - startPoint.y;
        double v2x = currentPoint.x - startPoint.x;
        double v2y = currentPoint.y - startPoint.y;

        // 计算叉积
        double crossProduct = v1x * v2y - v1y * v2x;

        // 计算向量长度的乘积
        double magnitudeProduct = std::sqrt(v1x * v1x + v1y * v1y) * std::sqrt(v2x * v2x + v2y * v2y);

        // 计算夹角的正弦值
        double sinAngle = std::abs(crossProduct) / magnitudeProduct;

        // 如果夹角的正弦值小于阈值，则认为近似共线
        if (sinAngle < std::sin(threshold))
        {
            return currentPoint;
        }

        prevPoint = currentPoint;
    }

    return bezier::Point(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
}

// bezier::Point findFarthestLinePt(const bezier::Bezier<3>& curve, double threshold = 0.0174533) {
//     bezier::Point startPoint = curve.valueAt(0);
//     bezier::Point prevPoint = startPoint;
//     bezier::Point farthestPoint = startPoint; // 初始化最远点为起点

//     for (float t = 0.02f; t <= 1.0f; t += 0.01f) {
//         bezier::Point currentPoint = curve.valueAt(t);

//         // 计算向量
//         double v1x = prevPoint.x - startPoint.x;
//         double v1y = prevPoint.y - startPoint.y;
//         double v2x = currentPoint.x - startPoint.x;
//         double v2y = currentPoint.y - startPoint.y;

//         // 计算叉积
//         double crossProduct = v1x * v2y - v1y * v2x;

//         // 计算向量长度的乘积
//         double magnitudeProduct = std::sqrt(v1x * v1x + v1y * v1y) * std::sqrt(v2x * v2x + v2y * v2y);

//         // 计算夹角的正弦值
//         double sinAngle = std::abs(crossProduct) / magnitudeProduct;

//         // 如果夹角的正弦值小于阈值，则认为近似共线
//         if (sinAngle < std::sin(threshold)) {
//             farthestPoint = currentPoint; // 更新最远点
//         } else {
//             // 如果不再共线，则返回当前找到的最远点
//             return farthestPoint;
//         }

//         prevPoint = currentPoint;
//     }

//     // 如果遍历完整个曲线，仍然共线，则返回最后一个点
//     return farthestPoint;
// }

// ... existing code ...

/**
 * @brief 计算曲率
 *
 * 该函数接受一个三阶贝塞尔曲线对象和一个参数 t 作为参数，计算曲线上 t 位置的曲率。
 * 通过计算一阶导数和二阶导数，然后使用曲率公式计算曲率。
 *
 * @param curve 三阶贝塞尔曲线对象
 * @param t 参数 t，范围为 [0, 1]
 * @return double 曲线上 t 位置的曲率
 */
double calculateCurvature(const bezier::Bezier<3>& curve, double t)
{
    // 计算曲线的一阶导数
    bezier::Bezier<2> firstDerivative = curve.derivative();
    // 计算曲线的二阶导数
    bezier::Bezier<1> secondDerivative = firstDerivative.derivative();

    // 计算在参数 t 处的一阶导数的值
    bezier::Point p1 = firstDerivative.valueAt(t);
    // 计算在参数 t 处的二阶导数的值
    bezier::Point p2 = secondDerivative.valueAt(t);

    // 计算曲率公式的分子部分，使用向量叉积的模
    double numerator = std::abs(p1.x * p2.y - p1.y * p2.x);
    // 计算曲率公式的分母部分，使用一阶导数向量的模的 1.5 次方
    double denominator = std::pow(p1.x * p1.x + p1.y * p1.y, 1.5);

    // 避免除零错误，如果分母为零，返回曲率为 0
    return denominator == 0 ? 0 : numerator / denominator;
}

/**
 * @brief 找到最大曲率点
 *
 * 该函数接受一个三阶贝塞尔曲线对象作为参数，遍历曲线上的点，找到曲率最大的点。
 *
 * @param curve 三阶贝塞尔曲线对象
 * @return bezier::Point 曲线上曲率最大的点
 */
bezier::Point findMaxCurvaturePoint(const bezier::Bezier<3>& curve)
{
    // 初始化最大曲率为 0
    double maxCurvature = 0;
    // 初始化最大曲率对应的参数 t 为 0
    double maxT = 0;

    // 遍历曲线上的点，参数 t 从 0 到 1，步长为 0.001
    for (double t = 0; t <= 1; t += 0.001)
    {
        // 计算当前参数 t 对应的曲线上的点的曲率
        double curvature = calculateCurvature(curve, t);
        // 如果当前曲率大于之前记录的最大曲率
        if (curvature > maxCurvature)
        {
            // 更新最大曲率为当前曲率
            maxCurvature = curvature;
            // 更新最大曲率对应的参数 t 为当前参数 t
            maxT = t;
        }
    }

    // 返回最大曲率对应的曲线上的点
    return curve.valueAt(maxT);
}

/**
 * @brief 找到最远的拐角点
 *
 * 该函数接受一个三阶贝塞尔曲线对象、曲率阈值和直线性阈值作为参数，
 * 找到曲线上满足曲率大于曲率阈值且从起点到该点近似直线的最远点。
 *
 * @param curve 三阶贝塞尔曲线对象
 * @param curvatureThreshold 曲率阈值，用于判断是否为拐角点
 * @param linearityThreshold 直线性阈值，用于判断从起点到当前点是否近似直线
 * @return bezier::Point 满足条件的最远点，如果没有找到则返回无效点
 */
bezier::Point findFarthestCornerPoint(const bezier::Bezier<3>& curve, double curvatureThreshold = 0.1, double linearityThreshold = 0.01)
{
    bezier::Point startPoint = curve.valueAt(0);
    bezier::Point farthestPoint = startPoint;
    double farthestDistance = 0;

    // 遍历曲线上的点
    for (float t = 0.01f; t <= 1.0f; t += 0.01f)
    {
        bezier::Point currentPoint = curve.valueAt(t);

        // 计算曲率
        double curvature = calculateCurvature(curve, t);

        // 检查是否为局部最大曲率点
        if (curvature > curvatureThreshold)
        {
            // 验证从起点到当前点的直线性
            double dx = currentPoint.x - startPoint.x;
            double dy = currentPoint.y - startPoint.y;
            double distance = std::sqrt(dx * dx + dy * dy);

            // 计算从起点到当前点的向量
            double vx = currentPoint.x - startPoint.x;
            double vy = currentPoint.y - startPoint.y;

            // 计算前一个点到当前点的向量
            bezier::Point prevPoint = curve.valueAt(t - 0.01f);
            double pvx = currentPoint.x - prevPoint.x;
            double pvy = currentPoint.y - prevPoint.y;

            // 计算向量叉积的绝对值
            double crossProduct = std::abs(vx * pvy - vy * pvx);

            // 如果当前点满足直线性且距离更远，则更新最远点
            if (distance > farthestDistance && crossProduct < linearityThreshold)
            {
                farthestPoint = currentPoint;
                farthestDistance = distance;
            }
        }
    }

    // 如果没有找到满足条件的点，返回无效点
    if (farthestDistance == 0)
    {
        return bezier::Point(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
    }

    return farthestPoint;
}

/**
 * @brief 主函数，程序入口
 *
 * 该函数是程序的入口，负责初始化 GLFW 和 GLAD，创建窗口和着色器程序，生成贝塞尔曲线和相关数据，
 * 创建 VAO 和 VBO，进入渲染循环，最后清理资源。
 *
 * @return int 程序退出状态码
 */
int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1800, 1600, "Cubic Bezier Curve", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 创建并链接着色器程序
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    GLint colorUniformLocation = glGetUniformLocation(shaderProgram, "u_Color");

    // 随机生成三阶贝塞尔曲线的四个控制点
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);

    std::vector<bezier::Point> controlPoints = {
        {static_cast<float>(dis(gen)), static_cast<float>(dis(gen))},
        {static_cast<float>(dis(gen)), static_cast<float>(dis(gen))},
        {static_cast<float>(dis(gen)), static_cast<float>(dis(gen))},
        {static_cast<float>(dis(gen)), static_cast<float>(dis(gen))} };

    bezier::Bezier<3> curve(controlPoints);

    // 创建曲线上的点
    std::vector<float> curvePts;
    for (float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        bezier::Point p = curve.valueAt(t);
        curvePts.push_back(static_cast<float>(p.x));
        curvePts.push_back(static_cast<float>(p.y));
    }

    // 找到最大曲率点
    bezier::Point maxCurvaturePoint = findMaxCurvaturePoint(curve);
    std::vector<float> maxCurvatureXPts = genX(maxCurvaturePoint, 0.02f);

    // 计算转弯点
    std::vector<double> turningPoints = findTurningPoints(curve);
    std::vector<float> allTurnRectPts;
    for (double t : turningPoints)
    {
        bezier::Point turningPoint = curve.valueAt(t);
        std::vector<float> rectPts = genRectangle(turningPoint, 0.03f);
        allTurnRectPts.insert(allTurnRectPts.end(), rectPts.begin(), rectPts.end());
    }

    // 找到与起点近似在一条直线上的最近一点
    // 当 threshold = 0.3 时，意味着当斜率变化小于 0.3 时，对应的角度变化约为 16.7 度
    // 10 度对应的弧度值是: 0.174533
    // 30 度对应的弧度值是: 0.523599
    // 45 度对应的弧度值是: 0.785398
    // 60 度对应的弧度值是: 1.0472
    // 90 度对应的弧度值是: 1.5708
    bezier::Point nearestPt = findNearestLinePt(curve, 0.5);
    std::cout << "nearestPt: " << nearestPt.x << " " << nearestPt.y << std::endl;
    std::vector<float> vecXPts = genX(nearestPt, 0.02f);

    bezier::Point farestPt = findFarthestCornerPoint(curve, 0.1, 0.05);
    std::cout << "farestPt: " << farestPt.x << " " << farestPt.y << std::endl;
    std::vector<float> farXPts = genX(farestPt, 0.05f);

    vecXPts.insert(vecXPts.end(), farXPts.begin(), farXPts.end());

    // 添加上起点
    auto startPt = curve.valueAt(0);
    std::vector<float> startXPt = genX(startPt, 0.01f);
    vecXPts.insert(vecXPts.end(), startXPt.begin(), startXPt.end());

    // 创建 VAO 和 VBO
    unsigned int curveVAO, curveVBO;
    createVAOAndVBO(curvePts, curveVAO, curveVBO);

    unsigned int rectangleVAO, rectangleVBO;
    createVAOAndVBO(allTurnRectPts, rectangleVAO, rectangleVBO);

    unsigned int nearestVAO, nearestVBO;
    createVAOAndVBO(vecXPts, nearestVAO, nearestVBO);

    unsigned int maxCurvatureVAO, maxCurvatureVBO;
    createVAOAndVBO(maxCurvatureXPts, maxCurvatureVAO, maxCurvatureVBO);

    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 绘制贝塞尔曲线
        glUniform4f(colorUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(curveVAO);
        glDrawArrays(GL_LINE_STRIP, 0, curvePts.size() / 2);

        // 绘制转弯处的矩形（使用 glMultiDrawArrays）
        if (!turningPoints.empty())
        {
            glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f); // 红色
            glBindVertexArray(rectangleVAO);

            std::vector<GLint> startIndices(turningPoints.size());
            std::vector<GLsizei> vertexCounts(turningPoints.size(), 4);

            for (size_t i = 0; i < turningPoints.size(); ++i)
            {
                startIndices[i] = static_cast<GLint>(i * 4); // 每个矩形有 4 个顶点
            }

            glMultiDrawArrays(GL_LINE_LOOP, startIndices.data(), vertexCounts.data(), turningPoints.size());
        }

        // 绘制最近的点
        glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
        glBindVertexArray(nearestVAO);
        glDrawArrays(GL_LINES, 0, vecXPts.size() / 2);

        // 绘制最大曲率点
        glUniform4f(colorUniformLocation, 1.0f, 0.0f, 1.0f, 1.0f);
        glBindVertexArray(maxCurvatureVAO);
        glDrawArrays(GL_LINES, 0, maxCurvatureXPts.size() / 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    cleanupOpenGLResources(curveVAO, curveVBO, shaderProgram);
    cleanupOpenGLResources(rectangleVAO, rectangleVBO, shaderProgram);
    cleanupOpenGLResources(nearestVAO, nearestVBO, shaderProgram);
    cleanupOpenGLResources(maxCurvatureVAO, maxCurvatureVBO, shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
// #include "bezier/bezier.h"
// #include <vector>
// #include <iostream>
// #include <random>
// #include <cmath>
//
//// 顶点着色器源码
// const char *vertexShaderSource = R"(
// #version 330 core
// layout (location = 0) in vec2 aPos;
// void main() {
//     gl_Position = vec4(aPos, 0.0, 1.0);
// }
//)";
//
//// 片段着色器源码，添加一个 uniform 变量来控制颜色
// const char *fragmentShaderSource = R"(
// #version 330 core
// out vec4 FragColor;
// uniform vec4 u_Color; // 添加一个 uniform 变量来控制颜色
// void main() {
//     FragColor = u_Color; // 使用 uniform 变量设置颜色
// }
//)";
//
//// 计算二阶导数为零的点
// std::vector<double> findTurningPoints(const bezier::Bezier<3> &curve, double epsilon = 0.03)
//{
//     bezier::Bezier<1> secondDerivative = curve.derivative().derivative();
//     std::vector<double> turningPoints;
//     // const double epsilon = 0.03; // 误差范围
//     for (double t = 0.0; t <= 1.0; t += 0.001)
//     {
//         if (std::abs(secondDerivative.valueAt(t).x) < epsilon ||
//             std::abs(secondDerivative.valueAt(t).y) < epsilon)
//         {
//             turningPoints.push_back(t);
//         }
//     }
//     return turningPoints;
// }
//
//// 在指定点处，生成小矩形的顶点数据
// std::vector<float> genRectange(const bezier::Point &pt, float size)
//{
//     std::vector<float> curvePts =
//         {
//             static_cast<float>(pt.x - size / 2),
//             static_cast<float>(pt.y - size / 2),
//             static_cast<float>(pt.x + size / 2),
//             static_cast<float>(pt.y - size / 2),
//             static_cast<float>(pt.x + size / 2),
//             static_cast<float>(pt.y + size / 2),
//             static_cast<float>(pt.x - size / 2),
//             static_cast<float>(pt.y + size / 2),
//         };
//     return curvePts;
// }
//
//// 在指定点处，生成以此点为中心的 X 形状的顶点数据
// std::vector<float> genX(const bezier::Point &pt, float size)
//{
//     std::vector<float> curvePts =
//         {
//             // 第一条线段的起点
//             static_cast<float>(pt.x - size / 2),
//             static_cast<float>(pt.y - size / 2),
//             // 第一条线段的终点
//             static_cast<float>(pt.x + size / 2),
//             static_cast<float>(pt.y + size / 2),
//             // 第二条线段的起点
//             static_cast<float>(pt.x - size / 2),
//             static_cast<float>(pt.y + size / 2),
//             // 第二条线段的终点
//             static_cast<float>(pt.x + size / 2),
//             static_cast<float>(pt.y - size / 2)};
//     return curvePts;
// }
//// 找到与起点近似在一条直线上的最近一点
// bezier::Point findNearestLinePt(const bezier::Bezier<3> &curve, double threshold)
//{
//     // 获取起点
//     bezier::Point startPoint = curve.valueAt(0);
//
//     // 初始化斜率
//     double prevSlope = 0.0;
//     bool firstPoint = true;
//
//     // 遍历曲线上的点
//     for (float t = 0.01f; t <= 1.0f; t += 0.01f)
//     {
//         bezier::Point currentPoint = curve.valueAt(t);
//
//         // 计算当前点与起点连线的斜率
//         double dx = currentPoint.x - startPoint.x;
//         double dy = currentPoint.y - startPoint.y;
//         double currentSlope = dx != 0 ? dy / dx : std::numeric_limits<double>::max();
//
//         // 如果是第一个点，记录斜率并继续
//         if (firstPoint)
//         {
//             prevSlope = currentSlope;
//             firstPoint = false;
//             continue;
//         }
//
//         // 计算斜率变化
//         double slopeChange = std::abs(currentSlope - prevSlope);
//
//         // 判断是否近似在一条直线上
//         if (slopeChange < threshold)
//         {
//             return currentPoint;
//         }
//
//         // 更新前一个斜率
//         prevSlope = currentSlope;
//     }
//
//     // 如果没有找到符合条件的点，返回起点
//     return startPoint;
// }
//
//// 计算曲率
// double calculateCurvature(const bezier::Bezier<3> &curve, double t)
//{
//     bezier::Bezier<2> firstDerivative = curve.derivative();
//     bezier::Bezier<1> secondDerivative = firstDerivative.derivative();
//
//     bezier::Point p1 = firstDerivative.valueAt(t);
//     bezier::Point p2 = secondDerivative.valueAt(t);
//
//     double numerator = std::abs(p1.x * p2.y - p1.y * p2.x);
//     double denominator = std::pow(p1.x * p1.x + p1.y * p1.y, 1.5);
//
//     if (denominator == 0)
//     {
//         return 0;
//     }
//
//     return numerator / denominator;
// }
//
//// 找到最大曲率点
// bezier::Point findMaxCurvaturePoint(const bezier::Bezier<3> &curve)
//{
//     double maxCurvature = 0;
//     double maxT = 0;
//
//     const double step = 0.001;
//     for (double t = 0; t <= 1; t += step)
//     {
//         double curvature = calculateCurvature(curve, t);
//         if (curvature > maxCurvature)
//         {
//             maxCurvature = curvature;
//             maxT = t;
//         }
//     }
//
//     return curve.valueAt(maxT);
// }
//
//// 在主函数中调用
// int main()
//{
//     // 初始化 GLFW
//     if (!glfwInit())
//     {
//         std::cerr << "Failed to initialize GLFW" << std::endl;
//         return -1;
//     }
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//     // 创建窗口
//     GLFWwindow *window = glfwCreateWindow(1800, 1600, "Cubic Bezier Curve", NULL, NULL);
//     if (!window)
//     {
//         std::cerr << "Failed to create GLFW window" << std::endl;
//         glfwTerminate();
//         return -1;
//     }
//     glfwMakeContextCurrent(window);
//
//     // 初始化 GLAD
//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//     {
//         std::cerr << "Failed to initialize GLAD" << std::endl;
//         glfwDestroyWindow(window);
//         glfwTerminate();
//         return -1;
//     }
//
//     // 创建并编译着色器
//     unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
//     glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//     glCompileShader(vertexShader);
//
//     unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//     glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//     glCompileShader(fragmentShader);
//
//     // 在主函数中，在渲染循环之前设置颜色的 uniform 位置
//     // 创建并链接着色器程序
//     unsigned int shaderProgram = glCreateProgram();
//     glAttachShader(shaderProgram, vertexShader);
//     glAttachShader(shaderProgram, fragmentShader);
//     glLinkProgram(shaderProgram);
//
//     // 删除不再需要的着色器对象
//     glDeleteShader(vertexShader);
//     glDeleteShader(fragmentShader);
//
//     // 获取 uniform 变量的位置
//     GLint colorUniformLocation = glGetUniformLocation(shaderProgram, "u_Color");
//
//     ///////////////////////////////
//     // 生成数据
//     // 随机生成三阶贝塞尔曲线的四个控制点
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_real_distribution<> dis(-1.0, 1.0); // 生成 -1.0 到 1.0 之间的随机数
//     std::vector<bezier::Point> controlPoints = {
//         {static_cast<float>(dis(gen)), static_cast<float>(dis(gen))}, // P0
//         {static_cast<float>(dis(gen)), static_cast<float>(dis(gen))}, // P1
//         {static_cast<float>(dis(gen)), static_cast<float>(dis(gen))}, // P2
//         {static_cast<float>(dis(gen)), static_cast<float>(dis(gen))}  // P3
//     };
//
//     // 创建三阶贝塞尔曲线
//     bezier::Bezier<3> curve(controlPoints);
//
//     // 创建曲线上的点
//     std::vector<float> curvePts;
//     for (float t = 0.0f; t <= 1.0f; t += 0.01f)
//     {
//         bezier::Point p = curve.valueAt(t);
//         curvePts.push_back(static_cast<float>(p.x)); // x 坐标
//         curvePts.push_back(static_cast<float>(p.y)); // y 坐标
//     }
//
//     // 找到最大曲率点
//     bezier::Point maxCurvaturePoint = findMaxCurvaturePoint(curve);
//     std::cout << "Max Curvature Point: (" << maxCurvaturePoint.x << ", " << maxCurvaturePoint.y << ")" << std::endl;
//     // 生成最大曲率点的 X 形状顶点数据
//     std::vector<float> maxCurvatureXPts = genX(maxCurvaturePoint, 0.02f);
//
//     // 计算转弯点
//     std::vector<double> turningPoints = findTurningPoints(curve);
//     // 输出转折点的坐标
//     for (double t : turningPoints)
//     {
//         bezier::Point inflectionPoint = curve.valueAt(t);
//         std::cout << "Inflection Point at t = " << t << "\t: ("
//                   << inflectionPoint.x << ", " << inflectionPoint.y << ")\n"
//                   << std::endl;
//     }
//     std::cout << "-- turningPoints:" << turningPoints.size() << std::endl;
//     // 转弯处的矩形
//     std::vector<float> allTurnRectPts;
//     for (double t : turningPoints)
//     {
//         bezier::Point turningPoint = curve.valueAt(t);
//         std::vector<float> recPts = genRectange(turningPoint, 0.03f);
//         allTurnRectPts.insert(allTurnRectPts.end(), recPts.begin(), recPts.end());
//     }
//
//     // 找到与起点近似在一条直线上的最近一点
//     auto nearestPt = findNearestLinePt(curve, 0.3);
//     std::cout << "-- findNearestLinePt:" << nearestPt.x << " " << nearestPt.y << std::endl;
//     std::vector<float> vecXPts = genX(nearestPt, 0.02f);
//     std::cout << "vecXPts:" << vecXPts.size() << std::endl;
//
//     auto startPt = curve.valueAt(0);
//     auto len = (nearestPt - startPt).length();
//     std::cout << "len:" << len << std::endl;
//     std::vector<float> startXPt = genX(startPt, 0.01f);
//     vecXPts.insert(vecXPts.end(), startXPt.begin(), startXPt.end());
//
//     ///////////////////////////////
//     // 创建 curveVBO 和 curveVAO
//     unsigned int curveVBO, curveVAO;
//     glGenVertexArrays(1, &curveVAO);
//     glGenBuffers(1, &curveVBO);
//
//     glBindVertexArray(curveVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
//     glBufferData(GL_ARRAY_BUFFER, curvePts.size() * sizeof(float), curvePts.data(), GL_STATIC_DRAW);
//
//     // 设置顶点属性指针
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
//     glEnableVertexAttribArray(0);
//
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//
//     // 创建矩形的 curveVBO 和 curveVAO
//     unsigned int rectangleVBO, rectangleVAO;
//     glGenVertexArrays(1, &rectangleVAO);
//     glGenBuffers(1, &rectangleVBO);
//
//     glBindVertexArray(rectangleVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
//     // 将所有矩形的顶点数据上传到缓冲区
//     glBufferData(GL_ARRAY_BUFFER, allTurnRectPts.size() * sizeof(float), allTurnRectPts.data(), GL_STATIC_DRAW);
//
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
//     glEnableVertexAttribArray(0);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//
//     // 创建最近点的 VBO 和 VAO
//     unsigned int nearestVBO, nearestVAO;
//     glGenVertexArrays(1, &nearestVAO);
//     glGenBuffers(1, &nearestVBO);
//
//     glBindVertexArray(nearestVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, nearestVBO);
//     // 将所有矩形的顶点数据上传到缓冲区
//     glBufferData(GL_ARRAY_BUFFER, vecXPts.size() * sizeof(float), vecXPts.data(), GL_STATIC_DRAW);
//
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
//     glEnableVertexAttribArray(0);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//
//     // 创建最大曲率点的 VBO 和 VAO
//     unsigned int maxCurvatureVBO, maxCurvatureVAO;
//     glGenVertexArrays(1, &maxCurvatureVAO);
//     glGenBuffers(1, &maxCurvatureVBO);
//
//     glBindVertexArray(maxCurvatureVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, maxCurvatureVBO);
//     glBufferData(GL_ARRAY_BUFFER, maxCurvatureXPts.size() * sizeof(float), maxCurvatureXPts.data(), GL_STATIC_DRAW);
//
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
//     glEnableVertexAttribArray(0);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//
//     // 使用着色器程序
//     glUseProgram(shaderProgram);
//
//     // 渲染循环
//     while (!glfwWindowShouldClose(window))
//     {
//         glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 黑色背景
//         glClear(GL_COLOR_BUFFER_BIT);
//
//         // 绘制贝塞尔曲线，设置曲线颜色为白色
//         {
//             glUniform4f(colorUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f); // 设置为白色
//             glBindVertexArray(curveVAO);
//             glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
//             glDrawArrays(GL_LINE_STRIP, 0, curvePts.size() / 2);
//         }
//
//         // 绘制转弯处的矩形，设置矩形颜色为红色
//         {
//             glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f); // 设置为红色
//             glBindVertexArray(rectangleVAO);
//             glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
//
//             // 使用 glDrawArrays 绘制每个矩形
//             if (0)
//             {
//                 for (size_t i = 0; i < turningPoints.size(); ++i)
//                 {
//                     // 计算当前矩形的起始索引
//                     size_t startIndex = i * 4; // 每个矩形有 4 个顶点
//                     // 绘制当前矩形
//                     glDrawArrays(GL_LINE_LOOP, startIndex, 4);
//                 }
//             }
//             else
//             {
//                 // 使用 glMultiDrawArrays 一次性绘制所有矩形
//                 // 准备起始索引数组和图元数量数组
//                 std::vector<GLint> startIndices;
//                 std::vector<GLsizei> vertexCounts;
//
//                 for (size_t i = 0; i < turningPoints.size(); ++i)
//                 {
//                     // 计算当前矩形的起始索引
//                     startIndices.push_back(static_cast<GLint>(i * 4));
//                     // 每个矩形有 4 个顶点
//                     vertexCounts.push_back(4);
//                 }
//
//                 // 使用 glMultiDrawArrays 一次性绘制所有矩形
//                 glMultiDrawArrays(GL_LINE_LOOP, startIndices.data(), vertexCounts.data(), static_cast<GLsizei>(turningPoints.size()));
//             }
//
//             // 绘制最近的点，设置曲线颜色为绿色
//             {
//                 // glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
//                 // glBindVertexArray(nearestVAO);
//                 // glBindBuffer(GL_ARRAY_BUFFER, nearestVBO);
//                 // glDrawArrays(GL_LINES, 0, vecXPts.size() / 2);
//
//                 glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
//                 glBindVertexArray(nearestVAO);
//                 glBindBuffer(GL_ARRAY_BUFFER, nearestVBO);
//
//                 // 准备起始索引数组和图元数量数组
//                 std::vector<GLint> startIndices;
//                 std::vector<GLsizei> vertexCounts;
//
//                 for (size_t i = 0; i < vecXPts.size(); ++i)
//                 {
//                     startIndices.push_back(static_cast<GLint>(i * 4));
//                     vertexCounts.push_back(4);
//                 }
//
//                 // 使用 glMultiDrawArrays 一次性绘制所有 X 形状
//                 glMultiDrawArrays(GL_LINES, startIndices.data(), vertexCounts.data(), static_cast<GLsizei>(startIndices.size()));
//             }
//
//             // 绘制最大曲率点的 X 形状，设置曲线颜色为蓝色
//             {
//
//                 glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
//                 glBindVertexArray(maxCurvatureVAO);
//                 glBindBuffer(GL_ARRAY_BUFFER, maxCurvatureVBO);
//
//                 // 准备起始索引数组和图元数量数组
//                 std::vector<GLint> startIndices;
//                 std::vector<GLsizei> vertexCounts;
//
//                 for (size_t i = 0; i < maxCurvatureXPts.size(); ++i)
//                 {
//                     startIndices.push_back(static_cast<GLint>(i * 4));
//                     vertexCounts.push_back(4);
//                 }
//
//                 // 使用 glMultiDrawArrays 一次性绘制所有 X 形状
//                 glMultiDrawArrays(GL_LINES, startIndices.data(), vertexCounts.data(), static_cast<GLsizei>(startIndices.size()));
//             }
//         }
//
//         // 交换缓冲区并处理事件
//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }
//
//     // 清理资源
//     glDeleteVertexArrays(1, &curveVAO);
//     glDeleteBuffers(1, &curveVBO);
//     glDeleteVertexArrays(1, &rectangleVAO);
//     glDeleteBuffers(1, &rectangleVBO);
//     glDeleteProgram(shaderProgram);
//     glfwDestroyWindow(window);
//     glfwTerminate();
//
//     return 0;
// }