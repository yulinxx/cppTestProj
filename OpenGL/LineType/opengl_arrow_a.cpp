
// 使用几何着色器，绘制出一条简单的  --->--- 线

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 in_pos;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(in_pos, 0.0, 1.0);
}
)";

const char* geometryShaderSource = R"(
#version 330 core
layout(lines) in;
layout(line_strip, max_vertices = 6) out; // 增加顶点输出数量，确保完整箭头

uniform mat4 projection;  // 传递投影矩阵
uniform float arrowSize = 0.1;  // 箭头大小

void main() {
    vec2 p0 = gl_in[0].gl_Position.xy;  // 线段起点
    vec2 p1 = gl_in[1].gl_Position.xy;  // 线段终点
    vec2 dir = normalize(p1 - p0);  // 线段方向
    vec2 perp = vec2(-dir.y, dir.x);  // 垂直方向

    // 绘制线段
    gl_Position = projection * vec4(p0, 0.0, 1.0);
    EmitVertex();
    gl_Position = projection * vec4(p1, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();

    // 绘制 > 形式的箭头（位于线段中点）
    vec2 arrowPos = mix(p0, p1, 0.5);  // 箭头中心位置（中点）
    vec2 tip = arrowPos + dir * arrowSize;  // 箭头尖端
    vec2 wing1 = arrowPos - dir * arrowSize * 0.5 + perp * arrowSize * 0.5;  // 上翼
    vec2 wing2 = arrowPos - dir * arrowSize * 0.5 - perp * arrowSize * 0.5;  // 下翼

    // 第一条线段：中心到上翼
    gl_Position = projection * vec4(arrowPos, 0.0, 1.0);
    EmitVertex();
    gl_Position = projection * vec4(wing1, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();

    // 第二条线段：中心到下翼
    gl_Position = projection * vec4(arrowPos, 0.0, 1.0);
    EmitVertex();
    gl_Position = projection * vec4(wing2, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 fragColor;

uniform vec4 color = vec4(0.0, 0.0, 1.0, 1.0);  // 蓝色

void main() {
    fragColor = color;
}
)";

GLuint loadShader() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometryShaderSource, nullptr);
    glCompileShader(geometryShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Simple Line with Arrow", nullptr, nullptr);
    if (!window) 
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 创建着色器程序
    GLuint shaderProgram = loadShader();
    glUseProgram(shaderProgram);

    // 定义一条简单的线段
    std::vector<float> vertices = {
        -0.5f, 0.0f,  // 起点
         0.5f, 0.0f   // 终点
    };

    // 创建 VAO 和 VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 设置正交投影矩阵，覆盖整个窗口
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    // 设置线宽（可选，增强可见性）
    glLineWidth(2.0f);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 2);  // 绘制一条线段

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}