// 该程序使用 GLFW 创建窗口上下文。
// 使用 OpenGL 3.3 Core Profile、通过 GLAD 加载函数指针，
// 并演示如何使用 glBufferSubData 更新顶点缓冲区（VBO）中部分数据的简单示例。

// 创建一个三角形。
// 初始顶点数据上传到 VBO。
// 在主循环中，每帧更新三角形的一个顶点位置（比如让三角形“跳动”），使用 glBufferSubData 只更新需要修改的部分。

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

// 简单的顶点着色器源码
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
void main()
{
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

// 简单的片段着色器源码
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;
void main()
{
    FragColor = vec4(ourColor, 1.0f);
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// 生成随机颜色的函数
void generateRandomColor(float& r, float& g, float& b)
{
    // 生成较鲜艳的颜色，避免太暗
    r = 0.2f + (rand() % 801) / 1000.0f;  // r: 0.2-1.0
    g = 0.2f + (rand() % 801) / 1000.0f;  // g: 0.2-1.0
    b = 0.2f + (rand() % 801) / 1000.0f;  // b: 0.2-1.0
}

int main()
{
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "glBufferSubData Example", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 输出 OpenGL 信息
    if (0)
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
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
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);



    srand((unsigned)time(nullptr)); // 初始化随机种子

    // 初始顶点数据（x, y, z, r, g, b）
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f
    };

    // 为每个顶点生成随机颜色
    for (int i = 0; i < 3; i++)
    {
        float r, g, b;
        generateRandomColor(r, g, b);
        vertices[i * 6 + 3] = r;
        vertices[i * 6 + 4] = g;
        vertices[i * 6 + 5] = b;
    }

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 注意：使用 GL_DYNAMIC_DRAW,申明这个缓冲区内容会频繁更新。
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // 设置顶点属性指针：位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 设置顶点属性指针：颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 主渲染循环
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // 每帧更新三角形顶部顶点的 Y 坐标
        static float time = 0.0f;
        time += 0.016f; // 假设 ~60 FPS
        float newY = 0.5f + 0.3f * sin(time); // 在 0.2 ~ 0.8 之间跳动

        // 使用 glBufferSubData 更新第三个顶点（索引为 2）的 Y 值
        // 第三个顶点从字节偏移 2 * 6 * sizeof(float) 开始，Y坐标是第1个分量
        glBufferSubData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(float) + 1 * sizeof(float), sizeof(float), &newY);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}