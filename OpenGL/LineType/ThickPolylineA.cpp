// 引入GLAD库,用于加载OpenGL函数指针
#include <glad/glad.h>
// 引入GLFW库,用于创建窗口和处理输入事件
#include <GLFW/glfw3.h>
// 引入标准输入输出库,用于打印错误信息
#include <stdio.h>
// 引入标准库,用于使用rand()函数生成随机数
#include <stdlib.h>
// 引入时间库,用于初始化随机数种子
#include <time.h>
// 引入向量容器库,用于存储顶点数据
#include <vector>

/**
 * 顶点着色器源码
 * 该着色器接收顶点位置数据,并将其转换为齐次坐标
 */
const char* vertexShaderSource = "#version 400\n"
// 定义顶点位置属性,位置为0
"layout (location = 0) in vec2 aPos;\n"
// 主函数
"void main()\n"
"{\n"
// 将顶点位置转换为齐次坐标
"   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
"}\0";

/**
 * 几何着色器源码
 * 该着色器将输入的线段转换为三角形带,以实现粗线效果
 */
const char* geometryShaderSource = "#version 400\n"
// 定义输入图元类型为线段
"layout (lines) in;\n"
// 定义输出图元类型为三角形带,最大顶点数为4
"layout (triangle_strip, max_vertices = 4) out;\n"
// 定义线宽的统一变量,默认值为0.003
"uniform float thickness = 0.003;\n"
// 主函数
"void main()\n"
"{\n"
// 获取第一个顶点的位置
"   vec2 p0 = gl_in[0].gl_Position.xy;\n"
// 获取第二个顶点的位置
"   vec2 p1 = gl_in[1].gl_Position.xy;\n"
// 计算线段的方向向量,并归一化
"   vec2 dir = normalize(p1 - p0);\n"
// 计算线段的法向量
"   vec2 normal = vec2(-dir.y, dir.x);\n"
// 计算偏移向量
"   vec2 offset = normal * thickness;\n"
// 发射第一个顶点
"   gl_Position = vec4(p0 - offset, 0.0, 1.0);\n"
"   EmitVertex();\n"
// 发射第二个顶点
"   gl_Position = vec4(p0 + offset, 0.0, 1.0);\n"
"   EmitVertex();\n"
// 发射第三个顶点
"   gl_Position = vec4(p1 - offset, 0.0, 1.0);\n"
"   EmitVertex();\n"
// 发射第四个顶点
"   gl_Position = vec4(p1 + offset, 0.0, 1.0);\n"
"   EmitVertex();\n"
// 结束图元
"   EndPrimitive();\n"
"}\0";

/**
 * 片段着色器源码
 * 该着色器将每个片段的颜色设置为红色
 */
const char* fragmentShaderSource = "#version 400\n"
// 定义输出颜色变量
"out vec4 FragColor;\n"
// 主函数
"void main()\n"
"{\n"
// 将片段颜色设置为红色
"   FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\0";

/**
 * 生成随机顶点的函数
 * @param numPoints 顶点数量
 * @return 包含随机顶点坐标的向量
 */
std::vector<float> generateRandomVertices(int numPoints)
{
    // 每个点需要2个float (x,y)
    std::vector<float> vertices(numPoints * 2);

    // 随机数种子,只需初始化一次
    static int seeded = 0;
    if (!seeded)
    {
        // 初始化随机数种子
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

    // 生成随机坐标,范围在 [-1, 1]
    for (int i = 0; i < numPoints * 2; i += 2)
    {
        // 生成x坐标
        vertices[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        // 生成y坐标
        vertices[i + 1] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    }

    return vertices;
}

/**
 * 窗口大小改变回调函数
 * @param window 窗口指针
 * @param width 新的窗口宽度
 * @param height 新的窗口高度
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // 设置视口大小
    glViewport(0, 0, width, height);
}

/**
 * 主函数
 * 程序入口点
 */
int main()
{
    // [GLFW和GLAD初始化代码保持不变]
    // 初始化GLFW库
    if (!glfwInit())
    {
        // 打印错误信息
        printf("GLFW initialization failed\n");
        return -1;
    }

    // 设置OpenGL版本为4.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // 使用核心模式
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Thick Polyline", NULL, NULL);
    if (!window)
    {
        // 打印错误信息
        printf("Window creation failed\n");
        // 终止GLFW库
        glfwTerminate();
        return -1;
    }

    // 设置当前上下文
    glfwMakeContextCurrent(window);
    // 设置窗口大小改变回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化GLAD库
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        // 打印错误信息
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    // [着色器编译和链接代码保持不变]
    // 创建顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 设置顶点着色器源码
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // 编译顶点着色器
    glCompileShader(vertexShader);

    // 创建几何着色器
    unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    // 设置几何着色器源码
    glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
    // 编译几何着色器
    glCompileShader(geometryShader);

    // 创建片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 设置片段着色器源码
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // 编译片段着色器
    glCompileShader(fragmentShader);

    // 创建着色器程序
    unsigned int shaderProgram = glCreateProgram();
    // 附加顶点着色器
    glAttachShader(shaderProgram, vertexShader);
    // 附加几何着色器
    glAttachShader(shaderProgram, geometryShader);
    // 附加片段着色器
    glAttachShader(shaderProgram, fragmentShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);

    // 删除顶点着色器
    glDeleteShader(vertexShader);
    // 删除几何着色器
    glDeleteShader(geometryShader);
    // 删除片段着色器
    glDeleteShader(fragmentShader);

    // 使用向量生成随机顶点
    int numPoints = 50;
    std::vector<float> vertices = generateRandomVertices(numPoints);

    // 创建VBO和VAO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 绑定VAO
    glBindVertexArray(VAO);
    // 绑定VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 使用向量的data()方法获取数据指针
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    // 启用顶点属性
    glEnableVertexAttribArray(0);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 设置清屏颜色
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // 清屏
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);
        // 绑定VAO
        glBindVertexArray(VAO);
        // 修改为使用numPoints而不是固定的3
        glDrawArrays(GL_LINE_STRIP, 0, numPoints);

        // 交换前后缓冲区
        glfwSwapBuffers(window);
        // 处理事件
        glfwPollEvents();
    }

    // 清理资源
    // 删除VAO
    glDeleteVertexArrays(1, &VAO);
    // 删除VBO
    glDeleteBuffers(1, &VBO);
    // 删除着色器程序
    glDeleteProgram(shaderProgram);
    // vector会自动析构,无需手动释放

    // 终止GLFW库
    glfwTerminate();
    return 0;
}