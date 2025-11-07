// 实现鼠标拾取三角网格的方法 - MarsCactus - 博客园
// https://www.cnblogs.com/DesertCactus/p/18537573

// 程序启动后，显示50个随机分布的彩色三角形（不同深度）。
// 左键点击三角形，控制台输出Selected triangle: <ID>，选中三角形变为黄色。
#include <glad/glad.h>      // OpenGL函数加载库

#include <GLFW/glfw3.h>     // 窗口管理库
#include <glm/glm.hpp>      // 数学库，用于矩阵和向量运算
#include <glm/gtc/matrix_transform.hpp>  // 矩阵变换函数
#include <glm/gtc/type_ptr.hpp>         // 类型指针转换

#include <iostream>         // 标准输入输出
#include <vector>           // 向量容器
#include <random>           // 随机数生成

// 窗口尺寸常量定义
const unsigned int SCR_WIDTH = 1400;  // 窗口宽度
const unsigned int SCR_HEIGHT = 1200; // 窗口高度

// 三角形结构体定义
struct Triangle
{
    glm::vec3 vertices[3]; // 三角形的三个顶点坐标
    glm::vec3 color;       // 三角形的显示颜色
    glm::vec3 pickColor;   // 用于拾取的颜色编码（每个三角形唯一）
};

// 全局变量声明
std::vector<Triangle> g_vTriangles;  // 三角形集合
unsigned int VBO, VAO;            // 顶点缓冲区对象和顶点数组对象
unsigned int g_shaderProgram;       // 普通渲染的着色器程序
unsigned int g_pickShaderProgram;   // 拾取模式的着色器程序

glm::mat4 matProj, matView;       // 投影矩阵和视图矩阵

bool g_bPickMode = false;           // 是否处于拾取模式
int g_nSeled = -1;                  // 当前选中的三角形索引（-1表示未选中）

// 检查OpenGL错误的辅助函数
void checkGLError(const char* operation)
{
    GLenum error = glGetError();  // 获取OpenGL错误码
    if (error != GL_NO_ERROR)     // 如果有错误发生
    {
        std::cerr << "OpenGL error after " << operation << ": " << error << std::endl;
    }
}

// 鼠标点击事件回调函数
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // 当左键按下时，激活拾取模式
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        g_bPickMode = true;  // 设置拾取模式标志
    }
}

// 顶点着色器源码
const char* vertexShaderSource = R"(
#version 460 core                // 使用OpenGL 4.6核心配置文件
layout (location = 0) in vec3 aPos;  // 顶点位置输入
uniform mat4 model;              // 模型矩阵
uniform mat4 matView;            // 视图矩阵
uniform mat4 matProj;            // 投影矩阵
void main() {
    // 计算顶点最终位置：投影矩阵 * 视图矩阵 * 模型矩阵 * 顶点坐标
    gl_Position = matProj * matView * model * vec4(aPos, 1.0);
}
)";

// 普通渲染的片段着色器源码
const char* fragmentShaderSource = R"(
#version 460 core
out vec4 FragColor;              // 输出颜色
uniform vec3 color;              // 输入的颜色值
void main() {
    FragColor = vec4(color, 1.0);
}
)";

// 拾取模式的片段着色器源码
const char* pickFragmentShaderSource = R"(
#version 460 core
out vec4 FragColor;              // 输出颜色
uniform vec3 pickColor;          // 拾取颜色（编码了三角形ID）
void main() {
    FragColor = vec4(pickColor, 1.0);
}
)";

// 编译单个着色器的函数
unsigned int compileShader(const char* source, GLenum type)
{
    unsigned int shader = glCreateShader(type);  // 创建着色器对象
    glShaderSource(shader, 1, &source, nullptr); // 设置着色器源码
    glCompileShader(shader);                     // 编译着色器

    int nSuccess;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &nSuccess);  // 获取编译状态

    if (!nSuccess)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);  // 获取详细错误日志

        std::cerr << "Shader compilation error ("
            << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
            << "): " << infoLog << std::endl;

        return 0;  // 返回0表示失败
    }

    return shader;  // 返回编译成功的着色器ID
}

// 创建着色器程序的函数
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    // 编译顶点着色器和片段着色器
    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    // 如果任一着色器编译失败，返回0
    if (!vertexShader || !fragmentShader)
    {
        return 0;
    }

    // 创建着色器程序并链接
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // 检查链接是否成功
    int nSuccess;
    glGetProgramiv(program, GL_LINK_STATUS, &nSuccess);
    if (!nSuccess)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking error: " << infoLog << std::endl;
        return 0;
    }

    // 链接成功后，删除不再需要的着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;  // 返回创建成功的着色器程序ID
}

// 初始化三角形数据的函数
void initTriangles()
{
    // 设置随机数生成器
    std::random_device rd;  // 随机设备
    std::mt19937 gen(rd()); // Mersenne Twister引擎
    // 定义各种随机分布范围
    std::uniform_real_distribution<float> posDist(-5.0f, 5.0f);   // X和Y坐标范围
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);  // 颜色分量范围
    std::uniform_real_distribution<float> zDist(-10.0f, 0.0f);    // Z坐标范围（负表示在相机前方）

    g_vTriangles.clear();  // 清空现有三角形数据
    // 生成50个随机三角形
    for (int i = 0; i < 50; ++i)
    {
        Triangle tri;
        // 为每个三角形随机生成三个顶点
        for (int j = 0; j < 3; ++j)
        {
            tri.vertices[j] = glm::vec3(posDist(gen), posDist(gen), zDist(gen));
        }
        // 为每个三角形随机生成颜色
        tri.color = glm::vec3(colorDist(gen), colorDist(gen), colorDist(gen));

        // 生成拾取颜色：将三角形索引i编码为RGB颜色
        // i 是一个 int 类型，在标准C++实现中， int 通常占用 32位 （4字节）
        // 高8位作为红色分量，中8位作为绿色分量，低8位作为蓝色分量
        tri.pickColor = glm::vec3(
            ((i & 0xFF0000) >> 16) / 255.0f,
            ((i & 0x00FF00) >> 8) / 255.0f,
            (i & 0x0000FF) / 255.0f
        );

        g_vTriangles.push_back(tri);  // 将三角形添加到集合中
    }
}

// 初始化顶点缓冲区的函数
bool initBuffers()
{
    // 生成顶点数组对象(VAO)和顶点缓冲区对象(VBO)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 检查生成是否成功
    if (VAO == 0 || VBO == 0)
    {
        std::cerr << "Failed to generate VAO or VBO" << std::endl;
        return false;
    }

    // 绑定VAO和VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 设置顶点属性指针
    // 参数说明：索引0, 3个分量, 浮点类型, 不标准化, 步长为3个float大小, 偏移量为0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  // 启用顶点属性

    checkGLError("initBuffers");
    return true;
}

// 渲染场景
// picking参数：true表示拾取模式，false表示普通渲染模式
void renderScene(bool bPicking)
{
    // 清除颜色缓冲区和深度缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 选择要使用的着色器程序
    unsigned int program = bPicking ? g_pickShaderProgram : g_shaderProgram;
    glUseProgram(program);
    checkGLError("glUseProgram");

    // 设置视图矩阵和投影矩阵
    glUniformMatrix4fv(glGetUniformLocation(program, "matView"), 1, GL_FALSE, glm::value_ptr(matView));
    glUniformMatrix4fv(glGetUniformLocation(program, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));

    glBindVertexArray(VAO);  // 绑定VAO
    checkGLError("glBindVertexArray");

    // 渲染每个三角形
    for (size_t i = 0; i < g_vTriangles.size(); ++i)
    {
        const Triangle& tri = g_vTriangles[i];

        // 创建单位模型矩阵（没有变换）
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

        if (bPicking)
        {
            // 拾取模式：使用编码了ID的拾取颜色
            glUniform3fv(glGetUniformLocation(program, "pickColor"), 1, glm::value_ptr(tri.pickColor));
        }
        else
        {
            // 普通渲染模式：如果是选中的三角形则显示黄色，否则显示原始颜色
            glm::vec3 selectedColor = (i == g_nSeled) ? glm::vec3(1.0f, 1.0f, 0.0f) : tri.color;
            glUniform3fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(selectedColor));
        }

        // 更新顶点数据并绘制
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tri.vertices), tri.vertices, GL_STATIC_DRAW);
        checkGLError("glBufferData");

        glDrawArrays(GL_TRIANGLES, 0, 3);  // 绘制三角形
        
        checkGLError("glDrawArrays");
    }
}

// 主函数
int main()
{
    // 初始化GLFW库
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置OpenGL版本和配置文件
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);     // 主版本4
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);     // 次版本6
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 核心配置文件

    // 创建GLFW窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Projection Triangle Picking", nullptr, nullptr);
    if (!window)
    {
        const char* description;
        glfwGetError(&description);
        std::cerr << "Failed to create GLFW window: " << description << std::endl;
        glfwTerminate();  // 终止GLFW
        return -1;
    }
    glfwMakeContextCurrent(window);  // 设置窗口为当前上下文

    // 初始化GLAD（加载OpenGL函数指针）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);  // 销毁窗口
        glfwTerminate();  // 终止GLFW
        return -1;
    }

    // 设置视口和启用深度测试
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);  // 设置视口大小
    glEnable(GL_DEPTH_TEST);  // 启用深度测试
    checkGLError("glEnable(GL_DEPTH_TEST)");

    // 注册鼠标点击回调函数
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // 创建着色器程序
    g_shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    g_pickShaderProgram = createShaderProgram(vertexShaderSource, pickFragmentShaderSource);
    if (g_shaderProgram == 0 || g_pickShaderProgram == 0)
    {
        std::cerr << "Failed to create shader programs" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 初始化三角形数据和缓冲区
    initTriangles();

    if (!initBuffers())
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 设置投影矩阵和视图矩阵
    // 透视投影：垂直视角45度，宽高比，近平面0.1，远平面100
    matProj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

    // 视图矩阵：相机位置(0,0,10)，看向原点，上方向(0,1,0)
    matView = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        if (g_bPickMode)
        {
            // 处理拾取模式
            //  清除缓冲区并以拾取模式渲染场景
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderScene(true);

            // 获取鼠标位置
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            ypos = SCR_HEIGHT - ypos;  // 翻转y轴（窗口坐标系与OpenGL坐标系y轴方向相反）

            // 读取鼠标点击位置的像素颜色
            unsigned char pixel[3];
            glReadPixels((int)xpos, (int)ypos, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
            checkGLError("glReadPixels");

            // 解码拾取颜色，获取三角形ID
            // 红色分量左移16位，绿色分量左移8位，蓝色分量不变
            int nPickedID = (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];

            // 更新选中的三角形索引
            if (nPickedID < g_vTriangles.size())
            {
                g_nSeled = nPickedID;
                std::cout << "Selected triangle: " << g_nSeled << std::endl;
            }
            else
            {
                g_nSeled = -1;  // 没有选中任何三角形
            }

            g_bPickMode = false;  // 结束拾取模式
        }

        // 正常渲染场景
        renderScene(false);

        // 交换前后缓冲区并处理事件
        glfwSwapBuffers(window);  // 交换颜色缓冲区
        glfwPollEvents();         // 处理窗口事件
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);       // 删除VAO
    glDeleteBuffers(1, &VBO);           // 删除VBO
    glDeleteProgram(g_shaderProgram);     // 删除普通着色器程序
    glDeleteProgram(g_pickShaderProgram); // 删除拾取着色器程序
    glfwDestroyWindow(window);          // 销毁窗口
    glfwTerminate();                    // 终止GLFW
    return 0;
}