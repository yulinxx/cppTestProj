#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 着色器源代码
const char* vs = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aColor;
    out vec3 ourColor;
    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        ourColor = aColor;
    }
)";

const char* fs = R"(
    #version 330 core
    in vec3 ourColor;
    out vec4 FragColor;
    void main()
    {
        FragColor = vec4(ourColor, 1.0f);
    }
)";

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    // 初始化GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1400, 1200, "glMultiDrawArrays Example", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 加载GLAD
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

    // 编译着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, nullptr);
    glCompileShader(vertexShader);

    // 检查着色器编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, nullptr);
    glCompileShader(fragmentShader);

    // 检查着色器编译错误
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 检查程序链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 定义顶点数据（包含位置和颜色）
    // 三角形1：红色三角形（左上）
    // 三角形2：绿色三角形（右上）
    // 三角形3：蓝色三角形（左下）
    // 三角形4：黄色三角形（右下）
    float vertices[] = {
        // 三角形1 - 红色
        -0.9f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // 顶点1
        -0.6f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // 顶点2
        -0.75f, 0.8f, 0.0f,  1.0f, 0.0f, 0.0f,  // 顶点3

        // 三角形2 - 绿色
        0.0f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // 顶点4
        0.3f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // 顶点5
        0.15f, 0.8f, 0.0f,  0.0f, 1.0f, 0.0f,  // 顶点6

        // 三角形3 - 蓝色
        -0.9f, -0.8f, 0.0f,  0.0f, 0.0f, 1.0f,  // 顶点7
        -0.6f, -0.8f, 0.0f,  0.0f, 0.0f, 1.0f,  // 顶点8
        -0.75f,-0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // 顶点9

        // 三角形4 - 黄色
        0.0f, -0.8f, 0.0f,  1.0f, 1.0f, 0.0f,  // 顶点10
        0.3f, -0.8f, 0.0f,  1.0f, 1.0f, 0.0f,  // 顶点11
        0.15f,-0.5f, 0.0f,  1.0f, 1.0f, 0.0f   // 顶点12
    };

    // 设置glMultiDrawArrays所需的参数
    // 每个数组的起始索引
    GLint first[] = { 0, 3, 6, 9 };
    // 每个数组的顶点数量
    //GLsizei count[] = { 3, 0, 3, 3 }; // 可控制第二個不绘制 即设置为 0
    GLsizei count[] = { 3, 3, 3, 3 };

    // 要绘制的数组数量
    int drawCount = 4;

    // 创建VAO和VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 绑定VAO和VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针 - 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 设置顶点属性指针 - 颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 输入处理
        processInput(window);

        // 渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);

        // 绑定VAO
        glBindVertexArray(VAO);

        // 使用glMultiDrawArrays绘制多个三角形
        glMultiDrawArrays(GL_TRIANGLES, first, count, drawCount);

        // 交换缓冲区并处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 释放资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}