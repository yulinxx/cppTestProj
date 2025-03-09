// 使用几何着色器,绘制出一条简单的  --->--- 线

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @brief 顶点着色器源代码
 *
 * 该顶点着色器接收二维顶点位置,并将其转换为齐次坐标,然后乘以投影矩阵.
 */
const char* vertexShaderSource = R"(
#version 330 core
// 输入顶点位置,布局位置为 0
layout(location = 0) in vec2 in_pos;

// 投影矩阵,用于将顶点从模型空间转换到裁剪空间
uniform mat4 projection;

void main() {
    // 将顶点位置转换为齐次坐标,并乘以投影矩阵
    gl_Position = projection * vec4(in_pos, 0.0, 1.0);
}
)";

/**
 * @brief 几何着色器源代码
 *
 * 该几何着色器接收线段作为输入,并输出带有箭头的线段.
 */
const char* geometryShaderSource = R"(
#version 330 core
// 输入为线段
layout(lines) in;
// 输出为线段带,最大顶点数为 6
layout(line_strip, max_vertices = 6) out; // 增加顶点输出数量,确保完整箭头

// 投影矩阵,用于将顶点从模型空间转换到裁剪空间
uniform mat4 projection;  // 传递投影矩阵
// 箭头大小,默认为 0.1
uniform float arrowSize = 0.1;  // 箭头大小

void main() {
    // 线段起点
    vec2 p0 = gl_in[0].gl_Position.xy;
    // 线段终点
    vec2 p1 = gl_in[1].gl_Position.xy;
    // 线段方向
    vec2 dir = normalize(p1 - p0);
    // 垂直方向
    vec2 perp = vec2(-dir.y, dir.x);

    // 绘制线段
    gl_Position = projection * vec4(p0, 0.0, 1.0);
    EmitVertex();
    gl_Position = projection * vec4(p1, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();

    // 绘制 > 形式的箭头(位于线段中点)
    // 箭头中心位置(中点)
    vec2 arrowPos = mix(p0, p1, 0.5);
    // 箭头尖端
    vec2 tip = arrowPos + dir * arrowSize;
    // 上翼
    vec2 wing1 = arrowPos - dir * arrowSize * 0.5 + perp * arrowSize * 0.5;
    // 下翼
    vec2 wing2 = arrowPos - dir * arrowSize * 0.5 - perp * arrowSize * 0.5;

    // 第一条线段:中心到上翼
    gl_Position = projection * vec4(arrowPos, 0.0, 1.0);
    EmitVertex();
    gl_Position = projection * vec4(wing1, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();

    // 第二条线段:中心到下翼
    gl_Position = projection * vec4(arrowPos, 0.0, 1.0);
    EmitVertex();
    gl_Position = projection * vec4(wing2, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
}
)";

/**
 * @brief 片段着色器源代码
 *
 * 该片段着色器设置片段的颜色.
 */
const char* fragmentShaderSource = R"(
#version 330 core
// 输出片段颜色
out vec4 fragColor;

// 颜色,默认为蓝色
uniform vec4 color = vec4(0.0, 0.0, 1.0, 1.0);  // 蓝色

void main() {
    // 设置片段颜色
    fragColor = color;
}
)";

/**
 * @brief 加载并编译着色器程序
 *
 * 该函数创建并编译顶点着色器、几何着色器和片段着色器,然后将它们链接到一个着色器程序中.
 *
 * @return GLuint 着色器程序的 ID
 */
GLuint loadShader()
{
    // 创建顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 设置顶点着色器源代码
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    // 编译顶点着色器
    glCompileShader(vertexShader);

    // 创建几何着色器
    GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    // 设置几何着色器源代码
    glShaderSource(geometryShader, 1, &geometryShaderSource, nullptr);
    // 编译几何着色器
    glCompileShader(geometryShader);

    // 创建片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 设置片段着色器源代码
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    // 编译片段着色器
    glCompileShader(fragmentShader);

    // 创建着色器程序
    GLuint shaderProgram = glCreateProgram();
    // 将顶点着色器附加到着色器程序
    glAttachShader(shaderProgram, vertexShader);
    // 将几何着色器附加到着色器程序
    glAttachShader(shaderProgram, geometryShader);
    // 将片段着色器附加到着色器程序
    glAttachShader(shaderProgram, fragmentShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);

    // 删除不再需要的着色器
    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

/**
 * @brief 主函数,程序入口
 *
 * 该函数初始化 GLFW 和 GLAD,创建窗口和着色器程序,定义顶点数据,设置投影矩阵,然后进入渲染循环.
 *
 * @return int 程序退出状态码
 */
int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置 OpenGL 版本和配置
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Simple Line with Arrow", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 设置当前上下文
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 创建着色器程序
    GLuint shaderProgram = loadShader();
    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 定义一条简单的线段
    std::vector<float> vertices = {
        -0.5f, 0.0f,  // 起点
         0.5f, 0.0f   // 终点
    };

    // 创建 VAO 和 VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 绑定 VAO 和 VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 将顶点数据复制到 VBO
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    // 启用顶点属性
    glEnableVertexAttribArray(0);

    // 设置正交投影矩阵,覆盖整个窗口
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    // 设置投影矩阵的统一变量
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    // 设置线宽(可选,增强可见性)
    glLineWidth(2.0f);

    // 设置清除颜色
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);

        // 绑定 VAO
        glBindVertexArray(VAO);
        // 绘制一条线段
        glDrawArrays(GL_LINES, 0, 2);

        // 检查 OpenGL 错误
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        // 交换前后缓冲区
        glfwSwapBuffers(window);
        // 处理事件
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}