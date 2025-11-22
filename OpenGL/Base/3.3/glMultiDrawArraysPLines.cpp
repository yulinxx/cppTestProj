/*
glMultiDrawArrays 是 OpenGL 1.4+ 中用于高效绘制多个图元（primitives）的函数。
在一次调用中绘制多个独立的几何图元（如点、线、三角形等），
而不需要多次调用 glDrawArrays，从而减少 CPU 与 GPU 之间的通信开销，提高渲染性能。

void glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount);
ode	GLenum	绘制的图元类型，例如：GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES, GL_TRIANGLE_STRIP 等。
first	const GLint* 指向一个整数数组，每个元素表示对应图元在顶点数组中的起始索引。
count	const GLsizei* 指向一个整数数组，每个元素表示对应图元要绘制的顶点数量。
drawcount	GLsizei	要绘制的图元个数（即 first 和 count 数组的长度）。

🧩 使用步骤
1.
准备顶点数据
将所有图元的顶点数据合并到一个大的顶点缓冲区（VBO）中。
2.
设置 VAO/VBO
创建并绑定顶点数组对象（VAO）和顶点缓冲对象（VBO）。
配置顶点属性指针（如位置、颜色等）。
3.
构建 first 和 count 数组
•
first[i] 表示第 i 个图元从哪个顶点开始。
count[i] 表示第 i 个图元包含多少个顶点。
4.
调用 glMultiDrawArrays
使用上述参数一次性提交多个图元的绘制请求。

*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <random>

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

// 窗口尺寸
const unsigned int SCR_WIDTH = 1400;
const unsigned int SCR_HEIGHT = 1400;

// 处理输入的函数
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// 窗口尺寸变化回调
void framebuffer_size_cb(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建GLFW窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Random Multi-Lines Example", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_cb);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
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

    // 顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);

    // 检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }

    // 片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);

    // 检查编译错误
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }

    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 检查链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
            << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 定义常量
    const int NUM_LINES = 10000;    // 多线段数量
    const int MIN_VERTICES = 2;   // 每条多线段的最小顶点数
    const int MAX_VERTICES = 100; // 每条多线段的最大顶点数

    // 数据存储
    std::vector<float> vertices;       // 所有顶点数据（x, y, z, r, g, b）
    std::vector<GLint> firstIndices;   // 每条多线段的起始索引
    std::vector<GLsizei> vertexCounts; // 每条多线段的顶点数量

    // 随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> vertexDist(MIN_VERTICES, MAX_VERTICES); // 顶点数量分布
    std::uniform_real_distribution<> posDist(-1.0f, 1.0f);                  // 位置分布
    std::uniform_real_distribution<> colorDist(0.2f, 1.0f);                 // 颜色分布

    // 生成随机多线段数据
    int currentVertexOffset = 0;
    for (int i = 0; i < NUM_LINES; ++i)
    {
        // 随机确定当前多线段的顶点数
        int numVertices = vertexDist(gen);

        // 记录当前多线段的起始索引和顶点数
        firstIndices.push_back(currentVertexOffset);
        vertexCounts.push_back(numVertices);

        // 为当前多线段生成随机颜色
        float r = colorDist(gen);
        float g = colorDist(gen);
        float b = colorDist(gen);

        // 生成顶点数据
        for (int j = 0; j < numVertices; ++j)
        {
            // 生成随机位置
            float x = posDist(gen);
            float y = posDist(gen);
            float z = 0.0f; // 在2D平面上

            // 添加顶点位置和颜色
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(r);
            vertices.push_back(g);
            vertices.push_back(b);

            //currentVertexOffset++;
        }

        currentVertexOffset += numVertices;
    }

    // 创建VAO和VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 绑定VAO和VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 设置顶点属性 - 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 设置顶点属性 - 颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 设置视口
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // 主渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入
        processInput(window);

        // 渲染指令
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);

        // 绑定VAO
        glBindVertexArray(VAO);

        // 绘制所有多线段
        glMultiDrawArrays(GL_LINE_STRIP, firstIndices.data(), vertexCounts.data(), NUM_LINES);

        // 交换缓冲并轮询事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // 终止GLFW
    glfwTerminate();
    return 0;
}
