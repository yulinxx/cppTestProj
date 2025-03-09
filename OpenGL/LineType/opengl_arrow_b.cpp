// 使用几何着色器,绘制出一条有三阶贝塞尔曲线的  --->--- 线
// 能缩放以及拖动画面

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>  // 包含clamp函数
#include <cmath>      // 包含clamp函数

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 初始视图范围
constexpr float X = 4.0f;

// Vertex Shader: 传递控制点位置
const char* vertexShaderSource = R"(
#version 400 core
// 输入:顶点位置
layout(location = 0) in vec2 in_pos;
// 输出:传递给下一个阶段的顶点位置
out vec2 v_pos;

void main() {
    // 将输入的顶点位置传递给下一个阶段
    v_pos = in_pos;
}
)";

// Tessellation Control Shader: 设置细分级别
const char* tessControlShaderSource = R"(
#version 400 core
// 输出4个顶点
layout(vertices = 4) out;
// 细分级别统一变量
uniform float tessLevel = 10.0;
// 输入:从顶点着色器传递过来的顶点位置
in vec2 v_pos[];
// 输出:传递给细分评估着色器的顶点位置
out vec2 tc_pos[];

void main() {
    // 将输入的顶点位置传递给下一个阶段
    tc_pos[gl_InvocationID] = v_pos[gl_InvocationID];

    if (gl_InvocationID == 0) {
        if (gl_in.length() == 2) {  // 直线
            // 直线的细分级别设置为1
            gl_TessLevelOuter[0] = 1.0;
            gl_TessLevelOuter[1] = 1.0;
        } else {  // 曲线
            // 曲线的细分级别使用统一变量
            gl_TessLevelOuter[0] = tessLevel;
            gl_TessLevelOuter[1] = tessLevel;
        }
    }
}
)";

// Tessellation Evaluation Shader: 计算直线和曲线的点
const char* tessEvaluationShaderSource = R"(
#version 400 core
// 细分模式:等间距的孤立线段
layout(isolines, equal_spacing) in;
// 包含缩放和拖动的变换矩阵
uniform mat4 transform;
// 输入:从细分控制着色器传递过来的顶点位置
in vec2 tc_pos[];
// 输出:传递给几何着色器的位置
out vec2 tes_pos;

void main() {
    // 获取细分坐标
    float u = gl_TessCoord.x;

    vec4 pos;
    if (gl_in.length() == 2) {  // 直线
        // 线性插值计算直线上的点
        pos = mix(vec4(tc_pos[0], 0.0, 1.0), vec4(tc_pos[1], 0.0, 1.0), u);
    } else {  // 三阶贝塞尔曲线
        // 计算u的平方和立方
        float u2 = u * u;
        float u3 = u2 * u;
        // 计算1 - u的平方和立方
        float oneMinusU = 1.0 - u;
        float oneMinusU2 = oneMinusU * oneMinusU;
        float oneMinusU3 = oneMinusU2 * oneMinusU;

        // 三阶贝塞尔曲线公式计算曲线上的点
        pos = oneMinusU3 * vec4(tc_pos[0], 0.0, 1.0) +
              3.0 * oneMinusU2 * u * vec4(tc_pos[1], 0.0, 1.0) +
              3.0 * oneMinusU * u2 * vec4(tc_pos[2], 0.0, 1.0) +
              u3 * vec4(tc_pos[3], 0.0, 1.0);
    }

    // 应用缩放和拖动变换
    tes_pos = (transform * pos).xy;
    // 设置顶点位置
    gl_Position = transform * pos;
}
)";

// Geometry Shader: 生成线段和动态调整的箭头
const char* geometryShaderSource = R"(
#version 400 core
// 输入:线段
layout(lines) in;
// 输出:线段带,最多8个顶点
layout(line_strip, max_vertices = 8) out;
// 输入:从细分评估着色器传递过来的顶点位置
in vec2 tes_pos[];
// 输出:传递给片段着色器的位置
out vec2 geo_pos;
// 变换矩阵
uniform mat4 transform;
// 动态箭头密度
uniform float arrowDensity = 1.0;
// 动态箭头大小
uniform float arrowSize = 0.1;

void main() {
    // 获取线段的起点和终点
    vec2 p0 = tes_pos[0];
    vec2 p1 = tes_pos[1];
    // 计算线段的方向向量
    vec2 dir = normalize(p1 - p0);
    // 计算垂直于线段方向的向量
    vec2 perp = vec2(-dir.y, dir.x);
    // 计算线段的长度
    float len = length(p1 - p0);

    // 绘制线段
    vec4 p0_transformed = transform * vec4(p0, 0.0, 1.0);
    vec4 p1_transformed = transform * vec4(p1, 0.0, 1.0);
    // 设置起点位置
    gl_Position = p0_transformed;
    geo_pos = p0;
    // 发射顶点
    EmitVertex();
    // 设置终点位置
    gl_Position = p1_transformed;
    geo_pos = p1;
    // 发射顶点
    EmitVertex();
    // 结束图元
    EndPrimitive();

    // 动态调整箭头密度和大小
    // 限制密度,防止过密或过稀疏
    float baseDensity = clamp(0.5f / arrowDensity, 0.2f, 2.0f);
    // 最大箭头数,防止过密
    int maxArrows = 5;
    // 计算箭头数量,限制箭头数量
    int numArrows = min(int(floor(len / baseDensity)), maxArrows);
    // 至少一个箭头
    if (numArrows < 1) numArrows = 1;

    for (int i = 0; i < numArrows; ++i) {
        // 均匀分布箭头位置
        float t = (float(i) + 0.5) / float(numArrows);
        // 计算箭头位置
        vec2 arrowPos = mix(p0, p1, t);

        // 动态调整箭头大小,保持可见性
        float adjustedArrowSize = clamp(arrowSize / arrowDensity, 0.05f, 0.2f);
        // 计算箭头尖端位置
        vec2 tip = arrowPos + dir * adjustedArrowSize;
        // 计算箭头一个翼的位置
        vec2 wing1 = arrowPos - dir * adjustedArrowSize * 0.5 + perp * adjustedArrowSize * 0.5;
        // 计算箭头另一个翼的位置
        vec2 wing2 = arrowPos - dir * adjustedArrowSize * 0.5 - perp * adjustedArrowSize * 0.5;

        // 第一条线段:中心到上翼
        gl_Position = transform * vec4(arrowPos, 0.0, 1.0);
        geo_pos = arrowPos;
        EmitVertex();
        gl_Position = transform * vec4(wing1, 0.0, 1.0);
        geo_pos = wing1;
        EmitVertex();
        EndPrimitive();

        // 第二条线段:中心到下翼
        gl_Position = transform * vec4(arrowPos, 0.0, 1.0);
        geo_pos = arrowPos;
        EmitVertex();
        gl_Position = transform * vec4(wing2, 0.0, 1.0);
        geo_pos = wing2;
        EmitVertex();
        EndPrimitive();
    }
}
)";

// Fragment Shader: 绘制蓝色线条
const char* fragmentShaderSource = R"(
#version 400 core
// 输入:从几何着色器传递过来的位置
in vec2 geo_pos;
// 输出:片段颜色
out vec4 fragColor;
// 蓝色
uniform vec4 color = vec4(0.0, 0.0, 1.0, 1.0);

void main() {
    // 设置片段颜色为蓝色
    fragColor = color;
}
)";

/**
 * @brief 加载并编译所有着色器,链接成一个着色器程序
 * @return 着色器程序的ID
 */
GLuint loadShader()
{
    // 创建顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 指定顶点着色器的源代码
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    // 编译顶点着色器
    glCompileShader(vertexShader);

    // 创建细分控制着色器
    GLuint tessControlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
    // 指定细分控制着色器的源代码
    glShaderSource(tessControlShader, 1, &tessControlShaderSource, nullptr);
    // 编译细分控制着色器
    glCompileShader(tessControlShader);

    // 创建细分评估着色器
    GLuint tessEvaluationShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
    // 指定细分评估着色器的源代码
    glShaderSource(tessEvaluationShader, 1, &tessEvaluationShaderSource, nullptr);
    // 编译细分评估着色器
    glCompileShader(tessEvaluationShader);

    // 创建几何着色器
    GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    // 指定几何着色器的源代码
    glShaderSource(geometryShader, 1, &geometryShaderSource, nullptr);
    // 编译几何着色器
    glCompileShader(geometryShader);

    // 创建片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 指定片段着色器的源代码
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    // 编译片段着色器
    glCompileShader(fragmentShader);

    // 创建着色器程序
    GLuint shaderProgram = glCreateProgram();
    // 将顶点着色器附加到着色器程序
    glAttachShader(shaderProgram, vertexShader);
    // 将细分控制着色器附加到着色器程序
    glAttachShader(shaderProgram, tessControlShader);
    // 将细分评估着色器附加到着色器程序
    glAttachShader(shaderProgram, tessEvaluationShader);
    // 将几何着色器附加到着色器程序
    glAttachShader(shaderProgram, geometryShader);
    // 将片段着色器附加到着色器程序
    glAttachShader(shaderProgram, fragmentShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);

    // 删除不再需要的着色器
    glDeleteShader(vertexShader);
    glDeleteShader(tessControlShader);
    glDeleteShader(tessEvaluationShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// 路径结构体,包含控制点
struct Path
{
    // 直线 (2 点) + 曲线 (4 点)
    std::vector<glm::vec2> controlPoints;
};

/**
 * @brief 生成一条包含直线和三阶贝塞尔曲线的路径
 * @param paths 存储生成路径的向量
 */
void generatePath(std::vector<Path>& paths)
{
    // 仅生成一条路径
    paths.resize(1);
    // 获取路径引用
    Path& path = paths[0];

    // 直线部分:从 (-X, 0) 到 (-X/2, 0)
    path.controlPoints.push_back(glm::vec2(-X, 0.0f));
    path.controlPoints.push_back(glm::vec2(-X / 2.0f, 0.0f));

    // 三阶贝塞尔曲线:从 (-X/2, 0) 到 (X, 0)
    path.controlPoints.push_back(glm::vec2(-X / 2.0f, 0.0f));  // 起始点
    path.controlPoints.push_back(glm::vec2(-X / 4.0f, X / 2.0f));  // 控制点 1
    path.controlPoints.push_back(glm::vec2(X / 2.0f, -X / 2.0f));  // 控制点 2
    path.controlPoints.push_back(glm::vec2(X, 0.0f));  // 终点
}

// 缩放因子
float zoomFactor = 1.0f;
// 拖动偏移
glm::vec2 panOffset(0.0f, 0.0f);
// 是否正在拖动
bool isDragging = false;
// 上次鼠标位置
double lastX, lastY;
// 窗口宽度
int windowWidth = 800;
// 窗口高度
int windowHeight = 600;

/**
 * @brief 窗口大小改变时的回调函数
 * @param window 窗口句柄
 * @param width 新的窗口宽度
 * @param height 新的窗口高度
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // 更新窗口宽度和高度
    windowWidth = width;
    windowHeight = height;
    // 设置视口大小
    glViewport(0, 0, width, height);
}

/**
 * @brief 鼠标滚动时的回调函数
 * @param window 窗口句柄
 * @param xoffset 鼠标滚动的x偏移量
 * @param yoffset 鼠标滚动的y偏移量
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // 根据鼠标滚动的y偏移量更新缩放因子
    zoomFactor += float(yoffset) * 0.1f;
    // 限制缩放范围在0.1到5.0之间
    zoomFactor = std::max(0.1f, std::min(5.0f, zoomFactor));
}

/**
 * @brief 鼠标按钮事件的回调函数
 * @param window 窗口句柄
 * @param button 鼠标按钮
 * @param action 按钮动作(按下或释放)
 * @param mods 修饰键状态
 */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            // 按下中键,开始拖动
            isDragging = true;
            // 获取当前鼠标位置
            glfwGetCursorPos(window, &lastX, &lastY);
        }
        else if (action == GLFW_RELEASE)
        {
            // 释放中键,停止拖动
            isDragging = false;
        }
    }
}

/**
 * @brief 鼠标光标移动时的回调函数
 * @param window 窗口句柄
 * @param xpos 鼠标光标的x位置
 * @param ypos 鼠标光标的y位置
 */
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isDragging)
    {
        // 计算鼠标移动的偏移量
        double dx = xpos - lastX;
        double dy = ypos - lastY;
        // 拖动敏感度随缩放调整
        float scale = 0.01f / zoomFactor;
        // 更新拖动偏移量
        panOffset.x += dx * scale;
        // y 方向反向(OpenGL 坐标系)
        panOffset.y -= dy * scale;
        // 更新上次鼠标位置
        lastX = xpos;
        lastY = ypos;
    }
}

int main()
{
    // 初始化GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置GLFW窗口的OpenGL版本和配置
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);  // 窗口最大化

    // 创建GLFW窗口
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Line with Arrow (Straight + Bezier)", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        // 终止GLFW
        glfwTerminate();
        return -1;
    }
    // 将窗口设置为当前上下文
    glfwMakeContextCurrent(window);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 设置回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // 加载着色器程序
    GLuint shaderProgram = loadShader();
    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 存储路径的向量
    std::vector<Path> paths;
    // 生成路径
    generatePath(paths);

    // 准备顶点数据:直线 (2 点) + 曲线 (4 点)
    std::vector<float> vertices;
    for (const auto& point : paths[0].controlPoints)
    {
        vertices.push_back(point.x);
        vertices.push_back(point.y);
    }

    // 顶点数组对象
    GLuint VAO, VBO, EBO;
    // 生成顶点数组对象
    glGenVertexArrays(1, &VAO);
    // 生成顶点缓冲对象
    glGenBuffers(1, &VBO);
    // 生成索引缓冲对象
    glGenBuffers(1, &EBO);

    // 绑定顶点数组对象
    glBindVertexArray(VAO);
    // 绑定顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 将顶点数据复制到顶点缓冲对象
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 索引数据:直线 (2 点) 和曲线 (4 点)
    std::vector<unsigned int> indices = {
        0, 1,          // 直线
        2, 3, 4, 5     // 曲线
    };
    // 绑定索引缓冲对象
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // 将索引数据复制到索引缓冲对象
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    // 启用顶点属性
    glEnableVertexAttribArray(0);

    // 启用图元重启
    glEnable(GL_PRIMITIVE_RESTART);
    // 设置图元重启索引
    glPrimitiveRestartIndex(0xFFFFFFFF);

    // 初始化变换矩阵
    glm::mat4 transform = glm::mat4(1.0f);
    // 设置变换矩阵的统一变量
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, &transform[0][0]);

    // 细分级别
    float tessLevel = 10.0f;
    // 设置细分级别的统一变量
    glUniform1f(glGetUniformLocation(shaderProgram, "tessLevel"), tessLevel);

    // 箭头密度
    float arrowDensity = 1.0f;
    // 设置箭头密度的统一变量
    glUniform1f(glGetUniformLocation(shaderProgram, "arrowDensity"), arrowDensity);

    // 箭头大小
    float arrowSize = 0.1f;
    // 设置箭头大小的统一变量
    glUniform1f(glGetUniformLocation(shaderProgram, "arrowSize"), arrowSize);

    // 设置清除颜色为白色
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // 设置线宽
    glLineWidth(2.0f);  // 增强线条可见性

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 获取当前窗口大小
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // 更新视口和投影矩阵,适配窗口大小
        glViewport(0, 0, width, height);
        float aspect = static_cast<float>(width) / height;
        glm::mat4 projection = glm::ortho(-X * aspect / zoomFactor + panOffset.x,
            X * aspect / zoomFactor + panOffset.x,
            -X / zoomFactor + panOffset.y,
            X / zoomFactor + panOffset.y, -1.0f, 1.0f);
        // 设置投影矩阵的统一变量
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, &projection[0][0]);

        // 更新变换矩阵:缩放 + 拖动
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(zoomFactor, zoomFactor, 1.0f));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(panOffset, 0.0f));
        transform = translate * scale;
        // 设置变换矩阵的统一变量
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, &transform[0][0]);

        // 动态调整箭头密度和大小
        // 限制密度,防止过密或过稀疏
        arrowDensity = std::clamp(0.5f / zoomFactor, 0.2f, 2.0f);
        // 设置箭头密度的统一变量
        glUniform1f(glGetUniformLocation(shaderProgram, "arrowDensity"), arrowDensity);
        // 限制大小,确保可见性
        arrowSize = std::clamp(0.1f / zoomFactor, 0.05f, 0.2f);
        // 设置箭头大小的统一变量
        glUniform1f(glGetUniformLocation(shaderProgram, "arrowSize"), arrowSize);

        // 清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);
        // 绑定顶点数组对象
        glBindVertexArray(VAO);

        // 绘制直线
        glDrawElements(GL_PATCHES, 2, GL_UNSIGNED_INT, (void*)0);
        // 绘制曲线
        glDrawElements(GL_PATCHES, 4, GL_UNSIGNED_INT, (void*)(2 * sizeof(unsigned int)));

        // 检查OpenGL错误
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        // 交换前后缓冲区
        glfwSwapBuffers(window);
        // 处理事件
        glfwPollEvents();
    }

    // 删除顶点数组对象
    glDeleteVertexArrays(1, &VAO);
    // 删除顶点缓冲对象
    glDeleteBuffers(1, &VBO);
    // 删除索引缓冲对象
    glDeleteBuffers(1, &EBO);
    // 删除着色器程序
    glDeleteProgram(shaderProgram);
    // 销毁GLFW窗口
    glfwDestroyWindow(window);
    // 终止GLFW
    glfwTerminate();

    return 0;
}