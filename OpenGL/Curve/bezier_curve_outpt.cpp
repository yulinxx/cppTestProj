// 使用OpenGL、GLAD和GLSL，生成一个由线段和三阶贝塞尔曲线构成的随机闭合图形
// 找到贝塞尔曲线的“向外凸”点（定义为距离图形中心最远的点），并在该点绘制一个“X”标记。

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <variant>
#include <cmath>
#include <iostream>
#include <random>

// 数据结构
struct Point
{
    float x, y;
};

struct Line
{
    Point start, end;
};

struct BezierCurve
{
    Point p0, p1, p2, p3;
};

using Edge = std::variant<Line, BezierCurve>;

// 窗口参数
const int WIDTH = 1200;
const int HEIGHT = 1200;

// 顶点着色器
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";

// 片段着色器
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
)";

// 随机数生成器
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dist(-0.8f, 0.8f);

// 计算贝塞尔曲线上的点
Point evalBezier(const BezierCurve& b, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    float mt = 1 - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;

    Point p;
    p.x = mt3 * b.p0.x + 3 * mt2 * t * b.p1.x + 3 * mt * t2 * b.p2.x + t3 * b.p3.x;
    p.y = mt3 * b.p0.y + 3 * mt2 * t * b.p1.y + 3 * mt * t2 * b.p2.y + t3 * b.p3.y;
    return p;
}

// 计算图形中心
Point calcShapeCenter(const std::vector<Edge>& edges)
{
    Point center{ 0, 0 };
    int count = 0;
    for (const auto& edge : edges)
    {
        std::visit([&center, &count](auto&& e) {
            if constexpr (std::is_same_v<std::decay_t<decltype(e)>, Line>)
            {
                center.x += e.start.x;
                center.y += e.start.y;
            }
            else
            {
                center.x += e.p0.x;
                center.y += e.p0.y;
            }
            count++;
            }, edge);
    }

    if (count > 0)
    {
        center.x /= count;
        center.y /= count;
    }

    return center;
}

// 找到贝塞尔曲线的“向外凸”点（距离中心最远）
Point findOutermostPoint(const BezierCurve& b, const Point& center, int samples = 100)
{
    float maxDistSq = 0.0f;
    float tMax = 0.0f;

    for (int i = 0; i <= samples; ++i)
    {
        float t = static_cast<float>(i) / samples;
        Point p = evalBezier(b, t);
        float dx = p.x - center.x;
        float dy = p.y - center.y;
        float distSq = dx * dx + dy * dy;
        if (distSq > maxDistSq)
        {
            maxDistSq = distSq;
            tMax = t;
        }
    }
    return evalBezier(b, tMax);
}

// 生成随机闭合图形
std::vector<Edge> generateRandomShape()
{
    std::vector<Edge> edges;
    Point startPoint{ dist(gen), dist(gen) };
    Point lastPoint = startPoint;
    const int numSegments = 4; // 2条线段 + 2条贝塞尔曲线

    for (int i = 0; i < numSegments - 1; ++i)
    {
        Point nextPoint{ dist(gen), dist(gen) };
        //if (i % 2 == 0)
        if (i == 0)
        { // 线段
            edges.push_back(Line{ lastPoint, nextPoint });
        }
        else
        { // 贝塞尔曲线
            Point p1{ dist(gen), dist(gen) };
            Point p2{ dist(gen), dist(gen) };
            edges.push_back(BezierCurve{ lastPoint, p1, p2, nextPoint });
        }
        lastPoint = nextPoint;
    }

    // 闭合图形：最后一条边连接到起点
    edges.push_back(Line{ lastPoint, startPoint });

    return edges;
}

// 将边转换为顶点数据
std::vector<float> edgesToVertices(const std::vector<Edge>& edges, int bezierSamples = 50)
{
    std::vector<float> vertices;
    for (const auto& edge : edges)
    {
        std::visit([&vertices, bezierSamples](auto&& e) {
            if constexpr (std::is_same_v<std::decay_t<decltype(e)>, Line>)
            {
                vertices.push_back(e.start.x); vertices.push_back(e.start.y);
                vertices.push_back(e.end.x);   vertices.push_back(e.end.y);
            }
            else
            {
                for (int i = 0; i <= bezierSamples; ++i)
                {
                    float t = static_cast<float>(i) / bezierSamples;
                    Point p = evalBezier(e, t);
                    vertices.push_back(p.x); vertices.push_back(p.y);
                }
            }
            }, edge);
    }
    return vertices;
}

// 创建“X”标记的顶点
std::vector<float> createXMarker(const Point& center, float size = 0.05f)
{
    std::vector<float> vertices = {
        center.x - size, center.y - size, // 左下
        center.x + size, center.y + size, // 右上
        center.x + size, center.y - size, // 右下
        center.x - size, center.y + size  // 左上
    };
    return vertices;
}

int main()
{
    // 初始化GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CAD Shape", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 编译着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 生成随机闭合图形
    std::vector<Edge> edges = generateRandomShape();
    std::vector<float> shapeVertices = edgesToVertices(edges);

    // 计算图形中心和最外凸点
    Point center = calcShapeCenter(edges);
    Point convexPoint{ 0, 0 };
    bool foundBezier = false;
    for (const auto& edge : edges)
    {
        if (std::holds_alternative<BezierCurve>(edge))
        {
            convexPoint = findOutermostPoint(std::get<BezierCurve>(edge), center);
            foundBezier = true;
            break; // 只处理第一条贝塞尔曲线
        }
    }
    std::vector<float> xMarkerVertices = createXMarker(convexPoint);

    // 设置VAO和VBO
    unsigned int VAO[2], VBO[2];
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);

    // 图形
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(float), shapeVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // X标记
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, xMarkerVertices.size() * sizeof(float), xMarkerVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 渲染循环
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 深灰色背景
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // 绘制图形（蓝色）
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f);
        glBindVertexArray(VAO[0]);
        int vertexOffset = 0;
        for (const auto& edge : edges)
        {
            if (std::holds_alternative<Line>(edge))
            {
                glDrawArrays(GL_LINES, vertexOffset, 2);
                vertexOffset += 2;
            }
            else
            {
                glDrawArrays(GL_LINE_STRIP, vertexOffset, 51); // 50采样点+起点
                vertexOffset += 51;
            }
        }

        // 绘制X标记（红色）
        if (foundBezier)
        {
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f);
            glBindVertexArray(VAO[1]);
            glDrawArrays(GL_LINES, 0, 4);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}