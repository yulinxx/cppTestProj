// 创建一个三角形。
// 使用 glMapBufferRange 将 VBO 映射到 CPU 可写内存。
// 每帧动态修改三角形顶部顶点的 Y 坐标（使其上下跳动）。
// 使用 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT 提高性能。

// glMapBufferRange 比 glBufferSubData 更高效（尤其在频繁/大块更新时），因为它避免了额外的内存拷贝。
// 使用标志位：
// GL_MAP_WRITE_BIT：写入的数据
// GL_MAP_INVALIDATE_RANGE_BIT：不关心该范围的旧内容，驱动可做优化（如避免同步等待）。
// 必须调用 glUnmapBuffer，否则行为未定义。
// 映射后返回的是 void*，需强转为对应类型（float*）。
// 偏移和长度单位是 字节。

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(0.2f, 0.8f, 0.9f, 1.0f);
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "glMapBufferRange Example", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 编译着色器
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, nullptr);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fs);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 初始顶点：静态分配，但将频繁更新
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  // 顶点 0
         0.5f, -0.5f, 0.0f,  // 顶点 1
         0.0f,  0.5f, 0.0f   // 顶点 2（将被动态修改）
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 使用 GL_DYNAMIC_DRAW 表示会频繁更新
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    float time = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        time += 0.016f; // ~60 FPS

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);

        // === 使用 glMapBufferRange 更新顶点数据 ===
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // 要更新的是第 2 个顶点（索引为 2）的 Y 分量
        size_t offset = (2 * 3 + 1) * sizeof(float); // 第7个float，即Y坐标
        size_t length = sizeof(float);

        // 映射缓冲区的一小段（只写、允许GPU优化）
        //glMapBufferRange 用于高效映射缓冲区对象的一部分到客户端地址空间的函数
        // 常用于动态更新顶点、索引或 Uniform 缓冲数据。
        void* mappedPtr = glMapBufferRange(
            GL_ARRAY_BUFFER,    // 目标缓冲区
            offset,             // 偏移
            length,             // 长度
            GL_MAP_WRITE_BIT |
            GL_MAP_INVALIDATE_RANGE_BIT  // 告诉驱动这段旧数据可丢弃
        );

        if (mappedPtr)
        {
            float newY = 0.5f + 0.3f * sin(time);
            *(float*)mappedPtr = newY;
            glUnmapBuffer(GL_ARRAY_BUFFER); // 必须 unmap！
        }
        else
        {
            std::cerr << "glMapBufferRange failed!" << std::endl;
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

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