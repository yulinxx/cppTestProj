#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

// 顶点着色器
const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    uniform mat4 transform;
    void main() {
        gl_Position = transform * vec4(aPos, 0.0, 1.0);
    }
)";

// 片段着色器
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色
    }
)";

// 创建着色器程序
unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// 数字的顶点数据
std::vector<std::vector<float>> digitVertices = {
    // 数字0
    {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5},
    // 数字1
    {0.0, -0.5, 0.0, 0.5, -0.25, 0.5, 0.25, 0.5},
    // 数字2
    {-0.5, 0.5, 0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0, -0.5, -0.5, -0.5, -0.5, 0.0, 0.0, 0.0, 0.0, 0.5},
    // 数字3
    {-0.5, 0.5, 0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0, -0.5, 0.5, -0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5},
    // 数字4
    {-0.5, 0.5, 0.0, 0.5, 0.0, -0.5, 0.5, -0.5, 0.5, 0.0, 0.0, 0.0},
    // 数字5
    {0.5, 0.5, -0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 0.0, -0.5, 0.5, -0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5},
    // 数字6
    {0.5, 0.0, 0.5, -0.5, -0.5, -0.5, -0.5, 0.0, -0.5, 0.5, 0.5, 0.5, 0.5, 0.0, 0.5, -0.5, -0.5, -0.5, -0.5, 0.0},
    // 数字7
    {-0.5, 0.5, 0.5, 0.5, 0.0, -0.5},
    // 数字8
    {-0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.0, 0.0, 0.0, -0.5, 0.5, -0.5, 0.5, 0.0, 0.0, 0.0},
    // 数字9
    {-0.5, 0.0, -0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.0, -0.5, 0.0, 0.0, 0.5, 0.0}
};

void drawDigit(int digit, float x, float y, float scale, unsigned int shaderProgram)
{
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, digitVertices[digit].size() * sizeof(float), digitVertices[digit].data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 设置变换矩阵
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(x, y, 0.0f));
    transform = glm::scale(transform, glm::vec3(scale, scale, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, glm::value_ptr(transform));

    glDrawArrays(GL_LINE_LOOP, 0, digitVertices[digit].size() / 2);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(1280, 720, "GLSL Ruler", NULL, NULL);
    glfwMakeContextCurrent(window);

    glewInit();

    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // 绘制标尺
        for (float i = 0; i <= 10; i++)
        {
            // 绘制刻度
            glBegin(GL_LINES);
            glVertex2f(i, 0);
            glVertex2f(i, 0.1);
            glEnd();

            // 绘制数字
            drawDigit(static_cast<int>(i), i, -0.2, 0.1, shaderProgram);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}