#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 立方体顶点数据（仅位置）
float vertices[] = {
    // 位置(3)
    -0.5f, -0.5f, -0.5f, // 0
     0.5f, -0.5f, -0.5f, // 1
     0.5f,  0.5f, -0.5f, // 2
    -0.5f,  0.5f, -0.5f, // 3
    -0.5f, -0.5f,  0.5f, // 4
     0.5f, -0.5f,  0.5f, // 5
     0.5f,  0.5f,  0.5f, // 6
    -0.5f,  0.5f,  0.5f  // 7
};

// 立方体索引
unsigned int indices[] = {
    0, 1, 2, 2, 3, 0, // 后面
    4, 5, 6, 6, 7, 4, // 前面
    0, 1, 5, 5, 4, 0, // 底面
    2, 3, 7, 7, 6, 2, // 顶面
    0, 3, 7, 7, 4, 0, // 左面
    1, 2, 6, 6, 5, 1  // 右面
};

// 简单着色器（OpenGL 3.3）
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // 红色
}
)";

// 检查 OpenGL 错误
void checkGLError(const char* operation)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error after " << operation << ": " << err << std::endl;
    }
}

// 编译着色器
GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    if (!shader)
    {
        std::cerr << "Failed to create shader" << std::endl;
        return 0;
    }
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    checkGLError("glCompileShader");

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error (" << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "): " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// 创建着色器程序
GLuint createProgram(const char* vsSource, const char* fsSource)
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    if (!vertexShader) return 0;
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);
    if (!fragmentShader)
    {
        glDeleteShader(vertexShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    if (!program)
    {
        std::cerr << "Failed to create program" << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    checkGLError("glLinkProgram");

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking error: " << infoLog << std::endl;
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置 OpenGL 3.3 核心配置文件
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Simple Cube", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    checkGLError("glfwMakeContextCurrent");

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLAD Initialized: " << (GLAD_GL_VERSION_3_3 ? "Success" : "Failed") << std::endl;
    checkGLError("gladLoadGLLoader");
    
    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    // 验证 OpenGL 函数指针
    if (!glGenBuffers || !glBindBuffer || !glBufferData)
    {
        std::cerr << "Critical OpenGL functions not loaded by GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 设置视口
    glViewport(0, 0, 800, 600);
    checkGLError("glViewport");

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    checkGLError("glEnable(GL_DEPTH_TEST)");

    // 创建着色器程序
    GLuint program = createProgram(vertexShaderSource, fragmentShaderSource);
    if (!program)
    {
        std::cerr << "Failed to create shader program" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLint mvpLoc = glGetUniformLocation(program, "uMVP");
    if (mvpLoc == -1)
    {
        std::cerr << "Failed to find uniform uMVP" << std::endl;
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    checkGLError("glGetUniformLocation");

    // 创建 VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    checkGLError("VBO setup");

    // 验证 VBO
    GLint bufferSize;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    std::cout << "VBO size: " << bufferSize << " (expected: " << sizeof(vertices) << ")" << std::endl;

    // 创建 EBO
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    checkGLError("EBO setup");

    // 创建 VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBindVertexArray(0);
    checkGLError("VAO setup");

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // 更新视口
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        checkGLError("glViewport update");

        // 清屏
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkGLError("glClear");

        // 使用单位矩阵
        float mvp[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // 绘制
        glUseProgram(program);
        checkGLError("glUseProgram");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp);
        checkGLError("glUniformMatrix4fv");
        glBindVertexArray(vao);
        checkGLError("glBindVertexArray");
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        checkGLError("glDrawElements");

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}