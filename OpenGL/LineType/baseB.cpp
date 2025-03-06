// 不修改 正交投影, 从而改变虚线间隔

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>

constexpr float X = 4.0f;

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 in_pos;
layout(location = 1) in float in_len;
uniform mat4 cameraTrans;
uniform float dashScale;
uniform float timeOffset = 0.0;
out float dashParam;

void main() {
    gl_Position = cameraTrans * vec4(in_pos, 0.0, 1.0);
    dashParam = in_len * dashScale + timeOffset;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in float dashParam;
uniform vec4 color;
out vec4 fragColor;

void main() {
    float dashPattern = mod(dashParam, 1.0);
    if (dashPattern < 0.5) {
        fragColor = color;
    } else {
        discard;
    }
}
)";

GLuint loadShader(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

glm::vec2 randomPoint(float minX = -X, float maxX = X, float minY = -X, float maxY = X)
{
    return glm::vec2(
        minX + (rand() / static_cast<float>(RAND_MAX)) * (maxX - minX),
        minY + (rand() / static_cast<float>(RAND_MAX)) * (maxY - minY)
    );
}

void generateRandomMixedLine(
    std::vector<float>& vertices,
    int numSegments,
    int lineSegments,
    int bezierSegments,
    float minX, float maxX, float minY, float maxY
)
{
    glm::vec2 startPoint = randomPoint(minX, maxX, minY, maxY);
    glm::vec2 currentPoint = startPoint;
    glm::vec2 prevPoint = startPoint;
    float dAccLen = 0.0;

    vertices.push_back(startPoint.x);
    vertices.push_back(startPoint.y);
    vertices.push_back(dAccLen);

    for (int i = 0; i < numSegments; ++i)
    {
        bool bLine = rand() % 2 == 0;

        if (bLine)
        {
            glm::vec2 point = randomPoint(minX, maxX, minY, maxY);
            vertices.push_back(point.x);
            vertices.push_back(point.y);

            float segmentLength = glm::distance(prevPoint, point);
            dAccLen += segmentLength;
            vertices.push_back(dAccLen);

            vertices.push_back(point.x);
            vertices.push_back(point.y);
            vertices.push_back(dAccLen);

            prevPoint = point;
            currentPoint = point;
        }
        else
        {
            glm::vec2 controlPoint1 = randomPoint(minX, maxX, minY, maxY);
            glm::vec2 controlPoint2 = randomPoint(minX, maxX, minY, maxY);
            glm::vec2 nextPoint = randomPoint(minX, maxX, minY, maxY);

            for (int j = 1; j <= bezierSegments; ++j)
            {
                float t = float(j) / float(bezierSegments);
                float u = 1.0f - t;

                glm::vec2 point = u * u * u * currentPoint +
                    3.0f * u * u * t * controlPoint1 +
                    3.0f * u * t * t * controlPoint2 +
                    t * t * t * nextPoint;

                vertices.push_back(point.x);
                vertices.push_back(point.y);

                float segmentLength = glm::distance(prevPoint, point);
                dAccLen += segmentLength;
                vertices.push_back(dAccLen);

                vertices.push_back(point.x);
                vertices.push_back(point.y);
                vertices.push_back(dAccLen);

                prevPoint = point;
            }

            currentPoint = nextPoint;
        }
    }
}

float zoomFactor = 1.0f;
GLint cameraTransLoc;  // 全局变量存储 uniform 位置

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoomFactor += float(yoffset) * 0.1f;
    zoomFactor = std::max(zoomFactor, 0.1f);

    // 在回调中更新 cameraTrans
    glm::mat4 cameraTrans = glm::ortho(-X * zoomFactor, X * zoomFactor, -X * zoomFactor, X * zoomFactor);
    glUniformMatrix4fv(cameraTransLoc, 1, GL_FALSE, &cameraTrans[0][0]);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    srand(static_cast<unsigned int>(time(NULL)));

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(1400, 1400, "OpenGL Dash Circle", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLuint shaderProgram = loadShader(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    // 初始化 cameraTrans 并获取 uniform 位置
    glm::mat4 cameraTrans = glm::ortho(-X, X, -X, X);
    cameraTransLoc = glGetUniformLocation(shaderProgram, "cameraTrans");
    glUniformMatrix4fv(cameraTransLoc, 1, GL_FALSE, &cameraTrans[0][0]);

    glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);

    std::vector<float> shapeVertices;
    {
        const int NUM_LINES = 6;
        const int NUM_SEGMENTS = 3;
        const int BEZIER_RES = 30;
        const int CIRCLE_RES = 350;
        generateRandomMixedLine(shapeVertices, NUM_LINES, BEZIER_RES, BEZIER_RES, -X, X, -X, X);
    }

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(float), shapeVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<void*>(0 * sizeof(float)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glfwSetScrollCallback(window, scroll_callback);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        float dashScale = 4.0f / zoomFactor;
        glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), dashScale);

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float>(now.time_since_epoch());
        float time = duration.count();
        glUniform1f(glGetUniformLocation(shaderProgram, "timeOffset"), time * 0.8f);

        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(shapeVertices.size() / 3));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}