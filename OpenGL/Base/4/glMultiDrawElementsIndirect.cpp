#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <random>
#include <memory>

// 确保使用正确的无符号整数类型
typedef unsigned int uint;

// 定义间接绘制命令结构（确保内存对齐正确）
//#ifdef __GNUC__
//#define PACKED __attribute__((packed))
//#else
//#define PACKED
//#endif

typedef struct PACKED
{
    uint count;         // 每个图元的顶点数量
    uint instanceCount; // 实例数量
    uint firstIndex;    // 第一个索引的位置
    int baseVertex;     // 基础顶点偏移
    uint baseInstance;  // 基础实例偏移
} DrawElementsIndirectCmd;

// 窗口大小
const unsigned int SCR_WIDTH = 1400;
const unsigned int SCR_HEIGHT = 1400;

// 处理窗口大小变化的回调函数
void framebuffer_size_cb(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 处理输入
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// 编译着色器程序
unsigned int compileShaderProgram()
{
    // 顶点着色器源码
    const char* vertexShaderSource = R"(
    #version 450 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;

    out vec3 ourColor;

    void main() {
        gl_Position = vec4(aPos, 1.0);
        ourColor = aColor;
    }
    )";

    // 片段着色器源码
    const char* fragmentShaderSource = R"(
    #version 450 core
    in vec3 ourColor;

    out vec4 FragColor;

    void main() {
        FragColor = vec4(ourColor, 1.0);
    }
    )";

    // 编译顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // 检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }

    // 编译片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 检查编译错误
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
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
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
            << infoLog << std::endl;
    }

    // 删除着色器，因为它们已经被链接到程序中
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main()
{
    // GLFW初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建GLFW窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "glMultiDrawElementsIndirect Example", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_cb);

    // 初始化GLAD
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

    // OpenGL 4.0及以上版本原生支持间接绘制，无需单独检查ARB扩展
    // 确保OpenGL版本至少为4.0
    int major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 4)
    {
        std::cerr << "OpenGL 4.0 or higher is required for indirect drawing!" << std::endl;
        std::cerr << "Current OpenGL version: " << major << "." << minor << std::endl;
        return -1;
    }

    // 编译着色器程序
    unsigned int shaderProgram = compileShaderProgram();

    // 定义常量
    const int NUM_TRIS = 100;       // 三角形数量
    const int VERTICES_PER_TRI = 3; // 每个三角形的顶点数
    const float TRI_SIZE = 0.1f;    // 三角形大小

    // 生成随机三角形的函数
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(-0.8f, 0.8f);
    std::uniform_real_distribution<float> disY(-0.8f, 0.8f);
    std::uniform_real_distribution<float> disColor(0.0f, 1.0f);

    // 准备顶点数据 (位置 + 颜色)
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<DrawElementsIndirectCmd> commands;

    // 随机生成三角形数据
    for (int i = 0; i < NUM_TRIS; ++i)
    {
        // 随机三角形中心位置
        float centerX = disX(gen);
        float centerY = disY(gen);
        float centerZ = 0.0f;

        // 随机颜色
        float r = disColor(gen);
        float g = disColor(gen);
        float b = disColor(gen);

        // 当前三角形的基础顶点索引
        unsigned int baseVertex = static_cast<unsigned int>(vertices.size() / 6); // 每个顶点有6个值(x,y,z,r,g,b)

        // 添加三角形的三个顶点
        // 顶点1
        vertices.push_back(centerX);
        vertices.push_back(centerY + TRI_SIZE);
        vertices.push_back(centerZ);
        vertices.push_back(r);
        vertices.push_back(g);
        vertices.push_back(b);

        // 顶点2
        vertices.push_back(centerX - TRI_SIZE);
        vertices.push_back(centerY - TRI_SIZE);
        vertices.push_back(centerZ);
        vertices.push_back(r);
        vertices.push_back(g);
        vertices.push_back(b);

        // 顶点3
        vertices.push_back(centerX + TRI_SIZE);
        vertices.push_back(centerY - TRI_SIZE);
        vertices.push_back(centerZ);
        vertices.push_back(r);
        vertices.push_back(g);
        vertices.push_back(b);

        // 添加索引
        indices.push_back(baseVertex);
        indices.push_back(baseVertex + 1);
        indices.push_back(baseVertex + 2);

        // 创建间接绘制命令
        DrawElementsIndirectCmd cmd;
        cmd.count = VERTICES_PER_TRI;          // 每个三角形3个顶点
        cmd.instanceCount = 1;                 // 每个命令绘制一个实例
        cmd.firstIndex = i * VERTICES_PER_TRI; // 该三角形在索引数组中的起始位置
        cmd.baseVertex = i * VERTICES_PER_TRI; // 该三角形在顶点数组中的起始位置
        cmd.baseInstance = 0;                  // 基础实例偏移
        commands.push_back(cmd);
    }

    // 创建VAO和VBO
    unsigned int VAO, VBO, EBO, indirectBuffer;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &indirectBuffer);

    // 绑定和配置缓冲区
    glBindVertexArray(VAO);

    // 配置顶点缓冲
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 配置索引缓冲
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 配置间接命令缓冲
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawElementsIndirectCmd), commands.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 启用线框模式以便更好地查看三角形（可选）
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入
        processInput(window);

        // 清除屏幕
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);

        // 绑定VAO
        glBindVertexArray(VAO);

        // 使用glMultiDrawElementsIndirect绘制所有三角形
        // 参数说明：
        // - GL_TRIANGLES: 图元类型
        // - GL_UNSIGNED_INT: 索引类型
        // - 0: 间接命令缓冲区中的偏移量
        // - NUM_TRIS: 要执行的命令数量
        // - 0: 命令之间的步长（0表示紧密排列）
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)0, NUM_TRIS, 0);

        // 交换缓冲并轮询事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &indirectBuffer);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}