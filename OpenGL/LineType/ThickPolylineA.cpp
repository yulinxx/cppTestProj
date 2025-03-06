#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>  // 用于 rand()
#include <time.h>    // 用于 srand()
#include <vector>    // 添加 vector 头文件

// [着色器代码保持不变]
const char* vertexShaderSource = "#version 400\n"
"layout (location = 0) in vec2 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
"}\0";

const char* geometryShaderSource = "#version 400\n"
"layout (lines) in;\n"
"layout (triangle_strip, max_vertices = 4) out;\n"
"uniform float thickness = 0.003;\n"
"void main()\n"
"{\n"
"   vec2 p0 = gl_in[0].gl_Position.xy;\n"
"   vec2 p1 = gl_in[1].gl_Position.xy;\n"
"   vec2 dir = normalize(p1 - p0);\n"
"   vec2 normal = vec2(-dir.y, dir.x);\n"
"   vec2 offset = normal * thickness;\n"
"   gl_Position = vec4(p0 - offset, 0.0, 1.0);\n"
"   EmitVertex();\n"
"   gl_Position = vec4(p0 + offset, 0.0, 1.0);\n"
"   EmitVertex();\n"
"   gl_Position = vec4(p1 - offset, 0.0, 1.0);\n"
"   EmitVertex();\n"
"   gl_Position = vec4(p1 + offset, 0.0, 1.0);\n"
"   EmitVertex();\n"
"   EndPrimitive();\n"
"}\0";

const char* fragmentShaderSource = "#version 400\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\0";

// 生成随机顶点的函数，返回 vector
std::vector<float> generateRandomVertices(int numPoints)
{
    std::vector<float> vertices(numPoints * 2);  // 每个点需要2个float (x,y)

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    // [GLFW和GLAD初始化代码保持不变]
    if (!glfwInit())
    {
        printf("GLFW initialization failed\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Thick Polyline", NULL, NULL);
    if (!window)
    {
        printf("Window creation failed\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    // [着色器编译和链接代码保持不变]
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
    glCompileShader(geometryShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    // 使用 vector 生成随机顶点
    int numPoints = 50;
    std::vector<float> vertices = generateRandomVertices(numPoints);

    // 创建 VBO 和 VAO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 使用 vector 的 data() 方法获取数据指针
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        // 修改为使用 numPoints 而不是固定的 3
        glDrawArrays(GL_LINE_STRIP, 0, numPoints);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    // vector 会自动析构，无需手动释放

    glfwTerminate();
    return 0;
}