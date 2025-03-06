#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>  // 用于 rand()
#include <time.h>    // 用于 srand()
#include <vector>    // 添加 vector 头文件

// 顶点着色器
const char* vertexShaderSource = "#version 400\n"
"layout (location = 0) in vec2 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
"}\0";

// 几何着色器
const char* geometryShaderSource = "#version 400\n"
"layout (lines_adjacency) in;\n"  // 使用 lines_adjacency 输入布局
"layout (triangle_strip, max_vertices = 4) out;\n"
"uniform float thickness = 0.03;\n"  // 线条粗细
"void main()\n"
"{\n"
"   vec2 p0 = gl_in[0].gl_Position.xy;\n"  // 前一个点
"   vec2 p1 = gl_in[1].gl_Position.xy;\n"  // 起点
"   vec2 p2 = gl_in[2].gl_Position.xy;\n"  // 终点
"   vec2 p3 = gl_in[3].gl_Position.xy;\n"  // 后一个点
"   \n"
"   // 计算前段和后段的方向\n"
"   vec2 dir1 = normalize(p2 - p1);\n"
"   vec2 dir2 = normalize(p3 - p2);\n"
"   \n"
"   // 计算法线\n"
"   vec2 normal1 = vec2(-dir1.y, dir1.x);\n"
"   vec2 normal2 = vec2(-dir2.y, dir2.x);\n"
"   \n"
"   // 计算连接处的法线（角度平分线）\n"
"   vec2 normal = normalize(normal1 + normal2);\n"
"   \n"
"   // 计算偏移\n"
"   vec2 offset1 = normal * thickness;\n"
"   vec2 offset2 = normal * thickness;\n"
"   \n"
"   // 生成粗线的顶点\n"
"   gl_Position = vec4(p1 - offset1, 0.0, 1.0);\n"
"   EmitVertex();\n"
"   gl_Position = vec4(p1 + offset1, 0.0, 1.0);\n"
"   EmitVertex();\n"
"   gl_Position = vec4(p2 - offset2, 0.0, 1.0);\n"
"   EmitVertex();\n"
"   gl_Position = vec4(p2 + offset2, 0.0, 1.0);\n"
"   EmitVertex();\n"
"   EndPrimitive();\n"
"}\0";

// 片段着色器
const char* fragmentShaderSource = "#version 400\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"  // 红色线条
"}\0";

// 生成随机顶点的函数，返回 vector
std::vector<float> generateRandomVertices(int numPoints)
{
    std::vector<float> vertices(numPoints * 2);  // 每个点需要2个float (x, y)

    // 随机数种子，只需初始化一次
    static int seeded = 0;
    if (!seeded)
    {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

    // 生成随机坐标，范围在 [-1, 1]
    for (int i = 0; i < numPoints * 2; i += 2)
    {
        vertices[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;     // x坐标
        vertices[i + 1] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; // y坐标
    }

    return vertices;
}

// 为 lines_adjacency 准备顶点数据
std::vector<float> prepareAdjacencyVertices(const std::vector<float>& vertices, int numPoints)
{
    std::vector<float> adjVertices;
    adjVertices.reserve(4 * (numPoints - 1) * 2);  // 每个图元4个点，每个点2个float

    // 第一个图元：重复第一个点
    adjVertices.push_back(vertices[0]);  // p0 (重复)
    adjVertices.push_back(vertices[1]);
    adjVertices.push_back(vertices[0]);  // p1
    adjVertices.push_back(vertices[1]);
    adjVertices.push_back(vertices[2]);  // p2
    adjVertices.push_back(vertices[3]);
    adjVertices.push_back(vertices[4]);  // p3
    adjVertices.push_back(vertices[5]);

    // 中间图元
    for (int i = 1; i < numPoints - 2; i++)
    {
        adjVertices.push_back(vertices[2 * (i - 1)]);  // p0
        adjVertices.push_back(vertices[2 * (i - 1) + 1]);
        adjVertices.push_back(vertices[2 * i]);      // p1
        adjVertices.push_back(vertices[2 * i + 1]);
        adjVertices.push_back(vertices[2 * (i + 1)]);  // p2
        adjVertices.push_back(vertices[2 * (i + 1) + 1]);
        adjVertices.push_back(vertices[2 * (i + 2)]);  // p3
        adjVertices.push_back(vertices[2 * (i + 2) + 1]);
    }

    // 最后一个图元：重复最后一个点
    adjVertices.push_back(vertices[2 * (numPoints - 3)]);  // p0
    adjVertices.push_back(vertices[2 * (numPoints - 3) + 1]);
    adjVertices.push_back(vertices[2 * (numPoints - 2)]);  // p1
    adjVertices.push_back(vertices[2 * (numPoints - 2) + 1]);
    adjVertices.push_back(vertices[2 * (numPoints - 1)]);  // p2
    adjVertices.push_back(vertices[2 * (numPoints - 1) + 1]);
    adjVertices.push_back(vertices[2 * (numPoints - 1)]);  // p3 (重复)
    adjVertices.push_back(vertices[2 * (numPoints - 1) + 1]);

    return adjVertices;
}

// 窗口大小调整回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        printf("GLFW initialization failed\n");
        return -1;
    }

    // 设置 OpenGL 版本和核心配置文件
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Thick Polyline with Adjacency", NULL, NULL);
    if (!window)
    {
        printf("Window creation failed\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    // 编译着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
    glCompileShader(geometryShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 删除不再需要的着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    // 生成随机顶点
    int numPoints = 50;
    std::vector<float> vertices = generateRandomVertices(numPoints);

    // 准备 lines_adjacency 顶点数据
    std::vector<float> adjVertices = prepareAdjacencyVertices(vertices, numPoints);

    // 创建并绑定 VAO 和 VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, adjVertices.size() * sizeof(float), adjVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        // 使用 GL_LINES_ADJACENCY 绘制
        glDrawArrays(GL_LINES_ADJACENCY, 0, 4 * (numPoints - 1));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}