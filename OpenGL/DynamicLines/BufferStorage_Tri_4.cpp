// 使用 OpenGL 4.4+， glBufferStorage + 持久映射（persistent mapping），实现无拷贝、无同步开销的极致性能。
// 实现零拷贝、无隐式同步、高性能流式更新——非常适合粒子系统、动态网格、GPU-CPU 数据交换等场景。
// 极致性能、每帧大量更新（OpenGL 4.4 + ）	✅ glBufferStorage + Persistent & Coherent Mapping
/*

- glBufferData ：OpenGL 1.0版本引入，是最基础的缓冲区数据设置函数，所有OpenGL实现都支持。
- glBufferSubData ：OpenGL 1.1版本引入，用于更新缓冲区的部分数据，是对glBufferData的补充。
- glMapBuffer ：OpenGL 1.5版本引入，是较早的缓冲区映射函数，功能较简单，后来被glMapBufferRange替代。
- glMapBufferRange ：OpenGL 3.0版本引入，提供了对缓冲区数据的更精细控制，可以指定访问范围和访问类型。
- glBufferStorage ：OpenGL 4.4版本引入，用于创建不可变的缓冲区存储，支持更高级的内存管理功能，如持久映射和客户端存储。

与传统方式对比	                       是否可变存储	是否支持持久映射	  性能	             复杂度
glBufferData + glBufferSubData	    ✅ 可变	    ❌ 不支持	       中等（有拷贝）	     简单
glBufferData + glMapBufferRange	    ✅ 可变	    ❌ 不支持           较好	                中等
glBufferStorage + glMapBufferRange	❌ 不可变	✅ 支持             极高（零拷贝, 无同步） 较高
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

// 检查 OpenGL 版本或扩展
bool checkPersistentMappingSupport()
{
    // 支持 glBufferStorage
   return (GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 4));
}

const char* vs = R"(
#version 440 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
void main() {
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

const char* fs = R"(
#version 440 core
in vec3 ourColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(ourColor, 1.0f);
}
)";

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// 生成随机颜色的函数
void generateRandomColor(float& r, float& g, float& b)
{
    // 生成较鲜艳的颜色，避免太暗
    r = 0.2f + (rand() % 801) / 1000.0f; // r: 0.2-1.0
    g = 0.2f + (rand() % 801) / 1000.0f; // g: 0.2-1.0
    b = 0.2f + (rand() % 801) / 1000.0f; // b: 0.2-1.0
}

int main()
{
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4); // 必须 ≥ 4.4
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Persistent Mapping Example", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
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

    // 检查是否支持 persistent mapping
    if (!checkPersistentMappingSupport())
    {
        std::cerr << "OpenGL 4.4 or ARB_buffer_storage not supported!" << std::endl;
        return -1;
    }

    // 编译着色器
    auto compileShader = [](GLenum type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);

        GLint ok;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

        if (!ok)
        {
            char log[512];
            glGetShaderInfoLog(s, 512, nullptr, log);
            std::cerr << "Shader compile error: " << log << std::endl;
        }
        return s;
        };

    GLuint vs = compileShader(GL_VERTEX_SHADER, vs);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fs);

    GLuint program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    srand((unsigned)time(nullptr)); // 初始化随机种子

    // 初始顶点数据（x, y, z, r, g, b）
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f // 将动态更新此顶点的 Y
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

    const size_t bufferSize = sizeof(vertices);

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 🔥 关键：使用 glBufferStorage 创建持久映射缓冲区 🔷 创建“不可变”但高性能的缓冲区
    // 一旦创建，缓冲区大小和存储不可变（更高效）。
    // flags 中必须包含 GL_MAP_PERSISTENT_BIT 才能持久映射。
    glBufferStorage(
        GL_ARRAY_BUFFER,            // 目标
        bufferSize,                 // 大小（字节）
        vertices,                   // 初始数据（可为 nullptr）
        GL_MAP_WRITE_BIT |          // 允许映射后读/写  ← 核心标志位！
        GL_MAP_PERSISTENT_BIT | // 允许持久映射（多次使用，不需反复 map/unmap）。
        GL_MAP_COHERENT_BIT     // CPU 写入后，GPU 自动可见，无需手动 flush
    );

    // 设置顶点属性
    // 位置属性（x, y, z）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 颜色属性（r, g, b）
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 🔥 永久映射缓冲区（只需 map 一次！）映射缓冲区的一部分到 CPU 地址空间
    float* mappedVertices = (float*)glMapBufferRange(
        GL_ARRAY_BUFFER,            // 目标
        0,                          // 偏移  ← 映射整个缓冲区
        bufferSize,                 // 大小（字节）
        GL_MAP_WRITE_BIT |          //  ← 核心标志位！
        GL_MAP_PERSISTENT_BIT | //
        GL_MAP_COHERENT_BIT     //
    );

    if (!mappedVertices)
    {
        std::cerr << "Failed to map buffer persistently!" << std::endl;
        return -1;
    }

    // 不再需要 glBindBuffer/VBO 操作来更新数据！
    // mappedVertices 指针在整个程序生命周期内有效

    float time = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        time += 0.016f;

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);

        // 🔥 直接写入映射内存！无需 glBufferSubData，无需 glMap/glUnmap
        mappedVertices[2 * 6 + 1] = 0.5f + 0.3f * sin(time); // 第3个顶点的Y（索引=2*6+1）

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 渲染结束：取消映射并清理
    glUnmapBuffer(GL_ARRAY_BUFFER); // 虽然 persistent，但退出前应 unmap

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}