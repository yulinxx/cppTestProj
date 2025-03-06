// 既不在主循环中更新 cameraTrans，也不在 scroll_callback 中更新cameraTrans，因为投影矩阵可能会影响其他 Shader，而你想让缩放逻辑独立于投影矩阵，同时保持虚线间隔固定。

// 在这种情况下，我们需要一种方法，让缩放效果不依赖于修改 cameraTrans，而是将缩放逻辑完全隔离到当前 Shader 的顶点处理中，并且不影响其他 Shader。最好的办法是：

// 在顶点着色器中引入一个独立的缩放因子（zoomFactor），直接作用于顶点位置。
// 将固定的 cameraTrans 设置一次，不再动态修改。
// 通过主循环更新 zoomFactor 和 dashScale，以保持虚线间隔固定。
// 以下是完整实现：

// 关键改动

// 顶点着色器中的缩放：
// 添加 uniform float zoomFactor = 1.0f，并在着色器中对顶点位置应用缩放：scaledPos = in_pos / zoomFactor。
// 使用除法（/）而不是乘法（*），因为 zoomFactor 增大时应缩小视图，反之亦然，这样与你的滚轮方向一致（向上滚放大，向下滚缩小）。
// 固定 cameraTrans：
// 在初始化时设置一次 glm::ortho(-X, X, -X, X)，之后不再修改，保证不影响其他 Shader。
// 主循环中更新：
// 只更新 dashScale（4.0f / zoomFactor）和 zoomFactor，保持虚线间隔固定。
// scroll_callback 的简化：
// 只更新 zoomFactor，不触及 OpenGL 状态。

// 工作原理
// zoomFactor 在着色器中的作用：
// 当 zoomFactor 增大（例如从 1.0 到 2.0），in_pos / zoomFactor 使顶点坐标变小，视图放大。
// 当 zoomFactor 减小（例如从 1.0 到 0.5），in_pos / zoomFactor 使顶点坐标变大，视图缩小。
// dashScale 的调整：
// dashScale = 4.0f / zoomFactor 确保虚线间隔在屏幕空间中保持固定，因为它与缩放因子反向关联。
// 不影响其他 Shader：
// cameraTrans 保持不变，其他 Shader 不会受到影响。

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
uniform float zoomFactor = 1.0f;  // 独立的缩放因子
uniform float timeOffset = 0.0;
out float dashParam;

void main() {
    vec2 scaledPos = in_pos / zoomFactor;  // 在着色器中应用缩放（除以 zoomFactor 表示放大）
    gl_Position = cameraTrans * vec4(scaledPos, 0.0, 1.0);
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoomFactor += float(yoffset) * 0.1f;
    zoomFactor = std::max(zoomFactor, 0.1f);  // 只更新 zoomFactor，不触及 cameraTrans
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

    glfwSetScrollCallback(window, scroll_callback);

    GLuint shaderProgram = loadShader(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    // 设置固定的投影矩阵，只初始化一次
    glm::mat4 cameraTrans = glm::ortho(-X, X, -X, X);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

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

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        // 更新 dashScale 和 zoomFactor，不触及 cameraTrans
        float dashScale = 4.0f / zoomFactor;  // 保持虚线间隔固定
        glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), dashScale);
        glUniform1f(glGetUniformLocation(shaderProgram, "zoomFactor"), zoomFactor);

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