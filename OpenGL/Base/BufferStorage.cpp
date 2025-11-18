// 使用 OpenGL 4.4+， glBufferStorage + 持久映射（persistent mapping），实现无拷贝、无同步开销的极致性能。
// 实现零拷贝、无隐式同步、高性能流式更新——非常适合粒子系统、动态网格、GPU-CPU 数据交换等场景。
//极致性能、每帧大量更新（OpenGL 4.4 + ）	✅ glBufferStorage + Persistent & Coherent Mapping
/*

与传统方式对比	                       是否可变存储	是否支持持久映射	        性能	                复杂度
glBufferData + glBufferSubData	    ✅ 可变	    ❌ 不支持	            中等（有拷贝）	        简单
glBufferData + glMapBufferRange	    ✅ 可变	    ❌ 不支持（加 flag 也无效）较好	            中等
glBufferStorage + glMapBufferRange	❌ 不可变	✅ 支持（需正确 flags）	极高（零拷贝, 无同步） 较高
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// 检查 OpenGL 版本或扩展
bool checkPersistentMappingSupport()
{
    //if (GLAD_GL_VERSION_4_4) return true;
    //if (GLAD_GL_ARB_buffer_storage) return true;

    if (GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 4))
    {
        // 支持 glBufferStorage
        return true;
    }
    return false;
}

const char* vertexShaderSource = R"(
#version 440 core
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 440 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.8f, 0.2f, 0.6f, 1.0f);
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

int main()
{
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4); // 必须 ≥ 4.4
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Persistent Mapping Example", nullptr, nullptr);
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
            char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
            std::cerr << "Shader compile error: " << log << std::endl;
        }
        return s;
        };

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 初始顶点数据
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f   // 将动态更新此顶点的 Y
    };

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
        GL_ARRAY_BUFFER,        // 目标
        bufferSize,             // 大小（字节）
        vertices,               // 初始数据（可为 nullptr）
        GL_MAP_WRITE_BIT |      // 允许映射后读/写  ← 核心标志位！
        GL_MAP_PERSISTENT_BIT | // 允许持久映射（多次使用，不需反复 map/unmap）。
        GL_MAP_COHERENT_BIT     // CPU 写入后，GPU 自动可见，无需手动 flush
    );

    // 设置顶点属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 🔥 永久映射缓冲区（只需 map 一次！）映射缓冲区的一部分到 CPU 地址空间
    float* mappedVertices = (float*)glMapBufferRange(
        GL_ARRAY_BUFFER,        // 目标
        0,                      // 偏移  ← 映射整个缓冲区
        bufferSize,             // 大小（字节）
        GL_MAP_WRITE_BIT |      //  ← 核心标志位！
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
        mappedVertices[7] = 0.5f + 0.3f * sin(time); // 第3个顶点的Y（索引=2*3+1=7）

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