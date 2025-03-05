// 生成多条连续的线（而不是单条线段的集合）。
// 使用 GL_LINE_STRIP 进行绘制，并确保顶点和索引存储时避免重复。
// 优化后的代码将调整 generateRandomMixedLine 函数以生成多条连续的折线，同时改进索引生成逻辑，使其适配 GL_LINE_STRIP 的连续绘制方式。以下是完整代码：

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>
#include <string>

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"

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
    float dashLength = in_len * dashScale + timeOffset;
    dashParam = dashLength;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in float dashParam;
uniform vec4 color;
uniform int dashType = 0;
out vec4 fragColor;

void main() {
    bool draw = false;
    float pattern;
    float cycle;

    switch(dashType) {
        case 0: // 默认等长虚线
            pattern = mod(dashParam, 1.0);
            draw = (pattern < 0.5);
            break;
        default:
            draw = true;
            break;
    }

    if (!draw) discard;
    fragColor = color;
}
)";

GLuint loadShader(const char* vertexShaderSource, const char* fragmentShaderSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

glm::vec2 randomPoint(float minX = -X, float maxX = X, float minY = -X, float maxY = X) {
    return glm::vec2(
        minX + (rand() / static_cast<float>(RAND_MAX)) * (maxX - minX),
        minY + (rand() / static_cast<float>(RAND_MAX)) * (maxY - minY)
    );
}

void generateRandomMixedLines(
    std::vector<float>& vertices,
    std::vector<std::vector<unsigned int>>& lineIndices,
    int numLines,
    int numSegments,
    int bezierSegments,
    float minX, float maxX, float minY, float maxY
) {
    vertices.clear();
    lineIndices.clear();

    for (int line = 0; line < numLines; ++line) {
        std::vector<unsigned int> currentLineIndices;
        glm::vec2 startPoint = randomPoint(minX, maxX, minY, maxY);
        glm::vec2 currentPoint = startPoint;
        glm::vec2 prevPoint = startPoint;
        float dAccLen = 0.0;

        unsigned int vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
        vertices.push_back(startPoint.x);
        vertices.push_back(startPoint.y);
        vertices.push_back(dAccLen);
        currentLineIndices.push_back(vertexIndex);

        for (int i = 0; i < numSegments; ++i) {
            bool bLine = rand() % 2 == 0;

            if (bLine) {
                glm::vec2 point = randomPoint(minX, maxX, minY, maxY);
                vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
                vertices.push_back(point.x);
                vertices.push_back(point.y);

                float segmentLength = glm::distance(prevPoint, point);
                dAccLen += segmentLength;
                vertices.push_back(dAccLen);

                currentLineIndices.push_back(vertexIndex);

                prevPoint = point;
                currentPoint = point;
            } else {
                glm::vec2 controlPoint1 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 controlPoint2 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 nextPoint = randomPoint(minX, maxX, minY, maxY);

                for (int j = 1; j <= bezierSegments; ++j) {
                    float t = float(j) / float(bezierSegments);
                    float u = 1.0f - t;

                    glm::vec2 point = u * u * u * currentPoint +
                                      3.0f * u * u * t * controlPoint1 +
                                      3.0f * u * t * t * controlPoint2 +
                                      t * t * t * nextPoint;

                    vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
                    vertices.push_back(point.x);
                    vertices.push_back(point.y);

                    float segmentLength = glm::distance(prevPoint, point);
                    dAccLen += segmentLength;
                    vertices.push_back(dAccLen);

                    currentLineIndices.push_back(vertexIndex);

                    prevPoint = point;
                }
                currentPoint = nextPoint;
            }
        }
        lineIndices.push_back(currentLineIndices);
    }
}

float zoomFactor = 1.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    zoomFactor += float(yoffset) * 0.1f;
    zoomFactor = std::max(zoomFactor, 0.1f);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    srand(static_cast<unsigned int>(time(NULL)));

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(1400, 1400, "OpenGL Dash Lines", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetScrollCallback(window, scroll_callback);

    GLuint shaderProgram = loadShader(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    glm::mat4 cameraTrans = glm::ortho(-X, X, -X, X);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

    glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), 8.0f);

    std::vector<float> shapeVertices;
    std::vector<std::vector<unsigned int>> shapeIndices;

    {
        const int NUM_LINES = 6;
        const int NUM_SEGMENTS = 3;
        const int BEZIER_RES = 30;
        generateRandomMixedLines(shapeVertices, shapeIndices, NUM_LINES, NUM_SEGMENTS, BEZIER_RES, -X, X, -X, X);
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(float), shapeVertices.data(), GL_STATIC_DRAW);

    std::vector<unsigned int> flatIndices;
    for (const auto& line : shapeIndices) {
        flatIndices.insert(flatIndices.end(), line.begin(), line.end());
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size() * sizeof(unsigned int), flatIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glClearColor(1.0, 1.0, 1.0, 1.0);

    while (!glfwWindowShouldClose(window)) {
        glm::mat4 cameraTrans = glm::ortho(-X * zoomFactor, X * zoomFactor, -X * zoomFactor, X * zoomFactor);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

        float dashScale = 12.0f / zoomFactor;
        glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), dashScale);

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float>(now.time_since_epoch());
        float time = duration.count();

        glUniform1f(glGetUniformLocation(shaderProgram, "timeOffset"), time * 0.8f);

        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        // 计算每条折线的偏移和计数
        std::vector<const GLvoid*> indicesOffsets;
        std::vector<GLsizei> counts;
        size_t offset = 0;
        for (const auto& line : shapeIndices) {
            counts.push_back(static_cast<GLsizei>(line.size()));
            indicesOffsets.push_back(reinterpret_cast<const GLvoid*>(offset * sizeof(unsigned int)));
            offset += line.size();
        }

        glMultiDrawElements(GL_LINE_STRIP, counts.data(), GL_UNSIGNED_INT, indicesOffsets.data(), static_cast<GLsizei>(counts.size()));

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        // static int i = 0;
        // i++;
        // if (i % 4000 == 0 && false) {
        //     int width, height;
        //     glfwGetWindowSize(window, &width, &height);
        //     std::vector<unsigned char> pixels(width * height * 4);
        //     glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        //     stbi_flip_vertically_on_write(1);
        //     std::string filename = "frame_" + std::to_string(i) + ".png";
        //     stbi_write_png(filename.c_str(), width, height, 4, pixels.data(), width * 4);
        // }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}



// 主要优化点说明
// 生成多条连续的折线：
// 函数 generateRandomMixedLines 替换了原来的 generateRandomMixedLine，增加了 numLines 参数，用于指定生成多少条独立的折线。
// 每条折线是连续的，直线段和贝塞尔曲线段通过索引连续连接。
// 顶点和索引存储优化：
// 顶点数据 (vertices) 只存储一次，避免重复。每个顶点包含 (x, y, dAccLen)。
// 索引数据 (lineIndices) 使用 std::vector<std::vector<unsigned int>> 存储，每条折线的索引单独保存在一个子数组中，确保连续性。
// 在生成顶点时，索引是连续添加的（而不是成对添加），适配 GL_LINE_STRIP 的绘制方式。
// 使用 GL_LINE_STRIP 和 glMultiDrawElements：
// 使用 glMultiDrawElements 替代 glDrawElements，以支持绘制多条独立的折线。
// 为每条折线计算起始索引位置 (firsts) 和索引数量 (counts)，然后一次性绘制所有折线。
// GL_LINE_STRIP 确保每条折线的顶点按顺序连续绘制，形成完整的折线。
// EBO 数据处理：
// 将 shapeIndices 中的所有索引展平为一个一维数组 (flatIndices)，上传到 EBO。
// glMultiDrawElements 使用偏移量和计数来区分每条折线。
// 运行与验证
// 编译运行：确保 GLFW、GLAD、GLM 和 stb_image_write.h 已正确配置。
// 效果：程序将生成 6 条独立的随机折线，每条折线由直线段和贝塞尔曲线段混合组成，显示为蓝色虚线。
// 缩放：使用鼠标滚轮可以缩放视图，虚线长度会根据缩放比例动态调整。
// 输出图像：当前代码中图像保存功能被禁用（if (i % 4000 == 0 && false)），如需启用，可将 false 改为 true。
// 进一步改进建议
// 颜色区分：为每条折线设置不同的颜色，可以通过在顶点数据中添加颜色属性或使用 Uniform 变量实现。
// 性能优化：如果折线数量非常多，考虑将顶点和索引数据改为动态缓冲（GL_DYNAMIC_DRAW），支持实时更新。
// 断开折线：如果需要绘制不连续的折线，可以在 lineIndices 中插入特殊标记（如 GL_UNSIGNED_INT 的最大值），并处理绘制逻辑。