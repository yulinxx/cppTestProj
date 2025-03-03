
// 使用几何着色器，绘制出一条有三阶贝塞尔曲线的  --->--- 线
// 能缩放以及拖动画面

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>  // 包含clamp函数
#include <cmath>      // 包含clamp函数

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr float X = 4.0f;  // 初始视图范围

// Vertex Shader: 传递控制点位置
const char* vertexShaderSource = R"(
#version 400 core
layout(location = 0) in vec2 in_pos;

out vec2 v_pos;

void main() {
    v_pos = in_pos;
}
)";

// Tessellation Control Shader: 设置细分级别
const char* tessControlShaderSource = R"(
#version 400 core
layout(vertices = 4) out;

uniform float tessLevel = 10.0;

in vec2 v_pos[];

out vec2 tc_pos[];

void main() {
    tc_pos[gl_InvocationID] = v_pos[gl_InvocationID];

    if (gl_InvocationID == 0) {
        if (gl_in.length() == 2) {  // 直线
            gl_TessLevelOuter[0] = 1.0;
            gl_TessLevelOuter[1] = 1.0;
        } else {  // 曲线
            gl_TessLevelOuter[0] = tessLevel;
            gl_TessLevelOuter[1] = tessLevel;
        }
    }
}
)";

// Tessellation Evaluation Shader: 计算直线和曲线的点
const char* tessEvaluationShaderSource = R"(
#version 400 core
layout(isolines, equal_spacing) in;

uniform mat4 transform;  // 包含缩放和拖动的变换

in vec2 tc_pos[];

out vec2 tes_pos;  // 传递给 Geometry Shader 的位置

void main() {
    float u = gl_TessCoord.x;

    vec4 pos;
    if (gl_in.length() == 2) {  // 直线
        pos = mix(vec4(tc_pos[0], 0.0, 1.0), vec4(tc_pos[1], 0.0, 1.0), u);
    } else {  // 三阶贝塞尔曲线
        float u2 = u * u;
        float u3 = u2 * u;
        float oneMinusU = 1.0 - u;
        float oneMinusU2 = oneMinusU * oneMinusU;
        float oneMinusU3 = oneMinusU2 * oneMinusU;

        pos = oneMinusU3 * vec4(tc_pos[0], 0.0, 1.0) +
              3.0 * oneMinusU2 * u * vec4(tc_pos[1], 0.0, 1.0) +
              3.0 * oneMinusU * u2 * vec4(tc_pos[2], 0.0, 1.0) +
              u3 * vec4(tc_pos[3], 0.0, 1.0);
    }

    tes_pos = (transform * pos).xy;  // 应用缩放和拖动
    gl_Position = transform * pos;
}
)";

// Geometry Shader: 生成线段和动态调整的箭头
const char* geometryShaderSource = R"(
#version 400 core
layout(lines) in;
layout(line_strip, max_vertices = 8) out;

in vec2 tes_pos[];

out vec2 geo_pos;  // 传递给 Fragment Shader 的位置

uniform mat4 transform;  // 变换矩阵
uniform float arrowDensity = 1.0;  // 动态箭头密度
uniform float arrowSize = 0.1;     // 动态箭头大小

void main() {
    vec2 p0 = tes_pos[0];
    vec2 p1 = tes_pos[1];
    vec2 dir = normalize(p1 - p0);  // 线段方向
    vec2 perp = vec2(-dir.y, dir.x);  // 垂直方向

    float len = length(p1 - p0);

    // 绘制线段
    vec4 p0_transformed = transform * vec4(p0, 0.0, 1.0);
    vec4 p1_transformed = transform * vec4(p1, 0.0, 1.0);
    gl_Position = p0_transformed;
    geo_pos = p0;
    EmitVertex();
    gl_Position = p1_transformed;
    geo_pos = p1;
    EmitVertex();
    EndPrimitive();

    // 动态调整箭头密度和大小
    float baseDensity = clamp(0.5f / arrowDensity, 0.2f, 2.0f);  // 限制密度，防止过密或过稀疏
    int maxArrows = 5;  // 最大箭头数，防止过密
    int numArrows = min(int(floor(len / baseDensity)), maxArrows);  // 限制箭头数量
    if (numArrows < 1) numArrows = 1;  // 至少一个箭头

    for (int i = 0; i < numArrows; ++i) {
        float t = (float(i) + 0.5) / float(numArrows);  // 均匀分布
        vec2 arrowPos = mix(p0, p1, t);

        // 动态调整箭头大小，保持可见性
        float adjustedArrowSize = clamp(arrowSize / arrowDensity, 0.05f, 0.2f);  // 限制大小范围
        vec2 tip = arrowPos + dir * adjustedArrowSize;
        vec2 wing1 = arrowPos - dir * adjustedArrowSize * 0.5 + perp * adjustedArrowSize * 0.5;
        vec2 wing2 = arrowPos - dir * adjustedArrowSize * 0.5 - perp * adjustedArrowSize * 0.5;

        // 第一条线段：中心到上翼
        gl_Position = transform * vec4(arrowPos, 0.0, 1.0);
        geo_pos = arrowPos;
        EmitVertex();
        gl_Position = transform * vec4(wing1, 0.0, 1.0);
        geo_pos = wing1;
        EmitVertex();
        EndPrimitive();

        // 第二条线段：中心到下翼
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
in vec2 geo_pos;
out vec4 fragColor;

uniform vec4 color = vec4(0.0, 0.0, 1.0, 1.0);  // 蓝色

void main() {
    fragColor = color;
}
)";

GLuint loadShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint tessControlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(tessControlShader, 1, &tessControlShaderSource, nullptr);
    glCompileShader(tessControlShader);

    GLuint tessEvaluationShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
    glShaderSource(tessEvaluationShader, 1, &tessEvaluationShaderSource, nullptr);
    glCompileShader(tessEvaluationShader);

    GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometryShaderSource, nullptr);
    glCompileShader(geometryShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, tessControlShader);
    glAttachShader(shaderProgram, tessEvaluationShader);
    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(tessControlShader);
    glDeleteShader(tessEvaluationShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

struct Path
{
    std::vector<glm::vec2> controlPoints;  // 直线 (2 点) + 曲线 (4 点)
};

void generatePath(std::vector<Path>& paths)
{
    paths.resize(1);  // 仅生成一条路径
    Path& path = paths[0];

    // 直线部分：从 (-X, 0) 到 (-X/2, 0)
    path.controlPoints.push_back(glm::vec2(-X, 0.0f));
    path.controlPoints.push_back(glm::vec2(-X / 2.0f, 0.0f));

    // 三阶贝塞尔曲线：从 (-X/2, 0) 到 (X, 0)
    path.controlPoints.push_back(glm::vec2(-X / 2.0f, 0.0f));  // 起始点
    path.controlPoints.push_back(glm::vec2(-X / 4.0f, X / 2.0f));  // 控制点 1
    path.controlPoints.push_back(glm::vec2(X / 2.0f, -X / 2.0f));  // 控制点 2
    path.controlPoints.push_back(glm::vec2(X, 0.0f));  // 终点
}

float zoomFactor = 1.0f;
glm::vec2 panOffset(0.0f, 0.0f);  // 拖动偏移
bool isDragging = false;
double lastX, lastY;
int windowWidth = 800, windowHeight = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoomFactor += float(yoffset) * 0.1f;
    zoomFactor = std::max(0.1f, std::min(5.0f, zoomFactor));  // 限制缩放范围
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            isDragging = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        }
        else if (action == GLFW_RELEASE)
        {
            isDragging = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isDragging)
    {
        double dx = xpos - lastX;
        double dy = ypos - lastY;
        float scale = 0.01f / zoomFactor;  // 拖动敏感度随缩放调整
        panOffset.x += dx * scale;
        panOffset.y -= dy * scale;  // y 方向反向（OpenGL 坐标系）
        lastX = xpos;
        lastY = ypos;
    }
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);  // 窗口最大化

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Line with Arrow (Straight + Bezier)", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

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

    GLuint shaderProgram = loadShader();
    glUseProgram(shaderProgram);

    std::vector<Path> paths;
    generatePath(paths);

    // 准备顶点数据：直线 (2 点) + 曲线 (4 点)
    std::vector<float> vertices;
    for (const auto& point : paths[0].controlPoints)
    {
        vertices.push_back(point.x);
        vertices.push_back(point.y);
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 索引数据：直线 (2 点) 和曲线 (4 点)
    std::vector<unsigned int> indices = {
        0, 1,          // 直线
        2, 3, 4, 5     // 曲线
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);

    glm::mat4 transform = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, &transform[0][0]);

    float tessLevel = 10.0f;
    glUniform1f(glGetUniformLocation(shaderProgram, "tessLevel"), tessLevel);

    float arrowDensity = 1.0f;
    glUniform1f(glGetUniformLocation(shaderProgram, "arrowDensity"), arrowDensity);

    float arrowSize = 0.1f;
    glUniform1f(glGetUniformLocation(shaderProgram, "arrowSize"), arrowSize);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);  // 增强线条可见性

    while (!glfwWindowShouldClose(window))
    {
        // 获取当前窗口大小
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // 更新视口和投影矩阵，适配窗口大小
        glViewport(0, 0, width, height);
        float aspect = static_cast<float>(width) / height;
        glm::mat4 projection = glm::ortho(-X * aspect / zoomFactor + panOffset.x,
            X * aspect / zoomFactor + panOffset.x,
            -X / zoomFactor + panOffset.y,
            X / zoomFactor + panOffset.y, -1.0f, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, &projection[0][0]);

        // 更新变换矩阵：缩放 + 拖动
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(zoomFactor, zoomFactor, 1.0f));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(panOffset, 0.0f));
        transform = translate * scale;
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, &transform[0][0]);

        // 动态调整箭头密度和大小
        arrowDensity = std::clamp(0.5f / zoomFactor, 0.2f, 2.0f);  // 限制密度，防止过密或过稀疏
        glUniform1f(glGetUniformLocation(shaderProgram, "arrowDensity"), arrowDensity);
        arrowSize = std::clamp(0.1f / zoomFactor, 0.05f, 0.2f);  // 限制大小，确保可见性
        glUniform1f(glGetUniformLocation(shaderProgram, "arrowSize"), arrowSize);

        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(VAO);

        // 绘制直线
        glDrawElements(GL_PATCHES, 2, GL_UNSIGNED_INT, (void*)0);
        // 绘制曲线
        glDrawElements(GL_PATCHES, 4, GL_UNSIGNED_INT, (void*)(2 * sizeof(unsigned int)));

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}