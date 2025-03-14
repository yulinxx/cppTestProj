#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "bezier/bezier.h" // 假设 bezier 头文件在项目目录的 bezier 子目录中
#include <vector>
#include <iostream>

// 顶点着色器源码
const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

// 片段着色器源码
const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色曲线
}
)";

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
    GLFWwindow *window = glfwCreateWindow(1800, 1600, "Cubic Bezier Curve", NULL, NULL);
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

    // 创建并编译着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 创建并链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 删除不再需要的着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 定义三阶贝塞尔曲线的控制点
    std::vector<bezier::Point> controlPoints = {
        {-0.5, -0.5}, // P0
        {-0.5, 0.5},  // P1
        {0.5, 0.5},   // P2
        {0.5, -0.5}   // P3
    };

    // 创建三阶贝塞尔曲线
    bezier::Bezier<3> curve(controlPoints);

    // 生成曲线上的点
    std::vector<float> vertices;
    for (float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        bezier::Point p = curve.valueAt(t);
        vertices.push_back(static_cast<float>(p.x)); // x 坐标
        vertices.push_back(static_cast<float>(p.y)); // y 坐标
        vertices.push_back(0.0f);                    // z 坐标设为 0
    }

    // 创建 VBO 和 VAO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 清空屏幕
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 黑色背景
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);

        // 绑定 VAO
        glBindVertexArray(VAO);

        // 绘制贝塞尔曲线
        glDrawArrays(GL_LINE_STRIP, 0, vertices.size() / 3);

        // 交换缓冲区并处理事件
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