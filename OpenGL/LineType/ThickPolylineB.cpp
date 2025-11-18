#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>  // 用于 rand()
#include <time.h>    // 用于 srand()
#include <vector>    // 添加 vector 头文件

// 顶点着色器(使用 Raw String Literal)
const char* vertexShaderSource = R"(
#version 400
layout (location = 0) in vec2 aPos;
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

// 几何着色器(优化法线和偏移计算,使用 Raw String Literal)
const char* geometryShaderSource = R"(
#version 400
layout (lines_adjacency) in;           // 输入:带邻接信息的线段
layout (triangle_strip, max_vertices = 32) out; // 输出:三角形条带,增加顶点数支持圆角
uniform float thickness;               // 线条厚度
uniform int segments = 8;              // 圆角细分段数

void main()
{
    // 获取四个输入点
    vec2 p0 = gl_in[0].gl_Position.xy; // 前一个点(邻接)
    vec2 p1 = gl_in[1].gl_Position.xy; // 线段起点
    vec2 p2 = gl_in[2].gl_Position.xy; // 线段终点
    vec2 p3 = gl_in[3].gl_Position.xy; // 后一个点(邻接)

    // 计算线段方向
    vec2 dir1 = normalize(p2 - p1);    // 第一段方向
    vec2 dir2 = normalize(p3 - p2);    // 第二段方向

    // 计算法线
    vec2 normal1 = vec2(-dir1.y, dir1.x);
    vec2 normal2 = vec2(-dir2.y, dir2.x);

    // 生成直线部分的顶点
    vec2 offset1 = thickness * normal1;
    vec2 offset2 = thickness * normal2;

    gl_Position = vec4(p1 - offset1, 0.0, 1.0); EmitVertex();
    gl_Position = vec4(p1 + offset1, 0.0, 1.0); EmitVertex();
    gl_Position = vec4(p2 - offset2, 0.0, 1.0); EmitVertex();
    gl_Position = vec4(p2 + offset2, 0.0, 1.0); EmitVertex();

    // 生成圆角顶点(仅在转折处)
    if (dot(dir1, dir2) < 0.999) {  // 判断是否为转折处
        vec2 center = p2;  // 转折点为中心
        float angle1 = atan(normal1.y, normal1.x);
        float angle2 = atan(normal2.y, normal2.x);
        float delta_angle = angle2 - angle1;
        if (delta_angle > 3.14159) delta_angle -= 2 * 3.14159;
        if (delta_angle < -3.14159) delta_angle += 2 * 3.14159;

        for (int i = 0; i <= segments; ++i) {
            float t = float(i) / float(segments);
            float angle = angle1 + t * delta_angle;
            vec2 offset = thickness * vec2(cos(angle), sin(angle));
            gl_Position = vec4(center + offset, 0.0, 1.0);
            EmitVertex();
        }
    }

    EndPrimitive();
}
)";

// 片段着色器(使用 Raw String Literal)
const char* fragmentShaderSource = R"(
#version 400
out vec4 FragColor;
uniform vec4 lineColor;  // 在全局作用域声明 uniform

void main()
{
    FragColor = lineColor;
}
)";

// 生成随机顶点的函数,返回 vector
std::vector<float> generateRandomVertices(int numPoints)
{
    std::vector<float> vertices(numPoints * 2);  // 每个点需要2个float (x, y)

    static int seeded = 0;
    if (!seeded)
    {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

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
    adjVertices.reserve(4 * (numPoints - 1) * 2);  // 每个图元4个点,每个点2个float

    // 第一个图元:重复第一个点
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

    // 最后一个图元:重复最后一个点
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
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
    glCompileShader(geometryShader);

    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n%s\n", infoLog);
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    // 删除不再需要的着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    // 生成随机顶点(增加点数以改善平滑性)
    int numPoints = 10;  // 增加点数
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

    // 获取 uniform 位置
    glUseProgram(shaderProgram);
    GLint thicknessLoc = glGetUniformLocation(shaderProgram, "thickness");
    GLint lineColorLoc = glGetUniformLocation(shaderProgram, "lineColor");

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        // 设置 uniform 值
        glUniform1f(thicknessLoc, 0.05f);           // 设置线条粗细
        glUniform4f(lineColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);  // 设置红色

        glBindVertexArray(VAO);
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